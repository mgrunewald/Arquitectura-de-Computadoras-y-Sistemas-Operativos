
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') { 
        //fprintf(stderr, "Error: Only absolute paths are supported. Path given: %s\n", pathname);
        return -1; }  // Only absolute paths are handled
    int curr = ROOT_INUMBER; // we start at the root inode (1)
    struct direntv6 dir_entry;
    char path_copy[strlen(pathname) + 1];
    strcpy(path_copy, pathname);
    char *token = strtok(path_copy, "/");
    while (token != NULL) {
        int result = directory_findname(fs, token, curr, &dir_entry); // look up the component name in the current directory
        if (result < 0) { return -1; } // component not found
        curr = dir_entry.d_inumber; // update the current inode number to the found entry's inode number
        token = strtok(NULL, "/"); // move to the next component in the path
    }
    return curr;
}
