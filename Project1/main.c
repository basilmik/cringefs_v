#include "stdio.h"
#include "cfs.h"

int main()
{

	printf("%d\n", sizeof(cfs_meta));
	cfs_format("container.txt");
	cfs_init("container.txt");
	cfs_shutdown();

	return 0;
}