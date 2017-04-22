/*
  Simple File System

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.

*/

#include "params.h"
#include "block.h"
#include "../stdefs.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"



/* Helper file for creating bitmap */
//http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html

//Need to add to make file
// Combined set and clear. If bit given in K, cleark that bit. Else set that bit
void setAndClearBit(int * bitmap, int bitK){
  
  int i = abs(bitK/32);
  int pos = abs(bitK%32);
  unsigned int flag = 1;

  if(bitK < 0){
    flag = ~flag;
    bitmap[i] = bitmap[i] & flag;
  }
  else{
    i *= -1;
    bitmap[i] | flag;
  }
  
}

int testBit(int * bitmap, int bitK){
    int i = bitK/32;
      int pos = bitK%32;

      unsigned int flag = 1;  

      flag = flag << pos;     

      if ( bitmap[i] & flag ){
        return 1;
      }     
         
      else{
        return 0;
      }

}


///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *sfs_init(struct fuse_conn_info *conn)
{
    fprintf(stderr, "in bb-init\n");
    log_msg("\nsfs_ini1t()\n");
    
    log_conn(conn);
    log_fuse_context(fuse_get_context());


//    return SFS_DATA;
//    SFS_DATA defined in /src/params.h
//    #define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data) is in params.h so its set on mount
//	  sfs_state compriesed of file pointer to logfile and the string with disk file name
//    init should open the disk with void disk_open(const char* diskfile_path) from block.c
//    not need for initTracker as it should be a disk mounted basis not FUSE instance basis.
//
//    an integer called diskfile in block.c gets set on the result of a system open call in disk_open
//    so if it sucessfully opens then disk_open will return since disk already opened

//	Rough Draft - How do we write this to the disk?
//	ToDo:
//		- Assign shit to /root
//		- 
	

		log_msg("\ndisk_open(path=%s) TOTAL_FS_SIZE: %i NUMI: %i\n", SFS_DATA->diskfile, TOTAL_FS_SIZE, TOTAL_INODES);
		// Opens disk simply returns if disk already open
		disk_open(SFS_DATA->diskfile);

    //log_msg("\nGod dammit\n", SFS_DATA->diskfile);

		
		char * super = (char *) malloc(BLOCK_SIZE);

     struct stat *statbuf = (struct stat*) malloc(sizeof(struct stat));
    int in = lstat((SFS_DATA)->diskfile,statbuf);
    log_msg("\nVIRTUAL DISK FILE STAT: %s\n", (SFS_DATA)->diskfile);
    log_stat(statbuf);

// Check if there is a SUPERBLOCK set using block_read
// Read first block (if set first block is super block
// Read should return (1) exactly @BLOCK_SIZE when succeeded, or (2) 0 when the requested block has never been touched before, or (3) a negtive value when failed. 
		int readResult = block_read(1, &super); 
    printf("Makes it here\n");

		if(readResult == 0){ // block has never been touched
			// Create and initialize SBlock
      int * blockFreeList = (int *)malloc(TOTAL_BLOCKS/sizeof(int));
      sblock * superBlock = (sblock *)malloc(sizeof(sblock));
			superBlock->fs_size = TOTAL_FS_SIZE;
      log_msg("TOTAL_FS_SIZE: %i\n", superBlock->fs_size);
			superBlock->block_size = BLOCK_SIZE;
			superBlock->num_inodes = TOTAL_INODES;
      superBlock->num_blocks = TOTAL_BLOCKS;
			superBlock->num_free_blocks =  TOTAL_BLOCKS - NUM_RESERVED_BLOCKS;
			superBlock->index_next_free_block = TOTAL_BLOCKS - superBlock->num_free_blocks - 1; // first free block right after superblock
      superBlock->free_block_list = NULL; // Not sure if should keep this 
			superBlock->num_free_inodes = TOTAL_INODES - 1; // ALl inodes free except first one (which will be set to root)
			superBlock->index_next_free_inode = 1; // First free inode should be root
			superBlock->free_inode_list = NULL; // Not sure if will keeep 
			superBlock->root_inode_num = 0; // Inode number for root directory made up for now		
			
      // Write the SBlock to to first block in FS using block_write
			// Write should return exactly @BLOCK_SIZE except on error. 

      int writeResult = block_write(1, superBlock);
     
      if(writeResult < 0){ //write failed
        log_msg("\nblock_write(0, &superBlock) failed\n");
        exit(0);
      }
      else{
        log_msg("\nblock_write(0, &superBlock) was successful");
        log_msg("\nsize of superBlock = %i\n", sizeof(sblock));
      }

      free(superBlock);
			
      int * inodeBitmap = (int *) malloc(TOTAL_INODES/sizeof(int));
      setAndClearBit(inodeBitmap, 0); // inode 0 = root so set it as used;
  
      
      if(writeResult < 0){ //write failed
        log_msg("\nblock_write(0, &inodeBitmapk) failed\n");
        exit(0);
      }
      else{
        log_msg("\nblock_write(0, &inodeBitmap) was successful");
        //log_msg("\nsize of inodeBitmap = %i\n", sizeof(sblock));
      }

      writeResult = block_write(2, inodeBitmap);
      free(inodeBitmap);

      int * blockBitmap = (int *) malloc(TOTAL_BLOCKS/sizeof(int));
      
      int i= 0;

      for(; i < superBlock->index_next_free_block; i++){
           setAndClearBit(blockBitmap, i);
      }

      writeResult = block_write(3, blockBitmap);

      if(writeResult < 0){ //write failed
        log_msg("\nblock_write(0, &blockBitmap) failed\n");
        exit(0);
      }
      else{
        log_msg("\nblock_write(0, &blockBitmap) was successful");
        //log_msg("\nsize of inodeBitmap = %i\n", sizeof(sblock));
      }
    
      free(blockBitmap);

      inode * inodeTable = malloc(sizeof(inode) * TOTAL_INODES);
      
      i = 0;

      for(; i< TOTAL_INODES; i++){
        inodeTable[i].iNum = i;            
      }

      inodeTable[0].iType = 'd';
      inodeTable[0].size = 0; 
      inodeTable[0].atime = time(&(inodeTable[0].atime));
      inodeTable[0].ctime = time(&(inodeTable[0].ctime));
      inodeTable[0].mtime = (&(inodeTable[0].mtime));
      inodeTable[0].groupID = getegid();
      inodeTable[0].userID = getuid();

      i=4;
      char * buffer = malloc(BLOCK_SIZE);
     
 
      writeResult = block_write(4, inodeTable);

      if(writeResult < 0){ //write failed
        log_msg("\nblock_write(0, &inodeTable) failed\n");
        exit(0);
      }
      else{
        log_msg("\nblock_write(0, &inoodeTable) was successful");
        //log_msg("\nsize of inodeBitmap = %i\n", sizeof(sblock));
      }

      
      
      

		}
		else if(readResult > 0){ // first block has been accessed before parse superBlock information
			char* superBlock = (char *)malloc(BLOCK_SIZE);
			sblock * test = (sblock *)superBlock;
      int superBlockRead = block_read(1, superBlock);

      log_msg("superBlockRead1: %i Block SIze: %i\n", superBlockRead, BLOCK_SIZE);
			if(superBlockRead > 0){ // superBlock read sucessfully
				log_msg("FS Contains Valid Super Block\n");
       log_msg("SuperBlock: TOTAL_FS_SIZE: %i\n NumI: %i \n", test->fs_size, test->num_inodes);

				log_msg("\nsize of superBlock = %i\n", sizeof(sblock));
			}
			else {
				log_msg("block read failed\n");
			}
			
		}

    else{
        log_msg("block read failed\n");
    }


 log_msg("Rile System Successfully Initialized SFS_STATE ---> DISKFILE: %s\n", SFS_DATA->diskfile);

   in = lstat((SFS_DATA)->diskfile,statbuf);
    log_msg("\nVIRTUAL DISK FILE STAT222: %s\n", (SFS_DATA)->diskfile);
    log_stat(statbuf);


  //log_conn(conn);
  //log_fuse_context(fuse_get_context());
  disk_close(SFS_DATA->diskfile);

return SFS_DATA;
	
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int sfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    
    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
    path, statbuf);
    
    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
      path, mode, fi);
    
    
    return retstat;
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);

    
    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
      path, fi);

    
    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int sfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_release(path=\"%s\", fi=0x%08x)\n",
    path, fi);
    

    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
      path, buf, size, offset, fi);

   
    return retstat;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
       struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
      path, buf, size, offset, fi);
    
    
    return retstat;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
      path, mode);
   
    
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
      path);
    
    
    return retstat;
}


/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int sfs_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_opendir(path=\"%s\", fi=0x%08x)\n",
    path, fi);
    
    
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
         struct fuse_file_info *fi)
{
    int retstat = 0;
    
    
    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    
    return retstat;
}

struct fuse_operations sfs_oper = {
  .init = sfs_init,
  .destroy = sfs_destroy,

  .getattr = sfs_getattr,
  .create = sfs_create,
  .unlink = sfs_unlink,
  .open = sfs_open,
  .release = sfs_release,
  .read = sfs_read,
  .write = sfs_write,

  .rmdir = sfs_rmdir,
  .mkdir = sfs_mkdir,

  .opendir = sfs_opendir,
  .readdir = sfs_readdir,
  .releasedir = sfs_releasedir
};

void sfs_usage()
{
    fprintf(stderr, "usage:  sfs [FUSE and mount options] diskFile mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct sfs_state *sfs_data;
    
    // sanity checking on the command line
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
  sfs_usage();

    sfs_data = malloc(sizeof(struct sfs_state));
    if (sfs_data == NULL) {
  perror("main calloc");
  abort();
    }

    // Pull the diskfile and save it in internal data
    sfs_data->diskfile = argv[argc-2];
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    sfs_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main, %s \n", sfs_data->diskfile);
    fuse_stat = fuse_main(argc, argv, &sfs_oper, sfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
