/****************************************************************/
// Created by Pranav Katkamwar, Andrew Khaznakovic, Karl Xu
// Spring 2017 - CS416
/****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _STDEFS_H
#define _STDEFS_H

#define	TOTAL_INODES	240000
#define BLOCK_SIZE 512

#define TOTAL_FS_SIZE ((8 + 16) * 1024 * 1024)
/****************************************************************/
// Structure Definitions
/****************************************************************/

//https://web.cs.wpi.edu/~rek/DCS/D04/UnixFileSystems.html
// Where im getting most of information

/* Inode Basics
 * An Inode number points to an Inode. An Inode is a data structure that stores the following information about a file :
 * Size of file
 * Device ID
 * User ID of the file
 * Group ID of the file
 * The file mode information and access privileges for owner, group and others
 * File protection flags
 * The timestamps for file creation, modification etc
 * link counter to determine the number of hard links
 * Pointers to the blocks storing file’s contents
 */


/* According to lecutre notes 
 * I-node contains: owner, type (directory, file, device), last modified
 * time, last accessed time, last I-node modified time, access
 * permissions, number of links to the file, size, and block pointers 
 */
typedef struct inode{
	short iType;			// dir or file //AK: Why short, if either dir or file we can just use char short is 2 bytes, we dont need 2^16 optiomns
	int ino;				// inode number
	long int size;			// data block size (bytes) | 0 = free | Around 4 GB
	int fileMode;			// Permissions
	time_t atime;			// inode access time
	time_t ctime;			// inode change time
	time_t mtime;			// inode modification time
	uid_t userID;			// user id
	gid_t groupID;			// group id
	// number of links (alias)
	// direct pointers to the datablocks for the file
	// does not contain file path
} inode;

inode * inodeTable;			// Global inode table

// Superblock to describe filesystem
// Each UNIX partition usually contains a special block called the superblock. 
// The superblock contains the basic information about the entire file system. 
// This includes the size of the file system, the list of free and allocated 
// blocks, the name of the partition, and the modification time of the filesystem.


/* Struct
 * s_list: superblock LL
 * s_dev: = Device indentifer
 * s_blocksize = block size in bytes
 * s_dir = dirty (modified) flag
 * s_op Superblock methods
 * s_lock = superblock semaphore
 * s_inodes = list of all inodes
 * s_io = inodes waiting for write
 * s_files = list of file objects
 */

// From class notes slide 24
/* Super Block contains
 * size of file system, number of free blocks, list of free blocks, 
 * index to the next free block, size of the I-node list,
 * number of free I-nodes, list of free I-nodes, index to the next free
 * I-node, locks for free block and free I-node lists, and flag to indicate
 * a modification to the SB
 */

typedef struct sblock {
	int num_inodes;
	int fs_size;
	int block_size;
	int num_free_blocks;
	int index_next_free_block;
	int num_free_inodes;
	int index_next_free_inode;
	int free_inode_list;
	int mod_flag;
	//list of free and allocated blocks
    // name of parition
	time_t atime; // last modified time
	// magic number?
	int root_inode_num; // Inode number of root directory
	// pointer to list of free blocks
} sblock;

// Not sure if be necessary but bitmap to keep track of blocks as in slide 13 of filesystem 
// slides

/* Bitmap: one bit for each block on the disk
 * Good to find a contiguous group of free blocks
 * Files are often accessed sequentially
 * For 1TB disk and 4KB blocks, 32MB for the bitmap
 * Chained free portions: pointer to the next one
 * Not so good for sequential access (hard to find sequential
 * blocks of appropriate size)
 * Index: treats free space as a file
 */


#endif


/*
Diskfile created with command:
dd if=/dev/zero of=diskfile bs=1024 count=24576
*/
