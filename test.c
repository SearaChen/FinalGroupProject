#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	int rv=strtol("2.3", NULL, 10);
	printf("%d\n",rv );
}