#include <stdlib.h>
#include <stdio.h>
#include <utime.h>
#include <unistd.h>
#include "fileData.h"
#include "dllist.h"
#include "jrb.h"
#include "jval.h"

static void getFileInfo(Dllist);
static void recreateFiles(Dllist);
static void setMode(Dllist);
static void setModTime(Dllist);
static void memCleanUp(Dllist);

int main(int argc, char **argv)
{
	Dllist fileinfo;
	
	if(argc > 1){ fprintf(stderr, "usage: %s\n", argv[0]); exit(1);}

	fileinfo = new_dllist();
	
	getFileInfo(fileinfo);

	recreateFiles(fileinfo);
	setModTime(fileinfo);
	setMode(fileinfo);

	memCleanUp(fileinfo);
	return 0;
}

/*getFileInfo: reads the data and checks the data size read from stdin and puts the data into a list of FileInfo structs
 *Param: list, the list to hold the data
 *Post-Con: the list will contain all the needed data to recreate the files and directories
 */
static void
getFileInfo(Dllist list)
{
	FileInfo *f;
	Jval vPtr;
	JRB inodes;
	int start, end, test;

	inodes = make_jrb();
	
	start = ftell(stdin);
	f = (FileInfo*) malloc(sizeof(FileInfo));
	fread(&(f->nameSize), 4, 1, stdin);
	end = ftell(stdin);
	test = end - start;
	if(test != 4){
		fprintf(stderr, "Bad tarc file at byte %d. Tried to read filename size but only got %d bytes.\n", start, test);
		free(f);
		exit(1);
	}

	while(!feof(stdin)){
		f->isLink = 0;
		f->fileName = (char*) malloc(sizeof(char)*(f->nameSize+1));
		
		start = ftell(stdin);
		fread(f->fileName, f->nameSize, 1, stdin);
		end = ftell(stdin);
		test = end - start;
		if(f->nameSize != test){ 
			fprintf(stderr, "Bad tarc file at byte %d. File name size is %d, but bytes read = %d\n", start, f->nameSize, test); 
			exit(1);
		}
		f->fileName[f->nameSize] = '\0';
		
		start = ftell(stdin);
		fread(&(f->inode), 8, 1, stdin);
		end = ftell(stdin);
		test = end - start;
		if(test != 8){ 
			fprintf(stderr, "Bad tarc file at byte %d. Tried to read inode size but only got %d bytes.\n", start, test);
			exit(1);
		}

		if(jrb_find_int(inodes, f->inode) == NULL){
			jrb_insert_int(inodes, f->inode, JNULL);
			start = ftell(stdin);
			fread(&(f->mode), 4, 1, stdin);
			end = ftell(stdin);
			test = end - start;
			if(test != 4){ 
				fprintf(stderr, "Bad tarc file at byte %d. Tried to read mode size but only got %d bytes.\n", start, test);
				exit(1);
			}
			
			start = ftell(stdin);
			fread(&(f->modTime), 8, 1, stdin);
			end = ftell(stdin);
			test = end - start;
			if(test != 8){ 
				fprintf(stderr, "Bad tarc file at byte %d. Tried to read mod time size but only got %d bytes.\n", start, test);
				exit(1);
			}
			
			if(!S_ISDIR(f->mode)){
				start = ftell(stdin);
				fread(&(f->fileSize), 8, 1, stdin);
				end = ftell(stdin);
				test = end - start;
			if(test != 8){ 
				fprintf(stderr, "Bad tarc file at byte %d. Tried to read file size but only got %d bytes.\n", start, test);
				exit(1);
			}
				
				f->fileBytes = (char*) malloc(sizeof(char)*f->fileSize);
				
				start = ftell(stdin);
				fread(f->fileBytes, f->fileSize, 1, stdin);
				end = ftell(stdin);
				test = end - start;
				if(f->fileSize != test){ 
					fprintf(stderr, "Bad tarc file at byte %d. File name size is %d, but bytes read = %d\n", start, f->nameSize, test); 
					exit(1);
				}
			}
		}
		else f->isLink = 1;
		vPtr.v = (void*) f;
		dll_append(list, vPtr);

		start = ftell(stdin);
		f = (FileInfo*) malloc(sizeof(FileInfo));
		fread(&(f->nameSize), 4, 1, stdin);
		end = ftell(stdin);
		test = end - start;
		if(test != 4 && test != 0){
			fprintf(stderr, "Bad tarc file at byte %d. Tried to read filename size but only got %d bytes.\n", start, test);
			exit(1);
		}
	}
	free(f);
	jrb_free_tree(inodes);
}

/*recreateFiles: recreates the files and directories
 *Param: list, a list of FileIfo structs that has the needed data
 *Post-Con: the files and directories will exist
 */
static void
recreateFiles(Dllist list)
{
	Dllist tmp;
	FileInfo *f, *tmpf;
	FILE* fout;

	dll_traverse(tmp, list){
		f = (FileInfo*) tmp->val.v;
		if(S_ISDIR(f->mode)){ 
			mkdir(f->fileName, 1777);
		}
		else if(f->isLink){
			tmpf = findSameInode(list, f->inode);
			link(tmpf->fileName, f->fileName);
			f->mode = tmpf->mode;
			f->modTime = tmpf->modTime;
		}
		else{
			fout = fopen(f->fileName, "wb");
			fwrite(f->fileBytes, f->fileSize, 1, fout);
			fclose(fout);
		}
	}
}

/*setMode: sets the mode for all the recreated files and directories
 *Param:list, the list of FileInfo structs that contains the filenames
 *Post-Con: the mode will be set properly for all the files
 */
static void
setMode(Dllist list)
{
	Dllist tmp;
	FileInfo *f;

	dll_traverse(tmp, list){
		f = (FileInfo*) tmp->val.v;
		chmod(f->fileName, f->mode);
	}
}

/*setModTime: sets the last modification time for the recreated files and directories
 *Param: list, the list of FileInfo structs that contains the filenames
 *Post-Con: the modification time will be set properly for all the files
 */
static void
setModTime(Dllist list)
{
	Dllist tmp;
	FileInfo *f;
	struct utimbuf time;

	dll_traverse(tmp, list){
		f = (FileInfo*) tmp->val.v;
		time.modtime = f->modTime;
		utime(f->fileName, &time);
	}
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
