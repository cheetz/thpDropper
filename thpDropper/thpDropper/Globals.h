#pragma once
#include "stdafx.h"

typedef struct _globals
{
	SOCKET conn;

	struct {
		WCHAR* pcwIpAddr;
		WCHAR* pcwPort;
	} ConnectionInfo;

	struct {
		UINT uConnectionAttempts;
		UINT uSleepDuration;
		uint32_t uSignature;
		UINT uMaxPayloadSize;
	} Settings;
} GLOBALS, *PGLOBALS;

extern GLOBALS g_Info;