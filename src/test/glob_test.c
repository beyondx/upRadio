#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <string.h>
#include <errno.h>

int main()
{
	int iret;
	glob_t glob_info;

	iret = glob("*.c", 0, NULL, &glob_info);
	
	if (iret  != 0) {
		fprintf(stderr, "glob failed, %s\n", strerror(errno));
		exit(-1);
	}
	
	int i;
	for (i = 0; i < glob_info.gl_pathc; ++i) {
		printf("glob_info.gl_pathv[%d] = %s\n", i, glob_info.gl_pathv[i]);
	}

	globfree(&glob_info);

	return 0;
}
