#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pcb.h"
#include "IOSCHEDULER.h"
#include "DISK_driver.h"


typedef struct node {
	int id; // not necessarity 
	char* command;
	char* data;
	struct node * next;
} Node;

Node * CURRENTNODE;


char* getBufferResult()
{  	// only called by the interpreter, when a read was successful
	return returnBlock();
}

int hasHardDiskMounted()
{
	return hasHardDisk();
}

void IOMounter(char* partitionName, int number_of_blocks, int block_size)
{
	printf("calling io moutner!\n");
	if(hasHardDiskMounted() != -1) // has a hardisk currently mounted
	{
		saveToDiskAndDisconnect();
	}

	partition(partitionName, number_of_blocks, block_size); // if already exist wouldn't do anything
	mount(partitionName);
}

int IOWriteScheduler(char* data, char* filename, PCB *ptr)
{
	if(hasHardDiskMounted() == -1)
	{
		printf("No partition found!\n");
		return 1;
	}
	int fatIndex  = openfile(filename); 
	if (fatIndex == -1) // file name not found
	{
		fatIndex = createFile(filename);
		if (fatIndex == -1)
		{
			printf("Error! FAT is full to create file: %s", filename);
		}
		fatIndex  = openfile(filename); 
	}

	if (fatIndex == -2)
	{
		printf("Error! fp is full to open file: %s\n",filename );
		return 1;
	}

	printf("IOWriteScheduler found index to be %d\n",fatIndex);
	int writeResult = writeBlock(fatIndex, data); // its error message shall be boardcasted in DISK_driver, do not have time to copy them to here
	return 1;
}


int IOReadscheduler(char* data, PCB *ptr)
{ 	// RETURN:  0 if success 
	//			-1 failure due to files not found
	// 			-2 no disk mounted

	if(hasHardDiskMounted() == -1)
	{
		printf("No partition found!\n");
		return -1;
	}
	
	// data represent the file name
	int openResult = openfile(data);
	if (openResult < 0) // error, most likely file doesn't exist
	{
		printf("File not found!\n");
		return -1;
	}
	else
	{
		int result= readBlock(openResult);
		return result;
	}	
}

void IOSave()
{
	saveToDiskAndDisconnect();
}

int IOScheduler(char* data, PCB *ptr, int cmd)
{
	int i;
	if(CURRENTNODE == NULL)
	{
		CURRENTNODE = malloc(sizeof(Node));

	}
	if (cmd == 0)
	{
		IOReadscheduler(data,NULL);
	}
	else
	{
		IOWriteScheduler(data,"",NULL);
	}
}