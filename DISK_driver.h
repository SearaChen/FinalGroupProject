#ifndef _DISKDRIVER_H_
#define _DISKDRIVER_H_


int isFileAlreadyOpen(char* filename);
void initIO();
int hasHardDisk();
int partition(char *name, int blocksize, int totalblocks); //loadFAT&createbuffer_block,returnsuccess/failure
int mount(char *name);
int createFile(char *name);
int openfile(char *name);
int readBlock(int file);
char *returnBlock();
int writeBlock(int file, char *data);
int saveToDiskAndDisconnect();

#endif