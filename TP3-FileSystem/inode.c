#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

#define SECTOR_INODE_COUNT (DISKIMG_SECTOR_SIZE / sizeof(struct inode)) // size of a disk sector (blocks) / inode size
#define SECTOR_ADD_COUNT (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) //size of a disk sector (blocks) / address size
// we know there are 8 address pointers -> i_addr[8] 
#define SINGLE_INDIRECT_SECTORS 7 //indexed to 0 [0,6], 7 
#define INDIRECT_ADD_COUNT SECTOR_ADD_COUNT * (SINGLE_INDIRECT_SECTORS)

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber <= 0 || !inp || !fs) { return -1; }

    int inode_sector = (inumber - 1) / SECTOR_INODE_COUNT; // target sector (int truncates it) 
    int position_on_the_sector = (inumber - 1) % SECTOR_INODE_COUNT; // position within the sector (indexado)
    int current_sector_global = INODE_START_SECTOR + inode_sector; // sector location on the disk
    struct inode buffer[SECTOR_INODE_COUNT]; // buffer for the read inodes from the sector
    int read_inode = diskimg_readsector(fs->dfd, current_sector_global, buffer); // read the sector
    if (read_inode == -1) { return -1; } //error handling (diskimg_readsector returns -1 when it fails)

    *inp = buffer[position_on_the_sector]; // copies the inode into what inp points at
    if (!(inp->i_mode & IALLOC)) { 
        fprintf(stderr, "Error: Inode %d is not allocated.\n", inumber);
        return -1; }  // the inode isn't allocated, so we can't hash it
    //if ((inp.i_mode & IFMT) != 0 && (inp.i_mode & IFMT) != IFDIR) { return -1; } // only proceed if the inode is a regular file or directory

    return 0; // success!
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {  
    if (!fs || !inp || blockNum < 0) { return -1; } //error handling 

    if ((inp->i_mode & ILARG) == 0) { return inp->i_addr[blockNum]; }  // small archive just return the block address in i_addr
    if ((unsigned int)blockNum < INDIRECT_ADD_COUNT) { //indrirect sectors [0,6]
        int sector = blockNum / SECTOR_ADD_COUNT; 
        int position_within_the_sector = blockNum % SECTOR_ADD_COUNT; //me había olvidado del ;
        uint16_t buffer_add[SECTOR_ADD_COUNT];    // misma lógica que antes
        int read_address = diskimg_readsector(fs->dfd, inp->i_addr[sector], buffer_add);
        if (read_address == -1) { return -1; } //error handling (diskimg_readsector returns -1 when it fails)
        return buffer_add[position_within_the_sector];   // returns disk block number
    } 
    else { //double indirect sector i_addr[7] 
        int blockNum_restarted = blockNum - INDIRECT_ADD_COUNT; // we adjust it so we treat it as if it starts from 0
        int first_level_index = blockNum_restarted / SECTOR_ADD_COUNT;  // position within the first-level double-indirect block
        int second_level_index = blockNum_restarted % SECTOR_ADD_COUNT; // position within the second-level block
        uint16_t first_level_buffer[SECTOR_ADD_COUNT]; // buffer for addresses in the first-level double-indirect block
        uint16_t second_level_buffer[SECTOR_ADD_COUNT]; // buffer for addresses in the second-level indirect block
        int read_first_level = diskimg_readsector(fs->dfd, inp->i_addr[SINGLE_INDIRECT_SECTORS], first_level_buffer); 
        if (read_first_level == -1) { return -1; } 
        int read_second_level = diskimg_readsector(fs->dfd, first_level_buffer[first_level_index], second_level_buffer);
        if (read_second_level == -1) { return -1; } 
        return second_level_buffer[second_level_index];
    } 
    return -1;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
