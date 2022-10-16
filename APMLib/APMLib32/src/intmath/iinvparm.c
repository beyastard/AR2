#include "intmath.h"

// Invalid parameter
void iinvparm(char* c)
{
	printf("Invalid parameter in %s call\n", c);
	give_up();
}
