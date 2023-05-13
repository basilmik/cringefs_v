#include "cfs.h"
FILE* cfs_container;
cfs_super_block cfs_sb;


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

int get_first_empty()
{
	return cfs_sb.first_empty_block;
}

int get_next_empty()
{
	if (cfs_sb.first_empty_block == -1)
		return -1;
	int stat = 0;

	int idx = cfs_sb.first_empty_block + 1;
	while (idx < CFS_NUMBER_OF_BLOCKS - cfs_sb.meta_end_idx)
	{
		stat = get_block_next_idx(idx);
		if (stat != -1)
			idx++;
		else
		{
			//cfs_sb.first_empty_block = idx;
			//return cfs_sb.first_empty_block;
			return idx;
			break;
		}
	}

	return -1;
}

int get_block_next_idx(int _cur_idx)
{
	if (_cur_idx == -1)
		return _cur_idx;

	int cur_pos = ftell(cfs_container);
	setpos_to_block_by_idx(_cur_idx);
	int read_next_value = 0;
	fread(&read_next_value, sizeof(int), 1, cfs_container);
	fseek(cfs_container, cur_pos, SEEK_SET);

	return read_next_value;
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




int update_sb()
{

	fseek(cfs_container, 0, SEEK_SET);
	fwrite(&cfs_sb, sizeof(cfs_super_block), 1, cfs_container);
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

int create_file(char* _name)
{
	if (get_first_empty() == -1)
	{
		printf("no space left for new writing file! exiting\n");
		return -1;
	}

	if (get_block_next_idx(CFS_NUMBER_OF_BLOCKS - cfs_sb.meta_end_idx - 1) != -1)
	{
		printf("cannot create new meta! exiting\n");
		return -1;
	}

	add_new_meta(_name);
	return 0;
}

void set_empty_block_idx(int _idx)
{
	cfs_sb.first_empty_block = _idx;
	update_sb();
}


int delete_block_list(int _start_idx)
{
	int consta = -1;
	int next_block_idx = get_block_next_idx(_start_idx);

	
		set_empty_block_idx(_start_idx);

	while (_start_idx != 0) //  set not used as empty (next_idx = -1)
	{
		if (_start_idx != -1 && get_first_empty() > _start_idx)
			set_empty_block_idx(_start_idx);

		setpos_to_block_by_idx(_start_idx);
		fwrite(&consta, sizeof(int), 1, cfs_container);
		_start_idx = next_block_idx;
		next_block_idx = get_block_next_idx(_start_idx);
	}
}

void write_block_next_idx(int* _next_idx, int _block_idx)
{
	setpos_to_block_by_idx(_block_idx);
	fwrite(_next_idx, sizeof(int), 1, cfs_container); // next idx = 0 -> sb meaning this one is last	
}

int write_block(int* _next_idx, char* _buf, int _buf_size, int _block_idx)
{
	setpos_to_block_by_idx(_block_idx);
	write_block_next_idx(_next_idx, _block_idx);
	fwrite(_buf, sizeof(char), _buf_size, cfs_container);
	return 0;
}

int get_file_size(FILE* _fp)
{
	fseek(_fp, 0, SEEK_END);
	int size = ftell(_fp);
	rewind(_fp);
	return size;
}



int write_file(char* _src, char* _dst_at_cfs)
{
	if (delete_file(_dst_at_cfs) == -1)
		return -1; //  error

	cfs_meta dst_file_meta;
	if (find_meta_by_fname(&dst_file_meta, _dst_at_cfs) == -1) // no such destination
	{
		return -1; // err
	}
	
	/*if (get_first_empty() == -1)
	{
		printf("no space left! exiting\n");
		return -1;
	}*/


	FILE* src_fd = fopen(_src, "rb");
	if (src_fd == NULL)
	{
		return -1;
	}


	int scr_size = get_file_size(src_fd);
	int writable_size = scr_size;
	int wrote_size = 0;
	int cur_block_idx = dst_file_meta.start_block_idx;
	
	int next_block_idx = 0;
	

	int last_block_idx = cur_block_idx;

	int num_of_blocks_needed = scr_size / (CRS_DATA_IN_BLOCK_SIZE);
	int size_taken = num_of_blocks_needed * (CRS_DATA_IN_BLOCK_SIZE);

	if (scr_size > size_taken)
	{
		num_of_blocks_needed += 1;
	}

	int read_now_size = 0;
	
	char* buf;

	next_block_idx = get_first_empty();
	
	while (writable_size > 0)
	{	
		update_first_empty_idx();

		if (cur_block_idx >= CFS_NUMBER_OF_BLOCKS - cfs_sb.meta_end_idx)
		{
			printf("error writng, cant fit\n");
			system("pause");
			return -1;
		}


		read_now_size = (writable_size >= CRS_DATA_IN_BLOCK_SIZE) ? CRS_DATA_IN_BLOCK_SIZE : writable_size;
		printf("read_now_size %d\n", read_now_size);

		buf = calloc(read_now_size, sizeof(char));
		if (buf == NULL)
		{
			exit(-1); // err
		}
		

		fread(buf, sizeof(char), read_now_size, src_fd);
		
		writable_size -= read_now_size;
		wrote_size += read_now_size;

		write_block((writable_size == 0 ? &writable_size : &next_block_idx), buf, read_now_size, cur_block_idx);
		
		last_block_idx = cur_block_idx;
		cur_block_idx = get_first_empty();
		
		if (cur_block_idx == -1 && scr_size != wrote_size)
		{
			printf("no space left\n");
			int zero = 0;
			write_block_next_idx(&zero, last_block_idx);
			update_first_empty_idx();
			update_size_by_meta(&dst_file_meta, wrote_size);
			update_sb();

			fclose(src_fd);
			return -1;
		}		
		next_block_idx = get_next_empty();
	}


	if (scr_size < dst_file_meta.content_size) // files becomes smaller
	{
		delete_block_list(cur_block_idx);			
	}
	
	update_size_by_meta(&dst_file_meta, scr_size);
	update_sb();

	fclose(src_fd);
	return 0;
}

int delete_file(char* _dst_at_cfs)
{
	cfs_meta dst_file_meta;
	if (find_meta_by_fname(&dst_file_meta, _dst_at_cfs) == -1) // no such destination
	{
		return -1; // err
	}
	if (dst_file_meta.content_size > 0)
	delete_block_list(dst_file_meta.start_block_idx);
	return 0;
}

// EOF