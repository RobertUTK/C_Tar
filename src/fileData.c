/*This file contains the function definitions for the FileInfo struct.
 *Author: Robert Schaffer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fileData.h"
#include "dllist.h"

/*createFileInfo: Allocates and fills a FileInfo struct with the needed data and returns its pointer.
 *Params: buf, a stat stucture with the needed data
 *		  pname, the path name for the file
 *		  prefix, the prefix to be removed from the path name
 *Returns: f, the FileInfo pointer with the data
 */
extern FileInfo*
createFileInfo(struct stat buf, char *pname, char *prefix)
{
    FileInfo *f;
	char *fname;
    FILE *fin;
    
	f = (FileInfo*) malloc(sizeof(FileInfo));

	fname = removePrefix(pname, prefix);
	
	f->fileName = (char*) malloc(sizeof(char)*(strlen(fname)+1));
    memcpy(f->fileName, fname, (strlen(fname)+1));
    f->nameSize = strlen(fname);
    f->inode = buf.st_ino;
    f->mode = buf.st_mode;
    f->fileSize = buf.st_size;
    f->modTime = buf.st_mtime;
    if(!S_ISDIR(buf.st_mode)){
        fin = fopen(pname, "rb");
        f->fileBytes = (char*) malloc(sizeof(char)*f->fileSize);
        fread(f->fileBytes, f->fileSize, 1, fin);
        fclose(fin);
    }
    return f;
}

/*removePrefix: Allocates a new string without the prefix of the path name
 *Params: name, the path name
 *		  prefix, the prefix to remove
 *Returns: noPrefix, a new string that has the name w/o the prefix
 */
extern char*
removePrefix(char* name, char* prefix)
{
	char *noPrefix;
	int length, i, count = 0;

	length = strlen(name) - strlen(prefix) + 1;
	noPrefix = (char*) malloc(sizeof(char)*length);
	
	for(i = strlen(prefix)+1; i <= strlen(name); i++){
		noPrefix[count] = name[i];
		count++;
	}
	return noPrefix;
}

/*findSameInode: Searches a list of FileInfo data and returns the first FileInfo with the same inode
 *Params: list, the list of FileInfos
 *		  i, the inode to look for
 *Returns f, a pointer to the first FileInfo with the same inode, otherwise NULL
 */
extern FileInfo*
findSameInode(Dllist list, ino_t i)
{	
	Dllist tmp;
	FileInfo *f;
	
	dll_traverse(tmp, list){
		f = (FileInfo*) tmp->val.v;
		if(i == f->inode) return f;
	}
	return NULL;
}

/*freeFileInfo: frees the allocated memory for a FileInfo struct
 *Params: f, a ptr ptr to a FileInfo
 *Post-Con: The memory allocated for f will be freed.
 */
extern void
freeFileInfo(FileInfo **f)
{
	free((*f)->fileName);
	if(!S_ISDIR((*f)->mode)) free((*f)->fileBytes);
	free(*f);
}
