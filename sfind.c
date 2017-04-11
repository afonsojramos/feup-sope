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
 char* perm_type;
 char initial_path;


 int verifyArgs(int argc, char *argv[]){
    int i = 2;
    initial_path = *argv[1];
    if(initial_path == '.' || initial_path == '/' || initial_path == '~')
    {
        
         for(; i < argc; i++)
     {
        if(strcmp(argv[i],"-name") == 0){
            name_file = argv[++i];
        }
        else if(strcmp(argv[i],"-type") == 0){
            if(*argv[i+1] != 'd' && *argv[i+1] != 'l' && *argv[i+1] != 'f')
            {
                return -1;
            
            }   
            else
            file_type = *argv[++i];
        }
        else if(strcmp(argv[i],"-perm") == 0){
            perm_type = argv[++i];
        }
        else if(strcmp(argv[i],"-print")==0){
                printf("Is to print\n");
                print = 1;
        }
        else if(strcmp(argv[i],"-delete") == 0){
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

/*char* search(struct stat buf)
{
    
    lstat(pwd,&buf);
    if(S_ISDIR(buf.st_mode))
   {

    
    
   }else if(S_ISREG(buf.st_mode))
   {

   }else if(S_ISLNK(buf.st_mode))
   {

   }
}*/

char* concatenateString(char* str1,char* str2)
{
    char* new_str;
    if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL)
    {
        new_str[0] = '\0';
        strcat(new_str,str1);
        strcat(new_str,str2);
    }else
    {
        printf("malloc failed!\n");
        exit(-1);
    }
    return new_str;
}

int main(int argc, char *argv[],char *envp[]){
    
   if(verifyArgs(argc,argv) == -1)
   {
       printf("Invalid arguments!\n");
       return -1;
   }
   char *cwd;
   size_t size = 256;
   char* pwd =  getcwd(cwd,size);   //getInitialPath(envp);
   struct stat buf;
   struct dirent *direntp;
   DIR *dirp;
   DIR *aux_dir;
   char *str;
   char name[200];
   pid_t pid;
   int status;

   if(argc < 2)
   {
       fprintf(stderr, "Usage: %s dir_name\n",pwd);
       exit(1);
   }

   if((dirp = opendir(pwd)) == NULL)
   {
       perror(pwd);
       exit(2);
   }

   while((direntp = readdir(dirp)) != NULL)
   {
       sprintf(name,"%s/%s",name_file,direntp->d_name);
       if(lstat(name,&buf) == -1)
       {
           perror("lstat ERROR");
           exit(3);
       }

       char *pathname = concatenateString(pwd,direntp->d_name);
       if(S_ISREG(buf.st_mode))
       {
           if(file_type == 'f' || direntp->d_name == name_file)
           {
               if(print != 0)
               printf(pathname);
               if(delete != 0)
               {
                   //TODO apaga ficheiro
                   if(rmdir(pathname) != 0)
                   {
                       perror("Error in deleting directory");
                       exit(5);
                   }
               }
           }
       }
       else if(S_ISDIR(buf.st_mode))
       {
           if(file_type == 'd' || direntp->d_name == name_file)
           {
                if(print != 0)
               printf(pathname);
               if(delete != 0)
               {
                   //TODO apaga ficheiro
                   if(rmdir(pathname) != 0)
                   {
                       perror("Error in deleting directory");
                       exit(5);
                   }
               }
           }
            else if((pid = fork()) < 0 ) fprintf(stderr,"fork error\n");
           else if (pid == 0)
           {
               //TODO recursivo usando exec
               if( chdir(pathname) != 0)
               {
                   perror("chdir ERROR");
                   exit(4);
               }
               execve("./sfind",argv,envp);
               
           }else 
           {
                wait(&status);
           }
       }
       else if(S_ISLNK(buf.st_mode))
       {
           if(file_type == 'l' || direntp->d_name == name_file )
           {
               if(print != 0)
                printf(pathname);
                if(delete != 0)
                {
                     if(rmdir(pathname) != 0)
                   {
                       perror("Error in deleting directory");
                       exit(5);
                   }
                }
           }
       }
   }

   closedir(dirp);

   

   
    return 0;
}





