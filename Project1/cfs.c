
#include "cfs.h"
FILE* cfs_container;
cfs_super_block cfs_sb;

int update_sb()
{
	
	fseek(cfs_container, 0, SEEK_SET);
	fwrite(&cfs_sb, sizeof(cfs_super_block), 1, cfs_container);
	return 0;
}

int get_empty_block_idx()
{
	return cfs_sb.first_empty_block;
}



int inc_num_meta_rec()
{
	cfs_sb.num_of_meta_rec += 1;
	return 0;
	// if > than size return error?
}

int update_meta_end_idx()
{
	int delta = CFS_BLOCK_SIZE / sizeof(cfs_meta);

	int num_of_full = cfs_sb.num_of_meta_rec / delta;
	cfs_sb.meta_end_idx = num_of_full;

	if (num_of_full * delta < cfs_sb.num_of_meta_rec)
	{
		// some are taking one not fully
		cfs_sb.meta_end_idx = num_of_full + 1;
	}
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
	printf("first_empty_block:  %d\n", cfs_sb.first_empty_block);
	printf("\n\n");

	/*cfs_block_ptr block_read_ptr = (cfs_block_ptr) calloc(1, sizeof(cfs_block));

	for (int i = 0; i < 4; i++)
	{
		printf("\nblock %d\n", i);
		read_block_by_idx(block_read_ptr, i);
		for (int j = 0; j < CFS_BLOCK_SIZE; j++)
		{
			printf("%x ", block_read_ptr->content[j]);
		}
	}*/


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


	fmt_sb.num_of_meta_rec = 0;
	fmt_sb.first_empty_block = 1;

	fwrite(&fmt_sb, sizeof(cfs_super_block), 1, fmt);

	int amount_to_format_at_sb = CFS_BLOCK_SIZE - sizeof(cfs_super_block);

	for (int i = 0; i < amount_to_format_at_sb; i++) // formatting sb to size of block
	{
		char fill = NULL;
		fwrite(&fill, sizeof(char), 1, fmt);
	}

	cfs_block empty_block = { -1, 0 };

	// creating CFS_NUMBER_OF_BLOCKS - 1(sb) empty blocks
	for (int i = 1; i < CFS_NUMBER_OF_BLOCKS; i++) 
	{
		//char empty = i;
		//for(int j = 0; j < CFS_BLOCK_SIZE; j++)

		fwrite(&empty_block, sizeof(cfs_block), 1, fmt);
	}

	fclose(fmt);

	return 0;
}






int update_size_by_path(char* _path, int _content_size)
{
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

int update_size_by_meta(cfs_meta_ptr upd_meta_ptr, int _content_size)
{
	
	if (upd_meta_ptr->content_size != _content_size)
		upd_meta_ptr->content_size = _content_size;


	int meta_offset_from_end = get_meta_offset_from_end(upd_meta_ptr->meta_idx);


	fseek(cfs_container, -meta_offset_from_end, SEEK_END);

	fwrite(upd_meta_ptr, sizeof(cfs_meta), 1, cfs_container);

	return 0;
}

int update_first_empty_idx()
{
	cfs_sb.first_empty_block = get_next_empty();
}

int get_next_empty()
{
	int stat = 0;

	int idx = cfs_sb.first_empty_block + 1;
	while (1)
	{
		stat = get_block_next_idx(idx);
		if (stat != -1)
			idx++;
		else
		{
			cfs_sb.first_empty_block = idx;
			break;
		}
	}

	return cfs_sb.first_empty_block;
}

int add_new_meta(char* _path)
{
	cfs_meta new_meta = { 0 };
	
	new_meta.content_size = 0; 
	new_meta.start_block_idx = get_empty_block_idx();
	//update_first_empty_idx();
	strcpy(&(new_meta.f_path), _path);


	new_meta.meta_idx = cfs_sb.num_of_meta_rec;
	int meta_offset_from_end = get_meta_offset_from_end(cfs_sb.num_of_meta_rec);

	inc_num_meta_rec();

	update_meta_end_idx();
	update_sb();
	fseek(cfs_container, -meta_offset_from_end, SEEK_END);
	
	fwrite(&new_meta, sizeof(cfs_meta), 1, cfs_container);

	return 0;
}


int check_if_fits(cfs_meta_ptr _file_meta, int _new_size)
{
	return (_new_size <= _file_meta->content_size);
}

int get_num_meta_blocks()
{
	return cfs_sb.meta_end_idx;
}

int get_blocks_to_meta_space_in_bytes()
{
	int space = get_num_meta_blocks() - get_empty_block_idx();
	return space * CFS_BLOCK_SIZE;
}

int get_space_btw_block(int _idx1, int _idx2)
{
	cfs_meta fm1;
	cfs_meta fm2;
	read_meta_by_idx(&fm1, _idx1);
	read_meta_by_idx(&fm2, _idx2);

}

int check_extension(cfs_meta_ptr _file_meta, int _new_size)
{
	if (_file_meta->meta_idx == cfs_sb.num_of_meta_rec) // if this record is last
	{
		if (get_blocks_to_meta_space_in_bytes() >= _new_size)
			return 1; // extend here
		else
			return -1; // cannot fit!
	}
	else
	{
		cfs_meta next_file_meta;
		read_meta_by_idx(&next_file_meta, _file_meta->meta_idx); // deleted files not counted
		int space = next_file_meta.start_block_idx - _file_meta->meta_idx;
		space *= CFS_BLOCK_SIZE; // in bytes
		if (space >= _new_size)
		{
			return 1;
		}
	}
}


int copy_file_to_cfs(char* _src, char* _dst_at_cfs)
{
	cfs_meta dts_file_meta;
	if (find_meta_by_fname(&dts_file_meta, _dst_at_cfs) == -1) // no such destination
		return -1; // err


	FILE* src_fd = fopen(_src, "rb");
	if (src_fd == NULL)
	{
		return -1;
	}

	fseek(src_fd, 0, SEEK_END);
	int scr_size = ftell(src_fd);
	rewind(src_fd);
	int writable_size = 0;

	if (check_if_fits(&dts_file_meta, scr_size))
	{
		writable_size = scr_size;
	}
	else
	{

	}

	if (dts_file_meta.content_size == 0) 
		writable_size = scr_size;
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

int get_block_next_idx(int _cur_idx)
{
	int cur_pos = ftell(cfs_container);
	setpos_to_block_by_idx(_cur_idx);
	int read_next_value = 0;
	fread(&read_next_value, sizeof(int), 1, cfs_container);
	fseek(cfs_container, cur_pos, SEEK_SET);
	return read_next_value;

}




int write_file(char* _src, char* _dst_at_cfs)
{
	cfs_meta dst_file_meta;
	if (find_meta_by_fname(&dst_file_meta, _dst_at_cfs) == -1) // no such destination
		return -1; // err


	FILE* src_fd = fopen(_src, "rb");
	if (src_fd == NULL)
	{
		return -1;
	}

	fseek(src_fd, 0, SEEK_END);
	int scr_size = ftell(src_fd);
	rewind(src_fd);
	int writable_size = scr_size;

	int cur_block_idx = dst_file_meta.start_block_idx;
	int next_block_idx = get_block_next_idx(cur_block_idx);
	int num_of_blocks_needed = scr_size / (CFS_BLOCK_SIZE - 4);
	int size_taken = num_of_blocks_needed * (CFS_BLOCK_SIZE - 4); 
	
	if (scr_size > size_taken)
	{
		num_of_blocks_needed += 1;
	}

	int read_now_size = 0;
	//if (scr_size < dst_file_meta.content_size) 
	{
		char* buf;
		while (writable_size > 0)
		{
			read_now_size = (writable_size >= CRS_DATA_IN_BLOCK_SIZE) ? CRS_DATA_IN_BLOCK_SIZE : writable_size;
			printf("read_now_size %d\n", read_now_size);

			buf = calloc(read_now_size, sizeof(char));
			if (buf == NULL)
			{
				exit(-1); // err
			}

			if (scr_size >= dst_file_meta.content_size) // files becomes bigger
				if (next_block_idx <= 0)
				{
					next_block_idx = get_next_empty();
					//update_first_empty_idx();
				}

			setpos_to_block_by_idx(cur_block_idx);
			
			fread(buf, sizeof(char), read_now_size, src_fd);
			
			writable_size -= read_now_size;

			if (writable_size == 0) // this block is the last				
			{
				fwrite(&writable_size, sizeof(int), 1, cfs_container); // next idx = 0 -> sb meaning this one is last			
			}
			else // this block is not last						
			{
				fwrite(&next_block_idx, sizeof(int), 1, cfs_container);
			}

			fwrite(buf, sizeof(char), read_now_size, cfs_container);

			cur_block_idx = next_block_idx;
			next_block_idx = get_block_next_idx(cur_block_idx);
			
		}

		if (scr_size < dst_file_meta.content_size) // files becomes smaller
		{
			int consta = -1;
			while (cur_block_idx != 0) //  set not used as empty (next_idx = -1)
			{
				setpos_to_block_by_idx(cur_block_idx);
				fwrite(&consta, sizeof(int), 1, cfs_container);
				cur_block_idx = next_block_idx;
				next_block_idx = get_block_next_idx(cur_block_idx);
			}
		}
	}
	//else //  files becomes bigger
	//{
	//	char* buf;
	//	while (writable_size > 0)
	//	{
	//		read_now_size = (writable_size >= CRS_DATA_IN_BLOCK_SIZE) ? CRS_DATA_IN_BLOCK_SIZE : writable_size;
	//		printf("read_now_size %d\n", read_now_size);

	//		buf = calloc(read_now_size, sizeof(char));
	//		if (buf == NULL)
	//		{
	//			exit(-1); // err
	//		}
	//		if (next_block_idx <= 0)
	//		{
	//			next_block_idx = get_next_empty();
	//		}
	//		setpos_to_block_by_idx(cur_block_idx);

	//		fread(buf, sizeof(char), read_now_size, src_fd);
	//		writable_size -= read_now_size;

	//		if (writable_size == 0) // this block is the last				
	//		{
	//			fwrite(&writable_size, sizeof(int), 1, cfs_container); // next idx = 0 -> sb meaning this one is last			
	//		}
	//		else // this block is not last						
	//		{
	//			fwrite(&next_block_idx, sizeof(int), 1, cfs_container);
	//		}

	//		fwrite(buf, sizeof(char), read_now_size, cfs_container);


	//		cur_block_idx = next_block_idx;
	//		next_block_idx = get_block_next_idx(cur_block_idx);
	//	}
	//}


	update_size_by_meta(&dst_file_meta, scr_size);
	update_meta_end_idx();
	update_sb();

	fclose(src_fd);

}