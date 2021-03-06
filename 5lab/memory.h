// James Jessen
// 10918967   
// 
// CptS 360

// ------------------------------------
// Functions for allocating and freeing
// memory primarily an ext2 filesystem
// ------------------------------------

#ifndef __MEMORY_H_
#define __MEMORY_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ext2fs/ext2_fs.h>

#include "util.h"
#include "transfer.h"

// Input an array of pointers like char* argv[]
// Goes through and frees each element
// and then frees the array
void free_array(char** array);

void set_free_inodes(int device, int change);
void set_free_blocks(int device, int change);
int ialloc(int device);
int balloc(int device);
int ifree (int device, int inode); 
int bfree (int device, int block); 


#endif
