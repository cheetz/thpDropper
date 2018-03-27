#pragma once
#include "Networking.h"
#include "Logging.h"
#include "Globals.h"
#include "ReflectiveLoader.h"

enum LOAD_TYPE
{
	PIC,
	REFLECTIVE,
	MAX_LOAD_TYPE
};

typedef struct _blob_packet
{
	HEAD_PACKET head;
	LOAD_TYPE loadType;
	uint32_t payloadLen;
	uint32_t checksum;
} BLOB_PACKET, *PBLOB_PACKET;

void LoadBlobHandler();