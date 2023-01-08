#include <stdio.h>
#include <stdlib.h>

int read_file(FILE *fd)
{
	int c, nc = 0;
	while ((c = fgetc(fd)) != EOF) {
		putchar(c);
		++nc;
	}

	return nc;
}

int main(void)
{
	FILE *fd;
	int n;

	fd = fopen("/dev/pcd", "w+");
	if (!fd) {
		perror("file opening failed");
		return EXIT_FAILURE;
	}

	fputs("Hello, world!\n", fd);

	if (fseek(fd, 0, SEEK_SET) != 0) {
		perror("fseek error");
	}

	n = read_file(fd);
	printf("%d bytes read\n", n);

	if (fseek(fd, 1, SEEK_END) != 0) {
		perror("fseek error");
	}

	if (fseek(fd, -510, SEEK_END) != 0) {
		perror("fseek error");	
	}

	n = read_file(fd);
	printf("%d bytes read\n", n);

	if (fseek(fd, 0, SEEK_SET) != 0) {
		perror("fseek error");
	}


	if (fseek(fd, 10, SEEK_CUR) != 0) {
		perror("fseek error");
	}

	n = read_file(fd);
	printf("%d bytes read\n", n);

	fclose(fd);

	return 0;
}
