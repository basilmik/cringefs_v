#include "stdio.h"
#include "cfs.h"
#define REFORMAT 0

int main()
{

	printf("%d\n", sizeof(cfs_meta));
	
#if REFORMAT
	cfs_init("container.txt");
	cfs_shutdown();
	getch();
#else
	cfs_format("container.txt");


	cfs_init("container.txt");
	create_file("f1");
	write_file("test.txt", "f1");
	cfs_shutdown();
	getch();

	cfs_init("container.txt");
	create_file("f2");	
	write_file("test2.txt", "f2");
	cfs_shutdown();

	getch();	
	cfs_init("container.txt");
	create_file("f3");
	write_file("test3.txt", "f3");
	cfs_shutdown();

	getch();
	cfs_init("container.txt");
	write_file("test4.txt", "f1");
	cfs_shutdown();
	
	getch();
	cfs_init("container.txt");
	create_file("f4");
	write_file("test6.txt", "f4");
	cfs_shutdown();

	/*getch();
	cfs_init("container.txt");
	write_file("test5.txt", "f2");
	cfs_shutdown();*/

	//getch();
	//cfs_init("container.txt");
	//cfs_shutdown();

	//getch();
	//getch();



#endif

	return 0;
}