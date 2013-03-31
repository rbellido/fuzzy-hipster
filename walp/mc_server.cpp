#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE     1024
#define MAXADDRSTR  16

#define TIMECAST_ADDR   "234.5.6.7"
#define TIMECAST_PORT   8910
#define TIMECAST_TTL    2
#define TIMECAST_INTRVL 5

char achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
u_long  lMCAddr;
u_short nPort              = TIMECAST_PORT;
u_long  lTTL               = TIMECAST_TTL;
u_short nInterval          = TIMECAST_INTRVL;
SYSTEMTIME stSysTime;

int main(int argc, char *argv[])
{
  int				nRet, i;
  BOOL				fFlag;
  SOCKADDR_IN		stLclAddr, stDstAddr;
  struct ip_mreq	stMreq;        /* Multicast interface structure */
  SOCKET			hSocket;
  WSADATA			stWSAData;

  printf("------------------------------------------------------\n");
  printf(" TimeCastSrvr - multicast time server\n");
  printf("------------------------------------------------------\n");

  // Init WinSock
  nRet = WSAStartup(0x0202, &stWSAData);
  if (nRet)
  {
      printf ("WSAStartup failed: %d\r\n", nRet);
      exit (1);
  }


  // Display current settings
  printf ("Multicast Address:%s, Port:%d, IP TTL:%d, Interval:%d\n",
    achMCAddr, nPort, lTTL, nInterval);


  // Get a datagram socket
  hSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (hSocket == INVALID_SOCKET)
  {
    printf ("socket() failed, Err: %d\n", WSAGetLastError());
    exit(1);
  }

  // Bind the socket
  stLclAddr.sin_family      = AF_INET; 
  stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY); // any interface
  stLclAddr.sin_port        = 0;                 // any port
  
  nRet = bind(hSocket, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr));
  if (nRet == SOCKET_ERROR) 
  {
      printf ("bind() port: %d failed, Err: %d\n", nPort, 
      WSAGetLastError());
  }

  // Join the multicast group
  stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
  stMreq.imr_interface.s_addr = INADDR_ANY;
  
  nRet = setsockopt(hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
  if (nRet == SOCKET_ERROR)
  {
    printf ("setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n",
				achMCAddr, WSAGetLastError());
  }

  // Set IP TTL to traverse up to multiple routers
  nRet = setsockopt(hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&lTTL, sizeof(lTTL));
  if (nRet == SOCKET_ERROR)
  {
    printf ("setsockopt() IP_MULTICAST_TTL failed, Err: %d\n",
      WSAGetLastError());
  }

  // Disable loopback
  fFlag = FALSE;
  nRet = setsockopt(hSocket, IPPROTO_IP,  IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));
  if (nRet == SOCKET_ERROR)
  {
    printf ("setsockopt() IP_MULTICAST_LOOP failed, Err: %d\n",
      WSAGetLastError());
  }

  // Assign our destination address
  stDstAddr.sin_family =      AF_INET;
  stDstAddr.sin_addr.s_addr = inet_addr(achMCAddr);
  stDstAddr.sin_port =        htons(nPort);

  for (;;)
  {
    GetSystemTime (&stSysTime);

    nRet = sendto(hSocket, (char *)&stSysTime, sizeof(stSysTime), 0,
			(struct sockaddr*)&stDstAddr, sizeof(stDstAddr));
    if (nRet < 0)
	{
      printf ("sendto() failed, Error: %d\n", WSAGetLastError());
      exit(1);
    }
	else
	{
        printf("Sent UTC Time %02d:%02d:%02d:%03d ",
            stSysTime.wHour, 
            stSysTime.wMinute, 
            stSysTime.wSecond,
			stSysTime.wMilliseconds);
        printf("Date: %02d-%02d-%02d to: %s:%d\n",
            stSysTime.wMonth, 
            stSysTime.wDay, 
            stSysTime.wYear, 
            inet_ntoa(stDstAddr.sin_addr), 
            ntohs(stDstAddr.sin_port));
    }

    // Wait for the specified interval
    Sleep(nInterval*1000);

  }

  // Close the socket
  closesocket(hSocket);

  // Tell WinSock we're leaving
  WSACleanup();

  return 0;
}