/*This file contains the FileInfo struct and the corresponding function prototypes.
 *Author: Robert Schaffer
 */

#ifndef FILEDATA_H
#define FILEDATA_H

#include <sys/stat.h>
#include "dllist.h"

typedef struct FileInfo{
	char * fileName;
	int nameSize;
	ino_t inode;
	mode_t mode;
	long int modTime;
	long int fileSize;
	char *fileBytes;
	int isLink;
}FileInfo;

extern FileInfo* createFileInfo(struct stat, char*, char*);
extern char* removePrefix(char*, char*);
extern FileInfo* findSameInode(Dllist, ino_t);
extern void freeFileInfo(FileInfo **);

#endif
