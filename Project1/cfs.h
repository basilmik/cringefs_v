#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include "stdlib.h"
#define CFS_STARTPOS 0xffff // for formating system
#define CFS_ENDPOS 0x5ffff
#define CFS_FILE_PATH_LEN 256
#define CFS_MAGIC 0x69A4
#define CFS_BLOCK_SIZE 64
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
} cfs_super_block, * cfs_super_block_ptr;

typedef struct cfs_block_t {
    char content[CFS_BLOCK_SIZE];
} cfs_block, * cfs_block_ptr;

typedef struct cfs_meta_t { // now takes 8 bytes

    //char f_path[CFS_FILE_PATH_LEN];

    int start_block_idx; //  4 bytes
    int size; // 4 bytes

    //char is_dir;
    //char cleared;

} cfs_meta, * cfs_meta_ptr;

int add_new_meta();