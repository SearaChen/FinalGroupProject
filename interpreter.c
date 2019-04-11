/*
* Note: yet to handle nested commands, if want to, need to be able to EXTRACT the string value when doing print from memory
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "kernel.h"
#include "interpreter.h"
#include "shell.h"
#include "ram.h"
#include "memorymanager.h"
#include "IOSCHEDULER.h"


#define MAXCHAR 10000

int help()
{
	return 1;
}

int quit()
{
	printf("Bye!\n");
	IOSave();
	//free(cpu);
	exit(EXIT_SUCCESS);
}

int set(char* varName, char* value) // set VAR STRING
{
	printf("value in set read: %s\n",value);

	writeToMemory(varName, value);
	return 1;
}

int print(char* varName)
{
	printFromMemory(varName);
	return 1;
}

int run(char* filepath)
{
	char *line;
	char **args;
	char str[MAXCHAR];

	FILE *fp;

	if( access( filepath, F_OK ) != -1 ) {
    	fp = fopen(filepath, "r");
    	while (fgets(str, MAXCHAR, fp) != NULL )
    	{
    		args = parse(str);
    		execute(args);
    	}
    	fclose(fp);
	} else {
    	printf("Script not found!\n");
	}
	return 1;
}

int exec(int commandWordCount, char **args)
//int exec()
{
	int i =1; 

	// queue all programs
	while (i < commandWordCount)
	{
		FILE *fp ;
		if( access( args[i], F_OK ) == -1 )
		{
			printf("Script %s not found!\n",args[i]);
			i++;
		}
		else
		{
	    	fp = fopen(args[i], "r");
	    	launcher(fp);

			i++;
		}
	}
	
	// performing robin-go-round
	while(READYHEAD != (ReadyQueueNode *) NULL && READYHEAD !=  NULL )
	{	
		int status;
		status = scheduler();

		if (status == -1)
		{
			printf("CPU is somehow not available error!");
		}
	}

	// wipe backing store
	wipeBackingStore();
	
	return 1;
}


int mountInterpreter(char* partitionName, int number_of_blocks, int block_size)
{
	IOMounter(partitionName, number_of_blocks, block_size);
	return 1;
}

int writeInterpreter(int commandWordCount,char ** args)
{
	// concatenate all of the args to the data
	int i=0;
	char filename[500];
	strcpy(filename, args[1]);
	char* data[commandWordCount*100];
	for(i= 2; i < commandWordCount; i++)
	{	
		strcpy(data, args[i]);
	}
	IOWriteScheduler(data,filename,NULL);
	// // check if it is already open 
	// if(fatIndex == -1) // not open yet
	// {
	// 	fatIndex  = openFile(filename); 
	// 	if (fatIndex == -1) // no such filename
	// 	{
	// 		fatIndex = createFile(filename);
	// 		if (fatIndex == -1)  // fat is full
	// 		{
	// 			printf("Error! FAT is full to create file: %s", filename);
	// 		}
	// 		fatIndex  = openFile(filename); 
	// 	}

	// 	if (fatIndex == -2)
	// 	{
	// 		printf("Error! fp is full to open file: %s\n",filename );
	// 	}
	// }

	// writeBlock(fatIndex, data);
	return 1;
}


int readInterpreter(char* filename, char*variableName)
{
	int IOResult=IOReadscheduler(filename, NULL);
	if(IOResult>=0)
	{
		char* resultData = getBufferResult();
		printf("result data from read: %s\n",resultData);
		printf("variableName: %s\n", variableName);
		set(variableName, resultData);

	}
	return 1;
}



int interpret(char **args)
{
	// counting the number of words input in a line
	int commandWordCount= 0;
	int status;
	while(args[commandWordCount] != NULL)
	{
		commandWordCount++;
	}

	// it is an empty line
	if (commandWordCount == 0)
	{
		return 1;
	}

	if (strcmp(args[0],"help") == 0)
	{
		status = help();
		if (status == 1)
		{
			printf("help: Displays all the commands\nquit: Exits / terminates the shell with \"Bye!\"\nset: Assigns a value to shell memory\nprint: Prints the STRING assigned to VAR\nrun: Executes the file SCRIPT.TXT\nexec: execute up to 3 programs at the same time\n");
		}
		return status;
	}
	else if(strcmp(args[0],"quit") == 0)
	{
		status = quit();  // the program actually never goes here 
		return status;
	}
	else if(strcmp(args[0],"set") == 0 && commandWordCount == 3)
	{
		status = set(args[1], args[2]);
		return status;
	}
	else if (strcmp(args[0],"print") == 0 && commandWordCount == 2)
	{
		status = print(args[1]);
		return status;
	}
	else if (strcmp(args[0],"run") == 0 && commandWordCount == 2)
	{
		status= run(args[1]);
		return status;
	}
	else if(strcmp(args[0],"exec") == 0 && (commandWordCount > 1 && commandWordCount < 5))
	{
		status= exec(commandWordCount, args);
		return status;
	}
	else if(strcmp(args[0],"Mount") == 0 )//&& commandWordCount ==4) //• Mount partitionName number_of_blocks block_size
	{
		int number_of_blocks = strtol(args[2], NULL, 20);
		int block_size = strtol(args[3], NULL, 20);
		if (number_of_blocks <=0 || block_size <=0 )
		{
			printf("Please enter two valid positive number!\n");
			return 1;
		}

		status = mountInterpreter(args[1], number_of_blocks,block_size);
		return status;

	}
	else if(strcmp(args[0],"Write") == 0 && commandWordCount > 2)  //Write filename [a bunch of words]
	{
		status = writeInterpreter(commandWordCount ,args);
		return status;

	}
	else if(strcmp(args[0],"Read") == 0 && commandWordCount == 3)// Read filename variable
	{
		status = readInterpreter(args[1],args[2]);
		//"EXO","bestband");
		return status;
	}
	else
	{
		printf("Unknown command\n");
		return 1;
	}

}
