#include "stdafx.h"
#include "Networking.h"
#include "Logging.h"
#include "Globals.h"

void * handlers[MESSAGE_TYPES::MAX]{};
bool CreateConnection(WCHAR * ipaddr, WCHAR * port)
{
	PADDRINFOW address;
	const ADDRINFOW hints{};
	if (int ret = GetAddrInfoW(ipaddr, port, &hints, &address) == 0)
	{
		SOCKET soc = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
		if (soc != INVALID_SOCKET)
		{
			int res = connect(soc, address->ai_addr, (int)address->ai_addrlen);
			if (res != SOCKET_ERROR)
			{
				InfoLog(L"Connected to %s:%s", ipaddr, port);
				g_Info.conn = soc;
				g_Info.ConnectionInfo.pcwIpAddr = ipaddr;
				g_Info.ConnectionInfo.pcwPort = port;
				FreeAddrInfoW(address);
				return true;
			}
		}
		else
		{
			ErrorLog(L"Failed to create a socket, [0x%x]", WSAGetLastError());
			FreeAddrInfoW(address);
		}
	}
	else
	{
		ErrorLog(L"GetAddrInfo failed with param: [0x%x]", ret);
	}
	return false;
}


bool reconnect()
{
	for (UINT nConnectionAttempts = 0; nConnectionAttempts < g_Info.Settings.uConnectionAttempts; nConnectionAttempts++)
	{
		if (!CreateConnection(g_Info.ConnectionInfo.pcwIpAddr, g_Info.ConnectionInfo.pcwPort))
		{
			InfoLog(L"Failed to reconnect on attempt #%d, sleeping %d seconds", nConnectionAttempts + 1, g_Info.Settings.uSleepDuration);
			Sleep(g_Info.Settings.uSleepDuration * 1000);
		}
		else
		{
			return true;
		}
	}
	return false;
}

bool BuildPacketAndSend(MESSAGE_TYPES eMsgType, HEAD_PACKET * pPacketHead, uint32_t uPacketSize)
{
	pPacketHead->msg = eMsgType;
	pPacketHead->signature = g_Info.Settings.uSignature;
	pPacketHead->size = uPacketSize;
startSend:
	UINT bytesSent = send(g_Info.conn, (char *)pPacketHead, uPacketSize, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		ErrorLog(L"Failed to send [0x%x] bytes, error code [0x%x]", uPacketSize, err);
		if (err == WSAECONNRESET || WSAENOTSOCK)
		{
			ErrorLog(L"Lost connection, attempting to reconnect");
			if (!reconnect())
			{
				return false;
			}
			goto startSend;
		}
	}
	return true;
}

bool RegisterHandler(MESSAGE_TYPES handler, void * pFunc)
{
	if (handler < MESSAGE_TYPES::MAX)
	{
		InfoLog(L"Handler ID: [0x%x] registered to [0x%p]", handler, pFunc);
		handlers[handler] = pFunc;
		return true;
	}
	ErrorLog(L"Handler ID: [0x%x] out of range", handler);
	return false;
}


bool ValidatePacket(PHEAD_PACKET packet)
{
	if (packet->signature != g_Info.Settings.uSignature)
	{
		InfoLog(L"Packet signature [%x] didn't match global signature [%x]", packet->signature, g_Info.Settings.uSignature);
		return false;
	}

	if (packet->msg >= MESSAGE_TYPES::MAX)
	{
		InfoLog(L"Packet message type [%d] outside of max range [%d]", packet->msg, MESSAGE_TYPES::MAX);
		return false;
	}
	return true;
}



DWORD ListenThread(LPVOID)
{
	for (;;)
	{
		HEAD_PACKET head{};
		int bytesRead = recv(g_Info.conn, (char *)&head, sizeof(head), MSG_PEEK);
		if(bytesRead != SOCKET_ERROR)
		{
			if (ValidatePacket(&head))
			{
				InfoLog(L"Valid packet [%d] received", head.msg);
				pCall call = (pCall)handlers[head.msg];
				call();
			}
		}
		else
		{
			int err = WSAGetLastError();
			if (err == WSAECONNRESET)
			{
				InfoLog(L"Lost connection to server, attempting to reconnect");
				if (!reconnect())
				{			
					return false;
				}
			}
		}
	}
}

bool StartListening()
{
	if (CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ListenThread, NULL, NULL, NULL))
	{
		return true;
	}
	return false;
}


bool NetworkStartup()
{
	WSAData wsaData;
	if (int ret = WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorLog(L"Failed to start WSAStartup [0x%x]", WSAGetLastError());
		return false;
	}
}
