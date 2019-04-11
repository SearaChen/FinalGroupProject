#ifndef _IOSCHEDULER_H_
#define _IOSCHEDULER_H_

#include "pcb.h"

char* getBufferResult();
void IOMounter(char* partitionName, int number_of_blocks, int block_size);
int IOWriteScheduler(char* data, char* filename, PCB *ptr);
int IOReadscheduler(char* data, PCB *ptr);
int IOScheduler(char* data, PCB *ptr, int cmd);
#endif