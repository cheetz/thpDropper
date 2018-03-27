#pragma once
#include "stdafx.h"

#ifdef _DEBUG

enum LOG_LEVEL
{
	INFO,
	ERR
};

#define InfoLog(x, ...) DebugLog(LOG_LEVEL::INFO, x, __VA_ARGS__)
#define ErrorLog(x, ...) DebugLog(LOG_LEVEL::ERR, x, __VA_ARGS__)
void DebugLog(LOG_LEVEL level, WCHAR * x, ...);
#else
#define InfoLog(...) {}
#define ErrorLog(...) {}
#endif
