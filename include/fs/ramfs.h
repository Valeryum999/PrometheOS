#ifndef _FS_RAMFS_H
#define _FS_RAMFS_H

#include <stdint.h>
#include <stddef.h>

enum inode_type {
    INODE_FILE,
    INODE_DIR,
};

typedef struct {
    int id;                   // Unique ID
    enum inode_type type;     // File or directory
    char *name;               // Name (basename)
    inode *parent;     // Parent directory
    inode **children;  // For directories: list of child inodes
    int child_count;

    char *data;               // For files: pointer to file data
    int size;              // File size
    int mode;                 // Permissions (e.g., 0755)
} inode;

inode *root_inode;
inode *ramfs_mkdir(inode *parent, const char *name, int mode);
inode *ramfs_create_file(inode *parent, const char *name, int mode, char *data, int size);
inode *ramfs_lookup(inode *dir, const char *name);


#endif