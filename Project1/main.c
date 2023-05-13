#include "stdio.h"
#include "cfs.h"
#define REFORMAT 1
int main()
{
#if REFORMAT 
	
	cfs_format("container.txt");

	cfs_init("container.txt");

	create_file("f1");

	import_file("test.txt", "f1");

	create_file("f2");
	import_file("test2.txt", "f2");

	create_file("f3");
	import_file("test3.txt", "f3");


	create_file("f4");
	import_file("test6.txt", "f4");

	cfs_shutdown();

	getch();

	cfs_init("container.txt");
	delete_file("f1", 1);
	delete_file("f3", 1);
	export_file("f1.txt", "f1");
	//export_file("f2.txt", "f2");
	export_file("f3.txt", "f3");
	export_file("f4.txt", "f4");
 	cfs_pack();
	cfs_shutdown();

#else
	cfs_format("container.txt");

	cfs_init("container.txt");

	create_file("f1");
	
	import_file("test.txt", "f1");

	create_file("f2");	
	import_file("test2.txt", "f2");

	create_file("f3");
	import_file("test3.txt", "f3");



	create_file("f4");
	import_file("test6.txt", "f4");

	cfs_shutdown();

#endif
	/*getch();
	cfs_init("container.txt");
	import_file("test5.txt", "f2");
	cfs_shutdown();*/

	//getch();
	//cfs_init("container.txt");
	//cfs_shutdown();

	//getch();
	//getch();


	return 0;
}