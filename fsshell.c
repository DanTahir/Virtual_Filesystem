/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
* Group Name: Segfault
* Project: Basic File System
*
* File: fsShell.c
*
* Description: Main driver for file system assignment.
*
* Make sure to set the #defined on the CMDxxxx_ON from 0 to 1 
* when you are ready to test that feature
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <getopt.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"

#define PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

#define SINGLE_QUOTE	0x27
#define DOUBLE_QUOTE	0x22
#define BUFFERLEN		200
#define DIRMAX_LEN		4096

/****   SET THESE TO 1 WHEN READY TO TEST THAT COMMAND ****/
#define CMDLS_ON	1
#define CMDCP_ON	1
#define CMDMV_ON	1
#define CMDMD_ON	1
#define CMDRM_ON	1
#define CMDCP2L_ON	1
#define CMDCP2FS_ON	1
#define CMDCD_ON	1
#define CMDPWD_ON	1
#define CMDTOUCH_ON	1
#define CMDCAT_ON	1


typedef struct dispatch_t
	{
	char * command;
	int (*func)(int, char**);
	char * description;
	} dispatch_t, * dispatch_p;


int cmd_ls (int argcnt, char *argvec[]);
int cmd_cp (int argcnt, char *argvec[]);
int cmd_mv (int argcnt, char *argvec[]);
int cmd_md (int argcnt, char *argvec[]);
int cmd_rm (int argcnt, char *argvec[]);
int cmd_touch (int argcnt, char *argvec[]);
int cmd_cat (int argcnt, char *argvec[]);
int cmd_cp2l (int argcnt, char *argvec[]);
int cmd_cp2fs (int argcnt, char *argvec[]);
int cmd_cd (int argcnt, char *argvec[]);
int cmd_pwd (int argcnt, char *argvec[]);
int cmd_history (int argcnt, char *argvec[]);
int cmd_help (int argcnt, char *argvec[]);

dispatch_t dispatchTable[] = {
	{"ls", cmd_ls, "Lists the file in a directory"},
	{"cp", cmd_cp, "Copies a file - source [dest]"},
	{"mv", cmd_mv, "Moves a file - source dest"},
	{"md", cmd_md, "Make a new directory"},
	{"rm", cmd_rm, "Removes a file or directory"},
        {"touch",cmd_touch, "Touches/Creates a file"},
        {"cat", cmd_cat, "Limited version of cat that displace the file to the console"},
	{"cp2l", cmd_cp2l, "Copies a file from the test file system to the linux file system"},
	{"cp2fs", cmd_cp2fs, "Copies a file from the Linux file system to the test file system"},
	{"cd", cmd_cd, "Changes directory"},
	{"pwd", cmd_pwd, "Prints the working directory"},
	{"history", cmd_history, "Prints out the history"},
	{"help", cmd_help, "Prints out help"}
};

static int dispatchcount = sizeof (dispatchTable) / sizeof (dispatch_t);

// Display files for use by ls command
int displayFiles (fdDir * dirp, int flall, int fllong)
	{
#if (CMDLS_ON == 1)				
	if (dirp == NULL)	//get out if error
		return (-1);
	
	struct fs_diriteminfo * di;
	struct fs_stat statbuf;
	
	di = fs_readdir (dirp);
	printf("\n");
	while (di != NULL) 
		{
		if ((di->d_name[0] != '.') || (flall)) //if not all and starts with '.' it is hidden
			{
			if (fllong)
				{
				// there's no way to make this work for paths without creating a dir
				// here and switching the working directory back and forth because fs_stat
				// reads a path from the working directory not the directory passed in to
				// fs_readdir, so we have to set the working directory to the directory
				// pointed to by fs_readdir, then set it back once we're done reading
				DirEntry * dir = dirInstance();
				dirCopyWorking(&dir);
				dirSetWorking(dirp->directoryStartLocation);
				fs_stat (di->d_name, &statbuf);
				printf ("%s    %9ld   %s\n", fs_isDir(di->d_name)?"D":"-", statbuf.st_size, di->d_name);
				dirSetWorking(dir[0].location);
				free(dir);
				dir=NULL;
				}
			else
				{
				printf ("%s\n", di->d_name);
				}
			}
		di = fs_readdir (dirp);
		}
	fs_closedir (dirp);
#endif
	return 0;
	}
	

/****************************************************
*  ls commmand
****************************************************/
int cmd_ls (int argcnt, char *argvec[])
	{
#if (CMDLS_ON == 1)				
	int option_index;
	int c;
	int fllong;
	int flall;
	char cwd[DIRMAX_LEN];
		
	static struct option long_options[] = 
		{
		/* These options set their assigned flags to value and return 0 */
		/* These options don't set flags and return the value */	 
		{"long",	no_argument, 0, 'l'},  
		{"all",		no_argument, 0, 'a'},
		{"help",	no_argument, 0, 'h'},
		{0,			0,       0,  0 }
		};
		
	option_index = 0;
#ifdef __GNU_LIBRARY__
    // WORKAROUND
    // Setting "optind" to 0 triggers initialization of getopt private
    // structure (holds pointers on data between calls). This helps
    // to avoid possible memory violation, because data passed to getopt_long()
    // could be freed between parse() calls.
    optind = 0;
#else
    // "optind" is used between getopt() calls to get next argument for parsing and should be
    // initialized before each parsing loop.
    optind = 1;
#endif
	fllong = 0;
	flall = 0;

	while (1)
		{	
		c = getopt_long(argcnt, argvec, "alh",
				long_options, &option_index);
				
		if (c == -1)
		   break;

		switch (c) {
			case 0:			//flag was set, ignore
			   printf("Unknown option %s", long_options[option_index].name);
			   if (optarg)
				   printf(" with arg %s", optarg);
			   printf("\n");
				break;
				
			case 'a':
				flall = 1;
				break;
				
			case 'l':
				fllong = 1;
				break;
				
			case 'h':
			default:
				printf ("Usage: ls [--all-a] [--long/-l] [pathname]\n");
				return (-1);
				break;
			}
		}
	
	
	if (optind < argcnt)
		{
		//processing arguments after options
		for (int k = optind; k < argcnt; k++)
			{
			if (fs_isDir(argvec[k]))
				{
				fdDir * dirp;
				dirp = fs_opendir (argvec[k]);
				displayFiles (dirp, flall, fllong);
				}
			else // it is just a file ?
				{
				if (fs_isFile (argvec[k]))
					{
					//no support for long format here
					printf ("%s\n", argvec[k]);
					}
				else
					{
					printf ("%s is not found\n", argvec[k]);
					}
				}
			}		
		}
	else   // no pathname/filename specified - use cwd
		{
		char * path = fs_getcwd(cwd, DIRMAX_LEN);	//get current working directory
		fdDir * dirp;
		dirp = fs_opendir (path);
		return (displayFiles (dirp, flall, fllong));
		}
#endif
	return 0;
	}


	
/****************************************************
*  Touch file commmand
****************************************************/

int cmd_touch (int argcnt, char *argvec[])
        {
#if (CMDTOUCH_ON == 1)     
        int testfs_src_fd;
        char * src;

        switch (argcnt)
                {
                case 2: //only one name provided
                        src = argvec[1];
                        break;

                default:
                        printf("Usage: touch srcfile\n");
                        return (-1);
                }

        testfs_src_fd = b_open (src, O_WRONLY | O_CREAT);
        if (testfs_src_fd < 0)
	    return (testfs_src_fd);	//return with error

        b_close (testfs_src_fd);
#endif
        return 0;
        }


/***************************************************
* Cat Command
***************************************************/

int cmd_cat (int argcnt, char *argvec[])
        {
#if (CMDCAT_ON == 1)     
        int testfs_src_fd;
        char * src;
        int readcnt;
        char buf[BUFFERLEN];

        switch (argcnt)
                {
                case 2: //only one name provided
                        src = argvec[1];
                        break;

                default:
                        printf("Usage: cat srcfile\n");
                        return (-1);
                }


        testfs_src_fd = b_open (src, O_RDONLY);

        if (testfs_src_fd < 0)
            {
	    printf ("Failed to open file system file: %s\n", src);
            return (testfs_src_fd);
            }


        do 
                {
                readcnt = b_read (testfs_src_fd, buf, BUFFERLEN);
                buf[readcnt] = '\0';
                printf("%s", buf);
                } while (readcnt == BUFFERLEN);
        b_close (testfs_src_fd);
#endif
        return 0;
        }




/****************************************************
*  Copy file commmand
****************************************************/
	
int cmd_cp (int argcnt, char *argvec[])
	{
#if (CMDCP_ON == 1)	
	int testfs_src_fd;
	int testfs_dest_fd;
	char * src;
	char * dest;
	int readcnt;
	char buf[BUFFERLEN];
	
	switch (argcnt)
		{
		case 2:	//only one name provided
			src = argvec[1];
			dest = src;
			break;
			
		case 3:
			src = argvec[1];
			dest = argvec[2];
			break;
		
		default:
			printf("Usage: cp srcfile [destfile]\n");
			return (-1);
		}
	
	
	testfs_src_fd = b_open (src, O_RDONLY);
	testfs_dest_fd = b_open (dest, O_WRONLY | O_CREAT | O_TRUNC);
	do 
		{
		readcnt = b_read (testfs_src_fd, buf, BUFFERLEN);
		b_write (testfs_dest_fd, buf, readcnt);
		} while (readcnt == BUFFERLEN);
	b_close (testfs_src_fd);
	b_close (testfs_dest_fd);
#endif
	return 0;
	}
	
/****************************************************
*  Move file commmand
****************************************************/
int cmd_mv (int argcnt, char *argvec[])
	{
#if (CMDMV_ON == 1)				
	// return -99;
	// **** TODO ****  For you to implement	
	int testfs_src_fd;
	int testfs_dest_fd;
	char * src;
	char * dest;
	int readcnt;
	char buf[BUFFERLEN];
	
	switch (argcnt)
		{
		case 2:	//only one name provided
			src = argvec[1];
			dest = src;
			break;
			
		case 3:
			src = argvec[1];
			dest = argvec[2];
			break;
		
		default:
			printf("Usage: cp srcfile [destfile]\n");
			return (-1);
		}
	
	
	testfs_src_fd = b_open (src, O_RDONLY);
	testfs_dest_fd = b_open (dest, O_WRONLY | O_CREAT | O_TRUNC);
	do 
		{
		readcnt = b_read (testfs_src_fd, buf, BUFFERLEN);
		b_write (testfs_dest_fd, buf, readcnt);
		} while (readcnt == BUFFERLEN);
	b_close (testfs_src_fd);
	b_close (testfs_dest_fd);
	if(testfs_src_fd != -1 && testfs_dest_fd != -1 && strcmp(src, dest) != 0){
		fs_delete(src);
	}
	


#endif
	return 0;
	}

/****************************************************
*  Make Directory commmand
****************************************************/
// Make Directory	
int cmd_md (int argcnt, char *argvec[])
	{
#if (CMDMD_ON == 1)				
	if (argcnt != 2)
		{
		printf("Usage: md pathname\n");
		return -1;
		}
	else
		{
		return(fs_mkdir(argvec[1], 0777));
		}
#endif
	return -1;
	}
	
/****************************************************
*  Remove directory or file commmand
****************************************************/
int cmd_rm (int argcnt, char *argvec[])
	{
#if (CMDRM_ON == 1)
	if (argcnt != 2)
		{
		printf ("Usage: rm path\n");
		return -1;
		}
		
	char * path = argvec[1];	
	
	//must determine if file or directory
	if (fs_isDir (path))
		{
		return (fs_rmdir (path));
		}		
	if (fs_isFile (path))
		{
		return (fs_delete(path));
		}	
		
	printf("The path %s is neither a file not a directory\n", path);
#endif
	return -1;
	}
	
/****************************************************
*  Copy file from test file system to Linux commmand
****************************************************/
int cmd_cp2l (int argcnt, char *argvec[])
	{
#if (CMDCP2L_ON == 1)				
	int testfs_fd;
	int linux_fd;
	char * src;
	char * dest;
	int readcnt;
	char buf[BUFFERLEN];
	
	switch (argcnt)
		{
		case 2:	//only one name provided
			src = argvec[1];
			dest = src;
			break;
			
		case 3:
			src = argvec[1];
			dest = argvec[2];
			break;
		
		default:
			printf("Usage: cp2l srcfile [Linuxdestfile]\n");
			return (-1);
		}
	
	
	testfs_fd = b_open (src, O_RDONLY);
	linux_fd = open (dest, O_WRONLY | O_CREAT | O_TRUNC, PERMISSIONS);
	do 
		{
		readcnt = b_read (testfs_fd, buf, BUFFERLEN);
		write (linux_fd, buf, readcnt);
		} while (readcnt == BUFFERLEN);
	b_close (testfs_fd);
	close (linux_fd);
#endif
	return 0;
	}
	
/****************************************************
*  Copy file from Linux to test file system commmand
****************************************************/
int cmd_cp2fs (int argcnt, char *argvec[])
	{
#if (CMDCP2FS_ON == 1)				
	int testfs_fd;
	int linux_fd;
	char * src;
	char * dest;
	int readcnt;
	char buf[BUFFERLEN];
	
	switch (argcnt)
		{
		case 2:	//only one name provided
			src = argvec[1];
			dest = src;
			break;
			
		case 3:
			src = argvec[1];
			dest = argvec[2];
			break;
		
		default:
			printf("Usage: cp2fs Linuxsrcfile [destfile]\n");
			return (-1);
		}
	
	
	testfs_fd = b_open (dest, O_WRONLY | O_CREAT | O_TRUNC);
	linux_fd = open (src, O_RDONLY);
	do 
		{
		readcnt = read (linux_fd, buf, BUFFERLEN);
		b_write (testfs_fd, buf, readcnt);
		} while (readcnt == BUFFERLEN);
	b_close (testfs_fd);
	close (linux_fd);
#endif
	return 0;
	}
	
/****************************************************
*  cd commmand
****************************************************/
int cmd_cd (int argcnt, char *argvec[])
	{
#if (CMDCD_ON == 1)	
	if (argcnt != 2)
		{
		printf ("Usage: cd path\n");
		return (-1);
		}
	char * path = argvec[1];	//argument
	
	if (path[0] == '"')
		{
		if (path[strlen(path)-1] == '"')
			{
			//remove quotes from string
			path = path + 1;
			path[strlen(path) - 1] = 0;
			}
		}
	int ret = fs_setcwd (path);
	if (ret != 0)	//error
		{
		printf ("Could not change path to %s\n", path);
		return (ret);
		}			
#endif
	return 0;
	}
	
/****************************************************
*  PWD commmand
****************************************************/
int cmd_pwd (int argcnt, char *argvec[])
	{
#if (CMDPWD_ON == 1)
	char * dir_buf = malloc (DIRMAX_LEN +1);
	char * ptr;	
	ptr = fs_getcwd (dir_buf, DIRMAX_LEN);	
	if (ptr == NULL)			//an error occurred
		{
		printf ("An error occurred while trying to get the current working directory\n");
		}
	else
		{
		printf ("%s\n", ptr);
		}
	free (dir_buf);
	dir_buf = NULL;
	ptr = NULL;
	
#endif
	return 0;
	}

/****************************************************
*  History commmand
****************************************************/
int cmd_history (int argcnt, char *argvec[])
	{
	HIST_ENTRY * he;
	int i = 0;
	
	
	for (i = history_base; i <= history_length; i++)
		{
		he = history_get(i);
		
		if (he != NULL)
			{
			printf ("%s\n", he->line);
			}
		}
	return 0;
	}
	
/****************************************************
*  Help commmand
****************************************************/
int cmd_help (int argcnt, char *argvec[])
	{
	for (int i = 0; i < dispatchcount; i++)
		{
		printf ("%s\t%s\n", dispatchTable[i].command, dispatchTable[i].description);
		}
	return 0;
	} 

void processcommand (char * cmd)
	{
	int cmdLen;
	char ** cmdv;	//command vector
	int cmdc;		//command count
	int i, j;
	
	cmdLen = strlen(cmd);
	cmdv = (char **) malloc (sizeof(char *) * ((cmdLen/2)+2));
	cmdc = 0;
	
	cmdv[cmdc] = cmd;
	++cmdc;
	
	for (i = 0; i < cmdLen; i++)
		{
		switch (cmd[i])
			{
			case ' ':
				cmd[i] = 0;	//NULL terminate prior string
				while (cmd[i+1] == ' ')  // null at end will prevent from overshooting string
					{
					i++;
					}
				if ((i+1) < cmdLen)	//there is still more
					{
					cmdv[cmdc] = &cmd[i+1];
					++cmdc;
					}
				break;
				
			case '\\':
				++i;	//skip next character
				break;
				
			case DOUBLE_QUOTE:	//ignore everything till next quote (unless unterminated)
				for (j = i+1; j < cmdLen; j++)
					{
					if (cmd[j] == '\\')
						{
						++j;	//skip next character
						}	
					else if (cmd[j] == DOUBLE_QUOTE)
						{
						break;
						}
					}
				if (j >= cmdLen)
					{
					printf("Unterminated string\n");
					free(cmdv);
					cmdv = NULL;
					return;
					}
				i=j;
				break;
				
			case SINGLE_QUOTE:
				for (j = i+1; j < cmdLen; j++)
					{
					if (cmd[j] == '\\')
						{
						++j;	//skip next character
						}	
					else if (cmd[j] == SINGLE_QUOTE)
						{
						break;
						}
					}
				if (j >= cmdLen)
					{
					printf("Unterminated string\n");
					free(cmdv);
					cmdv = NULL;
					return;
					}
				i=j;
				break;
			default:
				break;
			}
		}
		
#ifdef COMMAND_DEBUG
	for (i = 0; i < cmdc; i++)
		{
		printf("%s: length %d\n", cmdv[i], strlen(cmdv[i]));
		}
#endif		
	cmdv[cmdc] = 0;		//just be safe - null terminate array of arguments
	
	for (i = 0; i < dispatchcount; i++)
		{
		if (strcmp(dispatchTable[i].command, cmdv[0]) == 0)
			{
			dispatchTable[i].func(cmdc,cmdv);
			free (cmdv);
			cmdv = NULL;
			return;
			}
		}
	printf("%s is not a regonized command.\n", cmdv[0]);
	cmd_help(cmdc, cmdv);	
	free (cmdv);
	cmdv = NULL;
	}



int main (int argc, char * argv[])
	{
	char * cmdin;
	char * cmd;
	HIST_ENTRY *he;
	char * filename;
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    
	if (argc > 3)
		{
		filename = argv[1];
		volumeSize = atoll (argv[2]);
		blockSize = atoll (argv[3]);
		}
	else
		{
		printf ("Usage: fsLowDriver volumeFileName volumeSize blockSize\n");
		return -1;
		}
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);

	if (retVal != PART_NOERROR)
		{
		printf ("Start Partition Failed:  %d\n", retVal);
		return (retVal);
		}
		
	retVal = initFileSystem (volumeSize / blockSize, blockSize);
	
	if (retVal != 0)
		{
		printf ("Initialize File System Failed:  %d\n", retVal);
		closePartitionSystem();
		return (retVal);
		}

	if (argc > 4)
		if(strcmp("lowtest", argv[4]) == 0)
			runFSLowTest();


	using_history();
	stifle_history(200);	//max history entries
	
	while (1)
		{
		cmdin = readline("Prompt > ");
#ifdef COMMAND_DEBUG
		printf ("%s\n", cmdin);
#endif
		
		cmd = malloc (strlen(cmdin) + 30);
		strcpy (cmd, cmdin);
		free (cmdin);
		cmdin = NULL;
		
		if (strcmp (cmd, "exit") == 0)
			{
			free (cmd);
			cmd = NULL;
			exitFileSystem();
			closePartitionSystem();
			// exit while loop and terminate shell
			break;
			}
			
		if ((cmd != NULL) && (strlen(cmd) > 0))
			{
			he = history_get(history_length);
			if (!((he != NULL) && (strcmp(he->line, cmd)==0)))
				{
				add_history(cmd);
				}
			processcommand (cmd);
			}
				
		free (cmd);
		cmd = NULL;		
		} // end while
	}
