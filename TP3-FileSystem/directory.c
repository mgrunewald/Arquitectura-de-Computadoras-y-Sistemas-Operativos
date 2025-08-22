#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SECTOR_DIR_ENT_COUNT (DISKIMG_SECTOR_SIZE / sizeof(struct direntv6))


int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
  struct inode dir_inode;
  if (inode_iget(fs, dirinumber, &dir_inode) < 0) { return -1; }  // failed to load the inode
  if ((dir_inode.i_mode & IFDIR) == 0) {
    //fprintf(stderr, "Error: Inode %d is not a directory.\n", dirinumber);
    return -1; } // check if the inode is a directory
  int dir_size = inode_getsize(&dir_inode);
  int blocks_count = (dir_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

  struct direntv6 dir_entries[SECTOR_DIR_ENT_COUNT];
  for (int block = 0; block < blocks_count; block++) {
    int data_block = inode_indexlookup(fs, &dir_inode, block);
    if (data_block < 0) { return -1; } //error finding the block

    int bytes_read = diskimg_readsector(fs->dfd, data_block, dir_entries); //read the block
    if (bytes_read < 0) { return -1; } //error reading the block

    int entries_count = bytes_read / sizeof(struct direntv6);

    for (int i = 0; i < entries_count; i++) {
      if (strncmp(name, dir_entries[i].d_name, sizeof(dir_entries[i].d_name)) == 0) {
        *dirEnt = dir_entries[i]; // copies the found directory entry into dirEnt
        return 0;  // success!!
      }
    }
  } 
  return -1;
}
