#include <stdio.h>
#include "utils.h"

int main()
{
	DEBUG("hello this DEBUG  demo\n");

	CHK_EXIT(1 - 2);

	int n;
	
	CHK_RET_EXIT(n, 2 + 3);

	DEBUG("n = %d\n", n);

	return 0;
}
