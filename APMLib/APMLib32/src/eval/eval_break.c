#include "eval.h"

// Keyboard was hit. Pause if it was "Q".
void eval_break(void)
{
	if (_getch() != 'Q')
		return;

	while (_kbhit())
		_getch();

	printf("Interrupted: Press X to terminate, or ENTER to continue.\n");

	if (_getch() == 'X')
	{
		printf("Program terminated \n");
		exit(0);
	}
}
