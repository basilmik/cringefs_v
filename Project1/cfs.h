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
#define CFS_NUMBER_OF_BLOCKS 10


int cfs_init(char* _path);
int cfs_shutdown();
int cfs_format(char* _path);

typedef struct cfs_super_block_t {
    
    int magic;
    int sb_end_idx;
    int meta_end_idx;
    int num_of_meta_rec;
    int first_empty_block;

} cfs_super_block, * cfs_super_block_ptr;

typedef struct cfs_block_t {
   
    int next_idx; // if clean = -1
    unsigned char content[CRS_DATA_IN_BLOCK_SIZE];

} cfs_block, * cfs_block_ptr;

typedef struct cfs_meta_t { // 28

    int start_block_idx;
    unsigned char f_path[CFS_FILE_PATH_LEN];
    
    
    int content_size; // in bytes
    int meta_idx;
    int is_deleted;

} cfs_meta, * cfs_meta_ptr;


int create_file(char* _name);
int export_file(char* _dst, char* _src_at_cfs);
int delete_file(char* _dst_at_cfs, int _set_deleted);
void cfs_pack();