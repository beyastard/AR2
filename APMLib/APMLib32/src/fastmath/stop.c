#include "fastmath.h"

// Stop. Then exit if 'X' pressed
void stop(void)
{
	if (_getch() == 'X')
		exit(0);
}
