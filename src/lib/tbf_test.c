#include <stdio.h>
#include <stdlib.h>
#include "tbf.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define   BUF_SIZE	100

int main(int argc, char *argv[])
{
	assert(argc >= 2);
	int fd = open(argv[1], O_RDONLY);	

	if (fd < 0) {
		perror("open file failed");
		exit(-1);
	}
	
	char buf[BUF_SIZE] = {0};
	
	struct tbf_st *ptbf = tbf_init(10, 50);
	int n, len, ret, pos;

	while (1) {
		n = read(fd, buf, BUF_SIZE);
		if (n < 0) {
			perror("read");
			break;
		}else if (n == 0) {
			printf("endof file\n");
			break;
		}
		pos = 0;
		while (n > 0) {
			len = write(STDOUT_FILENO, buf + pos, tbf_get_token(ptbf, 10));
			fflush(stdout);
			if (len < 0) {
				perror("write");
				goto out;
			}
			pos += len;
			n -= len;
		}
	}
out:
	close(fd);
	tbf_destroy(ptbf);
	return 0;
}
