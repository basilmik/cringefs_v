#include "stdio.h"
#include "cfs.h"

int main()
{
	printf("hi!");
	cfs_format("container.txt");
	cfs_init("container.txt");
	cfs_shutdown();

	return 0;
}