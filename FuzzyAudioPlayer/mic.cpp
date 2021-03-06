#include "mic.h"

using namespace std;
using namespace libZPlay;

ZPlay * netstream;
ZPlay * micplayer;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	startMicSession
--
-- DATE:		April 1, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Ronald Bellido
--
-- PROGRAMMER:	Ronald Bellido
--
-- INTERFACE:	int startMicSession()
--
-- RETURNS:		0 - Something failed, 1 - All ok
--				
--
-- NOTES:
This function will start microphone session
----------------------------------------------------------------------------------------------------------------------*/
int startMicSession()
{
	LPMICVARS micvar = (LPMICVARS) malloc(sizeof(MICVARS));
	WSADATA wsadata;
	micvar->micsocket = createServer(&wsadata, UDP, &micvar->micaddr);

	ZPlay * netstream = CreateZPlay();
	
	netstream->SetSettings(sidSamplerate, 44100);
	netstream->SetSettings(sidChannelNumber, 2);
	netstream->SetSettings(sidBitPerSample, 16);
	netstream->SetSettings(sidBigEndian, 1);

	int n;
	if (netstream->OpenStream(1, 1, &n, 1, sfPCM) == 0)
	{
		cerr << "Error opening a microphone stream: " << netstream->GetError() << endl;
		netstream->Release();
		closesocket(micvar->micsocket);
		return 0;
	}

	micplayer = CreateZPlay();

	if (micplayer->OpenFile("wavein://", sfAutodetect) == 0)
	{
		cerr << "Error in OpenFile: " << micplayer->GetError() << endl;
		micplayer->Release();
		closesocket(micvar->micsocket);
		return 0;
	}

	micplayer->SetCallbackFunc(micCallback, (TCallbackMessage) (MsgWaveBuffer | MsgStop), (VOID*) micvar); //setup callback function whenever a mic picks up a sound

	micplayer->Play(); //start listening to the mic

	while (1)
	{
		char * buffer = new char[65507];
		int size = sizeof(micvar->micaddr);
		int bytesrecvd;
		if ((bytesrecvd = recvfrom(micvar->micsocket, buffer, 65507, 0, (SOCKADDR*)&micvar->micaddr, &size)) == -1)
		{
			cerr << "Error in recvfrom: " << WSAGetLastError() << endl;
			closesocket(micvar->micsocket);
			break;
		}



		netstream->PushDataToStream(buffer, bytesrecvd);
		delete buffer;


		if (bytesrecvd == 0) {
			//cout << "asdfjaksldfj zero  " << endl;
			closesocket(micvar->micsocket);
			break;
		}


		netstream->Play(); //send the received data into the stream
		
		TStreamStatus status;
		micplayer->GetStatus(&status);
		if (status.fPlay == 0)
			break; //microphone not playing anymore

		TStreamTime pos;
		micplayer->GetPosition(&pos);
		cout << "Pos: " << pos.hms.hour << " " << pos.hms.minute << " " <<
			pos.hms.second << " " << pos.hms.millisecond << endl;
	}

	micplayer->Release();
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:	micCallbakc
--
-- DATE:		April 1, 2013
--
-- REVISIONS:	
--
-- DESIGNER:	Ronald Bellido
--
-- PROGRAMMER:	Ronald Bellido
--
-- INTERFACE:	int __stdcall micCallback(void * instance, void * user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
--
-- RETURNS:		Dependent on the type of the TCallbackMessage (message) passed. See NOTES below.
--				
--
-- NOTES:
This function will mainly listen for the MsgWaveBuffer message, which is a message for when a decoding thread is
ready to send data to the soundcard.
	Important parameters:
		param1 - pointer to memory PCM buffer
		param2 - number of bytes in PCM data buffer
	Returns:
		0 - send data to soundcard
		1 - skip sending data to soundcard
		2 - stop playing

For MsgStop, all the parameters and the return type are not used. The message is used after a song stops playing.
----------------------------------------------------------------------------------------------------------------------*/
int __stdcall micCallback(void * instance, void * user_data, TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	MICVARS * micvar = (MICVARS *) user_data;

	if ( message == MsgStop)
	{
		micplayer->Stop();
		netstream->Stop();
		closesocket(micvar->micsocket);
		return 2;
	}
	
	if (sendto(micvar->micsocket, (const char *) param1, param2, 0, (const SOCKADDR *)& micvar->micaddr, sizeof(micvar->micaddr)) < 0)
	{
		cerr << "Error in sendto: " << GetLastError() << endl;
		
		return 1;
	}

	return 1;
}