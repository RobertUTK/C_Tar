/*This file takes the name of a directory from the second argument and creates a "tar" of that directory.
 *Author: Robert Schafer
 */
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"
#include "jval.h"
#include "fileData.h"

static void getFileList(Dllist, char *, char *);
static void createTar(Dllist);
static char* getPrefix(char*);
static void memCleanUp(Dllist);

int 
main(int argc, char **argv)
{
	Dllist data;
	char *prefix;
	
	if(argc != 2){
		fprintf(stderr, "usage: %s directory\n", argv[0]); 
		exit(1);
	}

	prefix = getPrefix(argv[1]);

	data = new_dllist();

	getFileList(data, argv[1], prefix);

	free(prefix);
	createTar(data);

	memCleanUp(data);

	return 0;
}

/*getFileList: Fills a list with FileInfo structs that contain the data needed for the tar.
 *Params: list, the list to fill
 *		  dname, the pathname for the file or directory
 *		  prefix, the prefix to remove from the path name
 *Post-Con: List will have all the FileInfo structs to create the tar
 */
static void
getFileList(Dllist list, char *dName, char *prefix)
{
	DIR *d;
	struct dirent *de;
	struct stat buf;
	char *s;
	int exists;
	Dllist directories, tmp;
	FileInfo *f;
	Jval vPtr;

	s = (char*) malloc(sizeof(char)*(strlen(dName)+258));
	
	exists = lstat(dName, &buf);
	f = createFileInfo(buf, dName, prefix);
	vPtr.v = (void*) f;
	dll_append(list, vPtr);

	d = opendir(dName);
	if(d == NULL) { perror(dName), exit(1);}

	directories = new_dllist();	

	for(de = readdir(d); de != NULL; de = readdir(d)){
		sprintf(s, "%s/%s", dName, de->d_name);
		exists = lstat(s, &buf);
		if(exists < 0) fprintf(stderr, "Couldn't stat %s\n", s);
		else if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
			f = createFileInfo(buf, s, prefix);
			vPtr.v = (void*) f;
			dll_append(list, vPtr);
		}
		if(S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
			dll_append(directories, new_jval_s(strdup(s)));
		}
	}
	closedir(d);
	dll_traverse(tmp, directories){
		getFileList(list, tmp->val.s, prefix);
		free(tmp->val.s);
	}
	free_dllist(directories);
	free(s);
}

/*createTar: Uses the list to write the binary for the tar file to stdout
 *Param: list, the list of the needed FileInfo structs
 */
static void 
createTar(Dllist list)
{
	Dllist tmp;
	JRB inodes;
	FileInfo *f;

	inodes = make_jrb();

dll_traverse(tmp, list){
		f = (FileInfo*) tmp->val.v;
		if((jrb_find_int(inodes, f->inode) == NULL && S_ISDIR(f->mode)) || !S_ISDIR(f->mode)){
			fwrite(&(f->nameSize), 4, 1, stdout);
			fwrite(f->fileName, f->nameSize, 1, stdout);
			fwrite(&(f->inode), 8, 1, stdout);
			if(jrb_find_int(inodes, f->inode) == NULL){
				jrb_insert_int(inodes, f->inode, JNULL);
				fwrite(&(f->mode), 4, 1, stdout);
				fwrite(&(f->modTime), 8, 1, stdout);
				if(!S_ISDIR(f->mode)){
					fwrite(&(f->fileSize), 8, 1, stdout);
					fwrite(f->fileBytes, f->fileSize, 1, stdout);
				}
			}
		}
	}
	jrb_free_tree(inodes);
}

/*getPrefix: finds the last / in a string and allocates a new string with everything up to it
 *Param: s, the string with the data
 *Returns: prefix, the data to the last /
 */
static char*
getPrefix(char* s)
{
	int i, prefixEnd;
	char *prefix;

	prefixEnd = 0;
	
	for(i = 0; i < strlen(s); i++)
		if(s[i] == '/') prefixEnd = i;
	
	prefix = (char*) malloc(sizeof(char)*(prefixEnd+1));

	for(i = 0; i < prefixEnd; i++)
		prefix[i] = s[i];
	prefix[i] = '\0';

	return prefix;
}

/*memCleanUp: free the memory allocated for the list and the FileInfo structs it contains.
 *Param: list, the list of FileInfo structs
 *Post-Con: All the memory for the list and data inside will be freed.
 */
static void
memCleanUp(Dllist list)
{
	Dllist tmp;
	FileInfo *f;

	dll_traverse(tmp, list){
		f = (FileInfo*) tmp->val.v;
		freeFileInfo(&f);
	}
	free_dllist(list);
}
