#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define MAX_SIZE	256
#define SUCCESS		0
#define ERROR		-1
#define DEVNAME		"/dev/hello"

#define DEBUG /* Can also be defined via Makefile */
#ifdef DEBUG
# define dbg(string, args...) printf(string, ##args)
#else
# define dbg(string, args...)
#endif

int main()
{
	int fd = ERROR;
	char buffer[32] = {0};

	fd = open(DEVNAME, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		printf("Device cannot be opened!!! \n");
		exit(1);
	}
	else
	{
		write(fd, buffer, 10);
		memset(buffer, 10, 0);
		read(fd, buffer, 10);

		close(fd);
	}

	return 0;
}
