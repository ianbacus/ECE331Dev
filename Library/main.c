#include "DCC.h"

/*
Sample main function
*/

int main()
{
	int counter = 0;
	DCC_init();
	while(1)
	{
		set_engineB(counter);
		set_engineA(counter);
		set_turnouts(counter);
		delay(1);
		counter++;
	}
}
