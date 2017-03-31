#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/stat.h>

 int print = 0, delete = 0;
 char* name_file;
 char file_type;
 int perm_type;
 char initial_path;


 int verifyArgs(int argc, char *argv[]){
    int i = 2;

    initial_path = *argv[1];
    if(initial_path == '.' || initial_path == '/' || initial_path == '~')
    {
        
         for(; i < argc; i++)
     {
        if(*argv[i] == "-name"){
            if(*argv[i+1] == " ")
                return -1;
            else 
            name_file = argv[++i];
        }
        else if(*argv[i] == "-type"){
            if(*argv[i+1] != 'd' && *argv[i+1] != 'l' && *argv[i+1] != 'f')
            {
                return -1;
            
            }   
            else
            file_type = *argv[++i];
        }
        else if(*argv[i] == "-perm"){
            perm_type = *argv[++i];
        }
        else if(*argv[i] == "-print"){
                print = 1;
        }
        else if(*argv[i] == "-delete"){
           delete = 1;
        }else return -1;
    }
    return 0;
    }else return -1;
   
}

char* getInitialPath(char *envp[])
{
    char user[4];
 strcpy(user,"PWD");
   
  char** env = envp;
  char* substring;
	
	for(env = envp ; *env != 0;env++)
	{
		
		
		substring =strtok(*env,"=");
		if(strcmp(user,substring) == 0)
		{
			substring= strtok(NULL,"\0");
			printf("%s\n",substring);
		}
		
		
	}
    return substring;
}

char* search(struct stat buf)
{
    
    lstat(pwd,&buf);
    if(S_ISDIR(buf.st_mode))
   {

    
    
   }else if(S_ISREG(buf.st_mode))
   {

   }else if(S_ISLNK(buf.st_mode))
   {

   }
}

int main(int argc, char *argv[],char *envp[]){
    
   if(verifyArgs(argc,argv) == -1)
   {
       return -1;
   }
   char* pwd = getInitialPath(envp);
   struct stat buf;
   

   
    return 0;
}





