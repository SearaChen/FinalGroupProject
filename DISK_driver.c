#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "DISK_driver.h"

// global data structures definition 
typedef struct PARTIT {
    int total_blocks;
    int block_size;
    int numOfFiles;
} Partition;


typedef struct FAT {
    char filename[100];
    int file_length;	// file length in number of blocks
    int blockPtrs[10]; 
    int current_location;
} FileIndexObject;


//global data structure instantiation 
FileIndexObject* fat[20];
char * block_buffer;
FILE *fp[5];
int fpInfo[5]; // stores the index of its corresding FileIndexObject location in fat
int originalNumberOFObjectOffset=0;
Partition* PARTITION;
char mountPartitionName[500];
int isHardDiskPresent=-1; // a binary bit indicating weather has a disk is mounted 

void encodeGlobalVariableToFile(FILE *partitionP)
{
	
	fwrite(PARTITION, sizeof(Partition), 1, partitionP); // segfault

	int i = 0;
	for (i=0; i < 20; i++)
	{	
		if(fat[i] == NULL)
		{continue;}
		fwrite(fat[i], sizeof(FileIndexObject), 1, partitionP);
	}

	printf("partition # of blocks: %d\n", PARTITION->total_blocks);
	printf("partition size of blocks: %d\n", PARTITION->block_size);
	printf("# OF files: %d\n", PARTITION->numOfFiles);
	printf("done encoding!!!!!!\n");
		
}


void printFatTable()
{	
	printf("=============== FAT table =================\n");
	int i=0;
	for (i =0; i < 20; i++)
	{
		if(fat[i] == NULL){continue;}
		int s;
		printf("------- file %s -------\n", fat[i]->filename);
		printf("current_location: %d\n", fat[i]->current_location);
		for(s=0;s<10;s++)
		{
			printf("%d,",fat[i]->blockPtrs[s]);
		}
		printf("\n");

	}
	printf("==================================\n");

}



void decodeGlobalVariableFromFile(FILE *infile)
{
	// read partition
	fread(PARTITION, sizeof(Partition),1,infile);

	printf("DONE DECODING!!!\n");
	printf("partition # of blocks: %d\n", PARTITION->total_blocks);
	printf("partition size of blocks: %d\n", PARTITION->block_size);
	printf("# OF files: %d\n", PARTITION->numOfFiles);

	// read fat
	FileIndexObject* input;
	input = malloc(sizeof(FileIndexObject));
	int i=0;

	while(fread(input, sizeof(FileIndexObject), 1, infile) && i<PARTITION->numOfFiles)
	{
		//input = fat[i];
		if (input == (FileIndexObject*)NULL)
		{
			continue;
		}
		fat[i] =input;
		fat[i]->current_location =0;
		input = malloc(sizeof(FileIndexObject));
		i++;

	}
}


void wipe_block_buffer()
{ // PURPOSE: wipe the buffer and allocate space
	int totalSize =PARTITION->total_blocks*PARTITION->block_size;
	int i;
	for(i=0;i< totalSize;i++)
	{
		block_buffer[i]='0';
	}
	// block_buffer = malloc(totalSize);
	// block_buffer=emptyContent;
	// strcpy(block_buffer, emptyContent);
}


FILE* giveBlockNumberContentPtr(FILE* f, int i)
{	
	FILE *fpDup = fdopen (dup (fileno (f)), "r+");
	int j ;
	int offsetFilesCount =0;
	fseek(fpDup, (sizeof(Partition)+(originalNumberOFObjectOffset)*sizeof(FileIndexObject)+ (i*PARTITION->block_size)), SEEK_SET);
	return fpDup;
}


void copyHardDriveToBuffer()
{
	wipe_block_buffer();
	int i=0;
	FILE *f = fopen(mountPartitionName,"r+");
	FILE* writePtr = giveBlockNumberContentPtr(f, 0);
	int c;
	while(i<PARTITION->block_size * PARTITION->total_blocks)
	{
		c = fgetc(writePtr);
		block_buffer[i] =c;
		i++;
	}
	fclose(f);
	fclose(writePtr);
}


void copyBufferToHardDrive()
{
	int i=0;
	FILE *f = fopen(mountPartitionName,"r+");
	FILE* writePtr = giveBlockNumberContentPtr(f, 0);
	int c;
	while(i<PARTITION->block_size * PARTITION->total_blocks)
	{
		c = fputc (block_buffer[i],writePtr);
		i++;
	}
	fclose(f);
	fclose(writePtr);
}


void printUserDataSection()
{
	FILE* f = fopen(mountPartitionName,"rb+");
	FILE* writePtr = giveBlockNumberContentPtr(f, 0);
	printf("User Data Section Content: \n");
	int c;
	int count=0;
	while((c=fgetc(writePtr)) != EOF)
	{
		printf("%c",c);
		count++;
		if(count % PARTITION->block_size == 0 )
		{
			printf(" ");
		}
	}
	printf("\n");
	fclose(writePtr);
	fclose(f);
}

void printBufferContent()
{
	int i=0;
	int c;
	printf("buffer: ");
	for(i=0; i < PARTITION->block_size * PARTITION->total_blocks; i++)
	{	
		c= block_buffer[i];
		printf("%c",c);
		if((i+1) % PARTITION->block_size == 0 )
		{
			printf(" ");
		}
	}
	printf("\n");
}

void printGlobalOpenFilePointer()
{
	int i;
	printf("GLOBAL pointers:");
	for(i=0;i<5;i++)
	{	
		if (fp[i] == NULL)
		{
			printf("null,");
			continue;
		}
		printf("%ld(%s),", ftell(fp[i]),fat[fpInfo[i]]->filename);
	}
	printf("\n");
}

printGlobalOpenFilePointerInfo()
{
	int i;
	printf("GLOBAL pointers:");
	for(i=0;i<5;i++)
	{	
		if (fp[i] == NULL)
		{
			printf("null,");
			continue;
		}
		printf("%d,", fpInfo[i]);
	}
	printf("\n");
}

void printFile(char*name)
{
	// get entire user data block
	printf("Printing content of filename: %s\n", name );
	wipe_block_buffer();
	FILE* mountedfp= fopen(mountPartitionName,"rb+");
	FILE* blockFilePtr = giveBlockNumberContentPtr(mountedfp, 0);
	int c;
	int count=0;
	fgets(block_buffer, PARTITION->total_blocks*PARTITION->block_size , blockFilePtr); 
    fclose(mountedfp);
    fclose(blockFilePtr);
	// looping through fat
	int i;
	for(i =0; i < 20 ; i++)
	{	
		if (fat[i]!=NULL)
		{		
			if(strcmp(name,fat[i]->filename) == 0)
			{
				int j;
				for(j=0;j<10;j++)
				{
					if(fat[i]->blockPtrs[j] ==-1){ return;}
					int s;
					for (s=0; s<PARTITION->block_size;s++)
					{
						printf("%c",block_buffer[fat[i]->blockPtrs[j]*PARTITION->block_size+s ]);
					}
					printf(" ");
				}
			}
		}
	}
	wipe_block_buffer();

	printf("No such file is found!\n");
}

int isFileAlreadyOpen(char* filename)
{// Purpose: check if a file with "filename" is already open through looping through fp's
 // Return : its fat index if it is already open
 //			 -1 if it is not already open
	// printf("check if file already open!\n");
	int i; 	
	// printf("fpInfo: ");
	// for (i =0; i < 5; i++){printf("%d", fpInfo[i]);} 	printf("\n");

	printGlobalOpenFilePointer();
	printGlobalOpenFilePointerInfo();

	for (i =0; i < 5; i++)
	{
		if(fp[i]!=NULL)
		{
			if (strcmp(fat[fpInfo[i]]->filename, filename) == 0)
			{
				printf("%s already opened! with name: %s\n",filename,fat[fpInfo[i]]->filename);
				printGlobalOpenFilePointer();
				printGlobalOpenFilePointerInfo();
				return fpInfo[i];
			}
		}
	}
	printf("%s not yet opened!\n",filename);
	printGlobalOpenFilePointer();
	printGlobalOpenFilePointerInfo();

	return -1;
}

// global data structures 
void initIO()
{
	// initialize FAT
	int i;
	for(i =0; i < 20 ; i++)
	{	
		fat[i] = (NULL);
	}

	// initialize fp
	for(i =0; i < 5 ; i++)
	{
		fp[i] = NULL;
	}

	// Initialize fpInfo
	for (i =0 ; i< 5; i++)
	{
		fpInfo[i] =-1; 
	}

	//initialize partition 
	isHardDiskPresent =-1;
	PARTITION = malloc(sizeof(Partition));
}


int hasHardDisk()
{
	return isHardDiskPresent;
}


int partition(char *name, int totalblocks,int blocksize)
{
	// PURPOSE: making partition directory if does not exist already 
	// RETURN : -1 if the file exist already
	//			0 success

	// check if partition exist
	struct stat st = {0};
	if (stat("./PARTITION/", &st) == -1) {
    	mkdir("./PARTITION/",  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	// create the file with file name
	char partitionPath[200];
	strcpy(partitionPath, "./PARTITION/");
	strcat( partitionPath, name);
	FILE *partitionP; 

	//check if already exist 

	//create partition
	PARTITION->total_blocks = totalblocks;
	PARTITION->block_size = blocksize;
	PARTITION->numOfFiles = 0;

	if (access(partitionPath, F_OK) == 0) // if file exist
	{
		return -1;
	}
	partitionP = fopen( partitionPath, "w");
	encodeGlobalVariableToFile(partitionP);

	int i;
	int c;
	for (i=0;i<blocksize*totalblocks; i++)  // writing list of 0 
	{	
		c='0';
		int result= putc(c, partitionP) ;
	}	

	fclose(partitionP);
	return 0;
}


int mount(char *name)
{
	// PURPOSE: decode the information of a partition, put them in global data structures 
	// Input: the partition's name
	char partitionPath[200];
	strcpy(partitionPath, "./PARTITION/");
	strcat( partitionPath, name);
	struct stat st = {0};

	FILE * f; 
	f= fopen(partitionPath,"r+");
	printf("partition path: %s,%d\n", partitionPath,sizeof(char));

	decodeGlobalVariableFromFile(f);

 	block_buffer = malloc(sizeof(char)*PARTITION->total_blocks*PARTITION->block_size);

 	strcpy(mountPartitionName, partitionPath);
 	isHardDiskPresent =0;

 	originalNumberOFObjectOffset = PARTITION->numOfFiles;

 	fclose(f);

	return 0;
}


int findAvailableSpotInfp()
{// PURPOSE: find an available spot (== NULL) in fpp
 // RETURN : -1 if not available spot found
			//  if found: the index 
	int fpIndex = 0;
	for(fpIndex=0; fpIndex<5;fpIndex++)
	{
		if (fp[fpIndex] == NULL)
		{
			break;
		}
	}

	if (fpIndex == 5)
	{
		return -1;
	}
	else
	{
		return fpIndex;
	}
}


int createFile(char *name)
{// PURPOSE: creating a file whose name is "name"
 // RETURN :  i if it is created successfully 
 // 		  -1 if failure when fat is full
 //			  -2 if file already existed
	int j;
	for (j =0; j< 20; j++)
	{
		if(fat[j] != NULL)
		{
			if(strcmp(fat[j]->filename,name) ==0)
			{
				return -2;
			}
		}
	}

	for (j =0; j< 20; j++) // finding an empty spot in FAT
	{	
		if(fat[j] == NULL)
		{
			//  create new FAT object 
			FileIndexObject* newFile;
			newFile = malloc(sizeof(FileIndexObject));
			strcpy(newFile->filename, name);
			int s;
			for (s=0;s<10;s++){newFile->blockPtrs[s] =-1;}
			newFile->current_location=0;
			newFile->file_length=0;

			// add to directory 
			fat[j] = newFile;
			PARTITION->numOfFiles ++;
			return j;
		}
	}
	return -1;
}


int openfile(char *name)
{	// PURPOSE: assuming that the file is not already open, open it from fat, and assign to global pointer
	// INPUT: the name of the file looking for
	// RETURN: -1 -- file with name not found 
	// 		  -2 -- do not have more spot in fp 
	//        -3 -- unexpected error (bug in code)
	//  fatIndex -- if success
	int i=-1; // FAT index 
	int isFound=-1;
	printf("Openning a file %s....\n",name);
	int tmp = isFileAlreadyOpen(name) ;
	if(tmp >= 0) // already opened, returned some pointer
	{
		return tmp;
	}

	for (i =0; i < 20 ; i++) // looping through fat 
	{
		if(fat[i]!= NULL && fat[i]!= (FileIndexObject*) NULL)
		{

			printf("Current filename: %s\n",fat[i]->filename);
			if(strcmp(fat[i]->filename, name)==0)
			{
				isFound = 0;
				break;
			}
		}
	}

	if (isFound == 0) //found in existing FAT 
	{
		int firstBlockPointer = fat[i] -> blockPtrs[0];
		
		// find an available spot in fp
		int fpIndex=findAvailableSpotInfp();

		if (fpIndex == -1){ // did not find available spot for fpIndex 
			return -2; //error -- do not have more spot in fp 
		}
		else
		{
			FILE * mountedfp;
			printf("first block pointer: %d\n", firstBlockPointer );
			mountedfp= fopen(mountPartitionName,"rb+");
			FILE* blockFilePtr = giveBlockNumberContentPtr(mountedfp, firstBlockPointer);
			
			// assignment to fp;
			fp[fpIndex] =  blockFilePtr; 
			fpInfo[fpIndex]= i;
			fclose(mountedfp);
			return i;
		}
	}

	if (isFound == -1) // didn't find empty slot in fat
	{
		return -1; // an error occured
	}

	return -3; // should not reach here
}


int readBlock(int fileFATIndex)
{

	//  PURPOSE: using the file FAT index number, load buffer with 1 block of data from blockID
	//	RETURN : 0 if success; 
	//		     -1 end of file error
	//			 -2 invalid index
	// 			 -3  unexpected error -- bug in the code
	// 	NOTE : If success -- global buffer_index_pointer +1
	//						fat entry current_location +1

	printf("Reading your block at index#%d ! \n",fat[fileFATIndex]->blockPtrs[fat[fileFATIndex]->current_location] );
	if(fileFATIndex< 0 || fileFATIndex > 19)
	{
		return -2;
	}
	printGlobalOpenFilePointer();
	printGlobalOpenFilePointerInfo();
	FileIndexObject *currentFile = fat[fileFATIndex];

	//find filepointer in fpinfo
	int isFound =-1;
	int i;
	FILE* currentfp;
	for (i=0; i< 5; i++)
	{
		if(fpInfo[i] == fileFATIndex)
		{
			isFound=0;
			currentfp = fp[i];
			break;
		}
	}

	if(currentfp == NULL || isFound == -1)
	{
		printf("DISK_driver.c : Error! Corresponding fp not found to fileFATIndex!\n");
		return -3;
		//exit(EXIT_SUCCESS);
	}

	if (currentFile->current_location > 9) // read through all 20 blockPtrs
	{
		printf("reached end of file for file, cannot read more: %s\n", currentFile->filename );
		return -1; // return error code
	}
	else 
	{
		int currentBlockNum= currentFile->blockPtrs[currentFile->current_location];
		if (currentBlockNum == -1) // has reached an invalid blockFilePtr
		{
			printf("reached end of file/some error blockFilePtR for file: %s\n", currentFile->filename );
			return -1; // return error code

		}

		// actually reading the block 
		int c;
		FILE* blockPtr = giveBlockNumberContentPtr(currentfp, currentBlockNum);
		wipe_block_buffer();
		i=0;
		while(i<PARTITION->block_size)
		{	
			c= fgetc(blockPtr);
			block_buffer[i]=c;
			i++;
		}
		currentFile->current_location ++;
		return 0;

	}
	return -3; // should never reach here
}


char *returnBlock()
{// PURPOSE : return block data as string from block_buffer
	char* resultString = malloc(PARTITION->total_blocks*PARTITION->block_size); 
	int i=0;
	while(i < PARTITION->total_blocks*PARTITION->block_size)
	{
		if(block_buffer[i] =='0'){
			break;
		}
		resultString[i]=block_buffer[i];
		i++;
	}
	resultString[i] = '\0';
	printf("RETURN BLOCK: result string: %s\n", resultString);
	return resultString; // return the most recently written character in block buffer
}


int writeBlock(int fileFATIndex, char *data)
{	// PURPOSE : write to a block, assuming that that it is already open
	// RETURN  : 0 success
	//			 -1 reached end of file cannot write more, or ran out of space
	//			 -2 invalid index
    //			 -3 unexpected error : file not found
    //			 -4 has reached end of file 
	printf("Calling writeBlock() -- DISK_driver.c\n");
	if (fileFATIndex< 0 || fileFATIndex > 19) { 
		printf("invalid index??\n" );
		return -2;
	}

	//find filepointer in fpinfo
	int isFound =-1;
	int i;
	FILE* currentfp;
	FileIndexObject* fileObject;
	for (i=0; i< 5; i++)
	{
		if(fpInfo[i] == fileFATIndex)
		{
			isFound=0;
			currentfp = fp[i];
			fileObject = fat[fpInfo[i]];
			break;
		}
	}

	if(isFound == -1){ 
		printf("writeBlock(): Error! did not found FAT entry!\n"); return -3;
	}else if (currentfp == NULL ||fileObject == NULL ) {
		printf("writeBlock(): Error! fpointer is null\n");
		return -3;
	}else{

		if (fileObject->current_location > 9) {
			printf("Has reached End of file!\n");
			return -1;
		}

		int dataIndex=0; int blockIndex=0;  int ch;
		copyHardDriveToBuffer();
		printBufferContent(); 
		while(dataIndex < strlen(data) && fileObject->current_location < 10 && blockIndex < PARTITION->total_blocks)
		{	
			printf("current position: %d -- pointer content: %d\n",fileObject->current_location,fileObject->blockPtrs[fileObject->current_location]);
			if (fileObject->blockPtrs[fileObject->current_location]!=-1) // overwriting own file
			{	
				int overwritingBlockNum = fileObject->blockPtrs[fileObject->current_location];
				printf("overwriting own block! at block# %d\n",overwritingBlockNum);
				int offset = PARTITION->block_size*overwritingBlockNum;
				int s; 
				for(s=0;s<PARTITION->block_size; s++)
				{
					block_buffer[offset+s] ='0';
				}

				int withinBlockCounter=0;
				while(withinBlockCounter < PARTITION->block_size && dataIndex < strlen(data) )
				{
					block_buffer[offset+withinBlockCounter] = data[dataIndex];
					withinBlockCounter++;
					dataIndex ++;
				}
				fileObject->current_location++;

			}
			else
			{
				printf("------- %s examining block# %d------\n", fileObject->filename,blockIndex);
				ch = block_buffer[PARTITION->block_size*blockIndex];
				printf("its first character : %c\n", ch);
				if (ch=='0') // the block is unused
				{
					int withinBlockCounter=0;
					while(withinBlockCounter < PARTITION->block_size && dataIndex < strlen(data) )
					{
						int offset = PARTITION->block_size*blockIndex + withinBlockCounter;
						block_buffer[offset] = data[dataIndex];
						withinBlockCounter++;
						dataIndex ++;
					}
					printf("written at block# %d\n", blockIndex);
					printBufferContent();

					fileObject->blockPtrs[fileObject->current_location] = blockIndex;
					fileObject->current_location++;
					fileObject->file_length++;
				}
				blockIndex++;
			}
		}

		copyBufferToHardDrive();

		if(dataIndex < strlen(data))
		{
			printf("Not enough space to write data!\n");
			return -1;
		}

		printFatTable();
		printf("finished writing !!!\n");
		printGlobalOpenFilePointer();
		printUserDataSection();

		//exit(EXIT_SUCCESS);
		return 0;
	}
	return -3;
}


int saveToDiskAndDisconnect()
{ //To be called before quit

	// rewind all open files pointer 
	printf("Saving it to disk....\n" );
	int i;
	int fatIndex=-1; 
	for(i =0; i < 5; i++)
	{
		if(fp[i]!= NULL)
		{
			fatIndex = fpInfo[i];
			fat[fatIndex]->current_location =0;
		}
	}

	printf("partition # of blocks: %d\n", PARTITION->total_blocks);
	printf("partition size of blocks: %d\n", PARTITION->block_size);
	printf("# OF files: %d\n", PARTITION->numOfFiles);
	// okay here!!!

	// copy the entire block of data to buffer 
	wipe_block_buffer();
	FILE* mountedfp= fopen(mountPartitionName,"r+");

	FILE* blockFilePtr = giveBlockNumberContentPtr(mountedfp, 0);

	int c;
	int count=0;

	for (i=0; i < PARTITION->total_blocks*PARTITION->block_size ; i++)
	{
		c=fgetc(blockFilePtr);
		block_buffer[i] = c;
		printf("%c",c);
	}
    fclose(blockFilePtr);
    fclose(mountedfp);


	mountedfp= fopen(mountPartitionName,"w");
	encodeGlobalVariableToFile( mountedfp);
   
   	// actual data
   	printf("Starting to write buffer in!\n");
   	printBufferContent();
    // write everything in 
	for (i=0;i< PARTITION->total_blocks*PARTITION->block_size; i++)
	{
		c= block_buffer[i];
		int result= putc(c,  mountedfp) ;
	}
	fclose(mountedfp);


	//close and wipe everything
	isHardDiskPresent=-1;
	free(block_buffer);
	for(i =0; i < 5; i++)
	{
		if(fp[i]!= NULL)
		{
			fclose(fp[i]); //wipe the slot
			fp[i] = NULL;
			fpInfo[i] = -1; // wipe the slot
		}
	}


	for(i =0; i < 20; i++)
	{
		if(fat[i]!= NULL)
		{
			free(fat[i]);
			fat[i]=NULL;
		}
	}

	printf("Partition is successfully saved!\n");
	return 1;

}

/*int main()
{
	// char partitionPath[200];
	// strcpy(partitionPath, "./PARTITION/");
	// strcat( partitionPath,"haha.txt");
	// FILE *partitionP; 
	// printf("%s\n", partitionPath );
	// partitionP = fopen( partitionPath, "w");



	// PARTITION = malloc(sizeof(Partition));
	// PARTITION->total_blocks = 10;
	// PARTITION->block_size = 5;
	// PARTITION->numOfFiles = 0;
	// printf("partition # of blocks: %d\n", PARTITION->total_blocks);
	// printf("partition # of blocks: %d\n", PARTITION->block_size);

	// encodeGlobalVariableToFile(partitionP);
	// fclose(partitionP);
	// free(PARTITION);

	// PARTITION = malloc(sizeof(Partition));
	// partitionP = fopen( partitionPath, "r+");

	// decodeGlobalVariableFromFile(partitionP);
	// printf("partition # of blocks: %d\n", PARTITION->total_blocks);
	// printf("partition # of blocks: %d\n", PARTITION->block_size);

	initIO();
	//partition("test3.txt", 5, 5);
	//FILE* p = fopen("./PARTITION/test3.txt","r+");
	
	int openResult; int createResult; int writeResult;
	// WRITING FILE TEST CASES
	int result=mount("test3.txt");
	openResult = createFile("BTS");
	openResult = openfile("BTS");
	writeResult = writeBlock(openResult, "JIMINIE");

	//writeResult = writeBlock(openResult, "jungkook");

	printFatTable();
	printGlobalOpenFilePointer();
	createResult = createFile("EXO");	
	openResult = openfile("EXO");
	writeResult = writeBlock(openResult, "taoCHEN");
	printf("writefile() result: %d\n",writeResult);  // jungk tao00 CHANY SCoup ook00 

	createResult = createFile("SEVENTEEN");
	openResult = openfile("SEVENTEEN");
	writeResult = writeBlock(openResult, "SCoup"); 

		printf("partition # of blocks: %d\n", PARTITION->total_blocks);
	printf("partition size of blocks: %d\n", PARTITION->block_size);
	printf("# OF files: %d\n", PARTITION->numOfFiles);

	printFatTable();
	saveToDiskAndDisconnect();

	printf("================= AFTER SAVED TO HARDISK =========\n");
	//initIO();
	mount("test3.txt");
	printFatTable();
	copyHardDriveToBuffer();
	printBufferContent();

	printf("we are done!!\n");
	// printFile("BTS");

	// int readBlockResult = 0;
	// openResult=openfile("BTS");
	// while (readBlockResult >=0)
	// {
	// 	readBlockResult= readBlock(openResult);
	// 	printf("buffer result: %s\n", block_buffer);

	// }





	//int writeResult= writeBlock(openResult,"BTSISTHEBEST!AND I ALSO LOVE EXO"); //"BTSISTHEBEST!"
	// openResult = openfile("EXO");
	// printGlobalOpenFilePointer();
	// printUserDataSection();
	// printf("openfile() result: %d\n",openResult );
	// openResult = openfile("EXO");
	// printGlobalOpenFilePointer();

	
	
	// int readBlockResult=readBlock(openResult);
	// printf("readBlock() result: %d\n",readBlockResult );
	// printf("buffer result: %s\n", block_buffer);
	
	// writeBlock(openResult,"exo comebacklalahaha");
	// printUserDataSection();

	
	//READING FILE TEST CASES
	// test case 1: 
// 	/*int openResult = openfile("BTS");
// 	printf("openfile() result: %d\n",openResult );

// 	int readBlockResult=readBlock(openResult);
// 	printf("readBlock() result: %d\n",readBlockResult );

// 	printf("block Content: %s\n",block_buffer);

// 	// test case 2:
// 	printf("===================\n");
// 	openResult = openfile("RISE");
// 	printf("openfile() result: %d\n",openResult );

// 	readBlockResult=readBlock(openResult);
// 	printf("readBlock() result: %d\n",readBlockResult );
// 	return 0;
}*/