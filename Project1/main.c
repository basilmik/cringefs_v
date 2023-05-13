#include "stdio.h"
#include "cfs.h"
#define REFORMAT 1
int main()
{
#if REFORMAT 
	
	cfs_init("container.txt");
	read_file("f1.txt", "f1");
	read_file("f2.txt", "f2");
	read_file("f3.txt", "f3");
	read_file("f4.txt", "f4");
	cfs_shutdown();

#else
	cfs_format("container.txt");

	cfs_init("container.txt");

	create_file("f1");
	write_file("test.txt", "f1");

	create_file("f2");	
	write_file("test2.txt", "f2");

	create_file("f3");
	write_file("test3.txt", "f3");



	create_file("f4");
	write_file("test6.txt", "f4");


#endif
	/*getch();
	cfs_init("container.txt");
	write_file("test5.txt", "f2");
	cfs_shutdown();*/

	//getch();
	//cfs_init("container.txt");
	//cfs_shutdown();

	//getch();
	//getch();


	return 0;
}