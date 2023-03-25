# CSC415 Group Term Assignment - File System

This is a GROUP assignment written in C.  Only one person on the team needs to submit the project.

Your team have been designing components of a file system.  You have defined the goals and designed the directory entry structure, the volume structure and the free space.  Now it is time to implement your file system.

While each of you can have your own github, only one is what you use for the project to be turned in.  Make sure to list that one on the writeups.

The project is in three phases.  The first phase is the "formatting" the volume.  This is further described in the steps for phase one and the phase one assignment.

The second phase is the implementation of the directory based functions.  See Phase two assignment.

The final phase is the implementation of the file operations.

To help I have written the low level LBA based read and write.  The routines are in fsLow.o, the necessary header for you to include file is fsLow.h.  You do NOT need to understand the code in fsLow, but you do need to understand the header file and the functions.  There are 2 key functions:



`uint64_t LBAwrite (void * buffer, uint64_t lbaCount, uint64_t lbaPosition);`

`uint64_t LBAread (void * buffer, uint64_t lbaCount, uint64_t lbaPosition);`

LBAread and LBAwrite take a buffer, a count of LBA blocks and the starting LBA block number (0 based).  The buffer must be large enough for the number of blocks * the block size.

On return, these function returns the number of **blocks** read or written.



In addition, I have written a hexdump utility that will allow you to analyze your volume file in the Hexdump subdirectory.

**Your assignment is to write a file system!** 

You will need to format your volume, create and maintain a free space management system, initialize a root directory and maintain directory information, create, read, write, and delete files, and display info.  See below for specifics.

I have provided an initial “main” (fsShell.c) that will be the driver to test you file system.  Your group can modifiy this driver as needed.   The driver will be interactive (with all built in commands) to list directories, create directories, add and remove files, copy files, move files, and two “special commands” one to copy from the normal filesystem to your filesystem and the other from your filesystem to the normal filesystem.

You should modify this driver as needed for your filesystem, adding the display/setting of any additional meta data, and other functions you want to add.

The shell also calls two function in the file fsInit.c `initFileSystem` and `exitFileSystem` which are routines for you to fill in with whatever initialization and exit code you need for your file system.  

Some specifics - you need to provide the following interfaces:

```
b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

```

Note that the function are similar to the b_read you have done, there is a signifigant difference since you now write the function to find the file information.  
You have to have methods of locating files, and knowing which logical block addresses are associated with the file.

Directory Functions - see [https://www.thegeekstuff.com/2012/06/c-directory/](https://www.thegeekstuff.com/2012/06/c-directory/) for reference.

```
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);
fdDir * fs_opendir(const char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdir
int fs_isFile(char * path);	//return 1 if file, 0 otherwise
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file

// This is NOT the directory entry, it is JUST for readdir.
struct fs_diriteminfo
    {
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
    };
```
Finally file stats - not all the fields in the structure are needed for this assingment

```
int fs_stat(const char *path, struct fs_stat *buf);

struct fs_stat
    {
    off_t     st_size;    		/* total size, in bytes */
    blksize_t st_blksize; 		/* blocksize for file system I/O */
    blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
    time_t    st_accesstime;   	/* time of last access */
    time_t    st_modtime;   	/* time of last modification */
    time_t    st_createtime;   	/* time of last status change */
	
    * add additional attributes here for your file system */
    };

```

These interfaces will also be provided to you in mfs.h.

**Note:** You will need to modify mfs.h for the fdDIR strucutre to be what your file system need to maintain and track interation through the directory structure.

A shell program designed to demonstrate your file system called fsshell.c is proviced.  It has a number of built in functions that will work if you implement the above interfaces, these are:
```
ls - Lists the file in a directory
cp - Copies a file - source [dest]
mv - Moves a file - source dest
md - Make a new directory
rm - Removes a file or directory
touch - creates a file
cat - (limited functionality) displays the contents of a file
cp2l - Copies a file from the test file system to the linux file system
cp2fs - Copies a file from the Linux file system to the test file system
cd - Changes directory
pwd - Prints the working directory
history - Prints out the history
help - Prints out help
```


This is deliberately vague, as it is dependent on your filesystem design.  And this all you may get initially for a real-world assignment, so if you have questions, please ask.

We will discuss some of this in class.

For our purposes use 10,000,000 or less (minimum 500,000) bytes for the volume size and 512 bytes per sector.  These are the values to pass into startPartitionSystem.

What needs to be submitted (via GitHub and iLearn):

* 	All source files (.c and .h)
* 	Modified Driver program (must be a program that just utilizes the header file for your file system).
* 	The Driver program must be named:  `fsshell.c`
* 	A make file (named “Makefile”) to build your entire program
 
* A PDF writeup on project that should include (this is also submitted in iLearn):
	* The github link for your group submission.
	* A description of your file system
	* Issues you had
	* Detail of how your driver program works
	* Screen shots showing each of the commands listed above
* 	Your volume file (limit 10MB)
*  There will also be an INDIVIDUAL report (form) to complete.




