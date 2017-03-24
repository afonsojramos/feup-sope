#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>


int main(int argc, char *argv[] ){
    char* name_file;
    char file_type;
    int perm_type;
    int print = 0, delete = 0;
    
    return 0;
}

int verifyArgs(int argc, char *argv[]){
    int i = 1;
    for(; i < argc; i++)
    {
        if(*argv[i] == "-name"){
            name_file = argv[++i];
        }
        else if(*argv[i] == "-type"){
            if(*argv[i+1] != 'd' && *argv[i+1] != 'l' && *argv[i+1] != 'f')
                break;
            else
            file_type = *argv[++i];
        }
        else if(*argv[i] == "-perm"){
            perm_type = *argv[++i];
        }
        else if(*argv[i] == "-print"){
           //perm_typer
        }
        else if(*argv[i] == "-delete"){
           //perm_type = *argv[++i];
        }
    }
}