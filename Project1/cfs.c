
#include "cfs.h"
FILE* cfs_container;
cfs_super_block cfs_sb;


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

int inc_num_meta_rec()
{
	cfs_sb.num_of_meta_rec += 1;
	return 0;
	// if > than size return error?
}

int get_meta_offset_from_end(int _meta_idx) // 64 (block size) / 8 (meta size) = num of meta in block
{
	int res = (_meta_idx + 1) * sizeof(cfs_meta);
	return res;
}



int setpos_to_block_by_idx(int _idx)
{
	fseek(cfs_container, _idx * CFS_BLOCK_SIZE, SEEK_SET);
}

int read_block_by_idx(cfs_block_ptr _block_ptr, int _idx)
{
	setpos_to_block_by_idx(_idx); // fseek(cfs_container, _idx * CFS_BLOCK_SIZE, SEEK_SET);
	fread(_block_ptr, sizeof(cfs_block), 1, cfs_container);
	return 0;
}

int read_meta_by_idx(cfs_meta_ptr _meta_ptr, int _idx)
{
	fseek(cfs_container, -get_meta_offset_from_end(_idx), SEEK_END);
	fread(_meta_ptr, sizeof(cfs_meta), 1, cfs_container);
	return 0;
}

int find_meta_by_fname(cfs_meta_ptr _meta_ptr, char* _fname)
{
	for (int i = 0; i < cfs_sb.num_of_meta_rec; i++)
	{
		read_meta_by_idx(_meta_ptr, i);
		if (strcmp(_meta_ptr->f_path, _fname) == 0)
		{
			return 0;
		}
	}

	return -1; // not found
}




int cfs_init(char* _path) 
{
	cfs_container = fopen(_path, "r+b");
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
		read_block_by_idx(block_read_ptr, i);
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






int update_size_by_path(char* _path, int _content_size)
{
	// check size -> resize -> change start_block_idx

	cfs_meta upd_meta;
	if (find_meta_by_fname(&upd_meta, _path) == -1)
	{
		return -1; // not found by such name
	}

	if (upd_meta.content_size != _content_size)
		upd_meta.content_size = _content_size;


	int meta_offset_from_end = get_meta_offset_from_end(upd_meta.meta_idx);


	fseek(cfs_container, -meta_offset_from_end, SEEK_END);

	fwrite(&upd_meta, sizeof(cfs_meta), 1, cfs_container);

	return 0;
}

int update_size_by_meta(cfs_meta_ptr upd_meta, int _content_size)
{

	if (upd_meta->content_size != _content_size)
		upd_meta->content_size = _content_size;


	int meta_offset_from_end = get_meta_offset_from_end(upd_meta->meta_idx);


	fseek(cfs_container, -meta_offset_from_end, SEEK_END);

	fwrite(&upd_meta, sizeof(cfs_meta), 1, cfs_container);

	return 0;
}



int add_new_meta(char* _path)
{
	cfs_meta new_meta;
	
	new_meta.content_size = 0; 
	new_meta.start_block_idx = get_empty_block_idx();

	strcpy(&(new_meta.f_path), _path);

	inc_empty_block_idx();

	new_meta.meta_idx = cfs_sb.num_of_meta_rec;
	int meta_offset_from_end = get_meta_offset_from_end(cfs_sb.num_of_meta_rec);

	inc_num_meta_rec();

	fseek(cfs_container, -meta_offset_from_end, SEEK_END);
	
	fwrite(&new_meta, sizeof(cfs_meta), 1, cfs_container);

	return 0;
}




int copy_file_to_cfs(char* _src, char* _dst_at_cfs)
{
	FILE* src_fd = fopen(_src, "rb");
	if (src_fd == NULL)
	{
		return -1;
	}

	fseek(src_fd, 0, SEEK_END);
	int scr_size = ftell(src_fd);
	rewind(src_fd);


	cfs_meta dts_file_meta; 
	if (find_meta_by_fname(&dts_file_meta, _dst_at_cfs) == -1)
		return -1; // err
	
	int writable_size = 0;
	if (dts_file_meta.content_size == 0) writable_size = scr_size;
	else
	writable_size = (scr_size > dts_file_meta.content_size) ? dts_file_meta.content_size : scr_size;
	char ch = 0;
	
	setpos_to_block_by_idx(dts_file_meta.start_block_idx);
	for (int i = 0; i < writable_size; i++)
	{
		fread(&ch, sizeof(char), 1, src_fd);
		fwrite(&ch, sizeof(char), 1, cfs_container);
		
	}

	update_size_by_meta(&dts_file_meta, writable_size);

	fclose(src_fd);

	return 0;
}

int create_file(char* _name)
{
	add_new_meta(_name);
	return 0;
}