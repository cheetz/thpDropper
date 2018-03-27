#include "stdafx.h"
#include "Globals.h"


GLOBALS g_Info = {
	NULL, // conn socket
	L"10.0.0.128", //hostname
	L"4444", //port
	10, // connection attempts
	25, // sleep duration
	0x1337, // packet signature
	500000, // max payload size
};