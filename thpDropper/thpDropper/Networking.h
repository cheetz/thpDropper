#pragma once
#include <Ws2tcpip.h>
typedef void(*pCall)();

enum MESSAGE_TYPES
{
	LOAD_BLOB,
	MAX
};

typedef struct _head_packet {
	uint32_t signature;
	MESSAGE_TYPES msg;
	uint32_t size;
} HEAD_PACKET, *PHEAD_PACKET;


bool BuildPacketAndSend(MESSAGE_TYPES eMsgType, HEAD_PACKET * pPacketHead, uint32_t uPacketSize);
bool RegisterHandler(MESSAGE_TYPES handler, void * pFunc);
bool NetworkStartup();
bool StartListening();
bool reconnect();