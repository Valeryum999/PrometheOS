#include <fs/ramfs.h>

inode *ramfs_mkdir(inode *parent, const char *name, int mode){
    inode *directory = malloc(sizeof(inode));
    directory->name = name;
    directory->mode = mode;
    parent->children[parent->child_count] = directory;
}
inode *ramfs_create_file(inode *parent, const char *name, int mode, char *data, int size){
    inode *file = malloc(sizeof(inode));;
    file->name = name;
    file->data = data;
    file->size = size;
    parent->children[parent->child_count] = file;
    parent->child_count++;
}
inode *ramfs_lookup(inode *dir, const char *name){
    for(int i=0; i<dir->child_count; i++){
        if(dir->children[i]->name == name){
            return dir->children[i];
        }
    }

    return NULL;
}