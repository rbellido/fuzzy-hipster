#include "server_net.h"

using namespace std;

/*
* Creates a server. Returns the listening socket.
*/
SOCKET createServer(WSADATA *wsaData, int protocol)
{
	int res;
	SOCKADDR_IN addr;
	SOCKET listenSocket;

	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );

	if ((res = WSAStartup(wVersionRequested, wsaData)) != 0)
	{
		cerr << "WSAStartup falied with error " << res << endl;
		WSACleanup();
		return NULL;
	}

    if ((listenSocket = WSASocket(AF_INET, (protocol == TCP) ? SOCK_STREAM : SOCK_DGRAM , 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		cerr << "Failed to get a socket with error " << WSAGetLastError() << endl;
		return NULL;
	}

    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    addr.sin_port           = htons((protocol == TCP) ? TCPPORT : UDPPORT);

	if (bind(listenSocket, (PSOCKADDR) &addr, sizeof(addr)) == SOCKET_ERROR)
	{
		cerr << "bind() falied with error " << WSAGetLastError() << endl;
		return NULL;
	}

    if (protocol != UDP)
    {
        if (listen(listenSocket, 5))
        {
            cerr << "listen() falied with error " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            return NULL;
        }
    }

	ULONG nonblock = 1;
	if(ioctlsocket(listenSocket, FIONBIO, &nonblock) == SOCKET_ERROR)
	{
		cerr << "ioctlsocket() falied with error " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		return NULL;
	}
	
	return listenSocket;
}