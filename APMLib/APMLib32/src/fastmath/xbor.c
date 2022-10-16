#include "fastmath.h"

// Subtract one from [EAX] and propogate ad infinitum
void xbor(long* a)
{
	__asm
	{
		mov    eax,a
xbor_:
		sub    dword ptr[EAX],1
        lea    EAX,[EAX+4]
        jc     xbor_
	}
}
