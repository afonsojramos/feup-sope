#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define MAX_MSG_LEN 1000
#define READ 0
#define WRITE 1


int n_lugares;

int checkParameters(int argc,char[] *argv)
{
    if(argc != 2)
    return -1;
    n_lugares = argv[1];
    return 0;
}
int main(int argc, char[] *argv)
{
    
}