
#include "cfs.h"
FILE* cfs_container;
cfs_super_block cfs_sb;

int read_block(cfs_block_ptr _block_ptr, int _idx)
{
	fseek(cfs_container, _idx * CFS_BLOCK_SIZE, SEEK_SET);	
	fread(_block_ptr, sizeof(cfs_block), 1, cfs_container);
	return 0;
}

int cfs_init(char* _path) 
{
	cfs_container = fopen(_path, "rb");
	if (cfs_container == NULL)
	{
		printf("error opening file\n");
		return -1; //  error
	}

	fread(&cfs_sb, sizeof(cfs_super_block), 1, cfs_container);

	printf("magic %d\n", cfs_sb.magic);
	printf("meta_end_idx %d\n", cfs_sb.meta_end_idx);
	printf("sb_end_idx %d\n", cfs_sb.sb_end_idx);
	printf("metarecords:  %d\n", cfs_sb.num_of_meta_rec);


	cfs_block_ptr block_read_ptr = (cfs_block_ptr) calloc(1, sizeof(cfs_block));

	for (int i = 0; i < 4; i++)
	{
		printf("\nblock %d\n", i);
		read_block(block_read_ptr, i);
		for (int j = 0; j < CFS_BLOCK_SIZE; j++)
		{
			printf("%x ", block_read_ptr->content[j]);
		}
	}

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
	FILE* fmt = fopen(_path, "w+b");
	if (fmt == NULL)
	{
		printf("error opening file for formatting\n");
		return -1; //  error
	}

	cfs_super_block fmt_sb;

	fmt_sb.magic = CFS_MAGIC;
	fmt_sb.meta_end_idx = 3;

	fmt_sb.sb_end_idx = 1;
	fmt_sb.empty_start_idx = 1;

	fmt_sb.num_of_meta_rec = 0;


	fwrite(&fmt_sb, sizeof(cfs_super_block), 1, fmt);

	int amount_to_format = CFS_BLOCK_SIZE - sizeof(cfs_super_block);

	for (int i = 0; i < amount_to_format; i++)
	{
		char fill = NULL;
		fwrite(&fill, sizeof(char), 1, fmt);
	}

	// creating CFS_NUMBER_OF_BLOCKS - 1 blocks
	for (int i = 1; i < CFS_NUMBER_OF_BLOCKS; i++) 
	{
		char empty = i;
		for(int j = 0; j < CFS_BLOCK_SIZE; j++)
			fwrite(&empty, sizeof(char), 1, fmt);
	}

	fclose(fmt);

	return 0;
}

int get_empty_block_idx()
{
	// got to check though meta records
	// now returns just last empty, no size check
	return cfs_sb.empty_start_idx;

}

int inc_empty_block_idx()
{
	cfs_sb.empty_start_idx += 1;
	return 0;
	// if > than size return error?
}

int get_meta_offset(int _meta_idx) // 64 (block size) / 8 (meta size) = num of meta in block
{

}


int add_new_meta()
{
	cfs_meta new_meta;
	new_meta.size = 16; // random
	new_meta.start_block_idx = get_empty_block_idx();
	return 0;
}

