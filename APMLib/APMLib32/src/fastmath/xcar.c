#include "fastmath.h"

// Add one to [EAX] and propogate ad infinitum
void xcar(long* a)
{
	__asm
	{
		mov    eax,a
xcar_:
		add    dword ptr[EAX],1
        lea    EAX,[EAX+4]
        jc     xcar_
	}
}
