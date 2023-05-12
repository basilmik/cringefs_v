#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"

#define CFS_STARTPOS 0xffff // for formating system
#define CFS_ENDPOS 0x5ffff
#define CFS_FILE_PATH_LEN 256
#define CFS_MAGIC 0x69A4

FILE* cfs_container;



int cfs_init(char* _path);

int cfs_shutdown();

int cfs_format(char* _path);

typedef struct cfs_super_block_t {
    int magic;
    int sb_end_idx;
    int meta_end_idx;
} cfs_super_block, * cfs_super_block_ptr;

typedef struct cfs_meta_t {

    //char f_path[CFS_FILE_PATH_LEN];
    //int start_block_idx;
    
    //int size; // in bytes
    //char is_dir;
    char cleared;

} cfs_meta, * cfs_meta_ptr;

cfs_super_block cfs_sb;