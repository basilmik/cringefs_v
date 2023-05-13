#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define CFS_STARTPOS 0xffff // for formating system
#define CFS_ENDPOS 0x5ffff
#define CFS_FILE_PATH_LEN 16 // 256
#define CFS_MAGIC 0x69A4
#define CFS_BLOCK_SIZE 64
#define CRS_DATA_IN_BLOCK_SIZE CFS_BLOCK_SIZE - 4
#define CFS_NUMBER_OF_BLOCKS 4



int cfs_init(char* _path);

int cfs_shutdown();

int cfs_format(char* _path);

typedef struct cfs_super_block_t {
    int magic;
    int sb_end_idx;
    int empty_start_idx;
    int meta_end_idx;
    int num_of_meta_rec;
    int first_empty_block;
} cfs_super_block, * cfs_super_block_ptr;

typedef struct cfs_block_t {
    int next_idx;
    char content[CFS_BLOCK_SIZE - 4];

} cfs_block, * cfs_block_ptr;

typedef struct cfs_meta_t { // now takes 8 bytes

    char f_path[CFS_FILE_PATH_LEN];

    int start_block_idx; //  
    int content_size; // in bytes
    int meta_idx;
    //char is_dir;
    //char cleared;

} cfs_meta, * cfs_meta_ptr;

int add_new_meta();


int copy_file_to_cfs(char* _src, char* _dst_at_cfs);

int create_file(char* _name);