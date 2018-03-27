#include "stdafx.h"
#include "Logging.h"

#ifdef _DEBUG

WCHAR * Symbols = L"!+";


void DebugLog(LOG_LEVEL level, WCHAR * x, ...)
{
	va_list args;
	wprintf(L"[%c] ", Symbols[level]);
	va_start(args, x);
	vwprintf(x, args);
	va_end(args);
	wprintf(L"\n");
}


#endif