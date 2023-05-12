
#include "cfs.h"

int cfs_init(char* _path) {
	cfs_container = fopen(_path, "r");
	if (cfs_container == NULL)
	{
		printf("error opening file\n");
		return -1; //  error
	}
	fread(&cfs_sb, sizeof(cfs_super_block), 1, cfs_container);
	printf("magic %d\n", cfs_sb.magic);
	printf("meta_end_idx %d\n", cfs_sb.meta_end_idx);
	printf("sb_end_idx %d\n", cfs_sb.sb_end_idx);

	return 0;
};

int cfs_shutdown() {
	if (cfs_container == NULL)
	{
		printf("error closing file\n");
		return -1; //  error
	}
	fclose(cfs_container);
	return 0;
};

int cfs_format(char* _path)
{
	FILE* fmt = fopen(_path, "w+");
	if (fmt == NULL)
	{
		printf("error opening file for formatting\n");
		return -1; //  error
	}

	cfs_super_block fmt_sb;
	fmt_sb.magic = CFS_MAGIC;
	fmt_sb.meta_end_idx = 0;
	fmt_sb.sb_end_idx = 1;

	fwrite(&fmt_sb, sizeof(cfs_super_block), 1, fmt);
	printf("formatted\n");

	fclose(fmt);



	return 0;
}

