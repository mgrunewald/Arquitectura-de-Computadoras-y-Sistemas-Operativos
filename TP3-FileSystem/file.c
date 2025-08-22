#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"


int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode file_inode;
    if (inode_iget(fs, inumber, &file_inode) < 0) { return -1; } // load and check loading of the inode
    int file_size = inode_getsize(&file_inode);
    int max_blocks = (file_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    if (blockNum >= max_blocks) { 
        //fprintf(stderr, "Error: Block number %d exceeds file size for inode %d.\n", blockNum, inumber);
        return -1; 
        }
    int data_block = inode_indexlookup(fs, &file_inode, blockNum);
    if (data_block < 0) { return -1; } // error in finding the data block
    int bytes_read = diskimg_readsector(fs->dfd, data_block, buf);
    if (bytes_read < 0) { return -1; }  // error reading the block
    int remaining_bytes = file_size - (blockNum * DISKIMG_SECTOR_SIZE);
    int valid_bytes = (remaining_bytes < DISKIMG_SECTOR_SIZE) ? remaining_bytes : DISKIMG_SECTOR_SIZE; //valid bytes in the block
    return valid_bytes;
}
