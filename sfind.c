#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <signal.h>

 int print = 0, delete = 0, execute_command = 0;
 char name_file[25];
 char file_type[2];
 char* perm_type;
char initial_path[sizeof(char*)];
char command[25];



 int verifyArgs(int argc, char *argv[]){
    int i = 2;
    //initial_path = argv[1];
    strcpy(initial_path,argv[1]);    
         for(; i < argc; i++)
     {
        if(strcmp(argv[i],"-name") == 0){
            //printf("%s\n",argv[i+1]);
            strcpy(name_file,argv[++i]);
        }
        else if(strcmp(argv[i],"-type") == 0){
            if(strcmp(argv[i+1],"d") != 0 && strcmp(argv[i+1],"l") != 0 && strcmp(argv[i+1],"f") != 0)
            {
                return -1;
            
            }   
            else
          //  file_type = *argv[++i];
          strcpy(file_type,argv[++i]);
        }
        else if(strcmp(argv[i],"-perm") == 0){
            strcpy(perm_type,argv[++i]);
        }
        else if(strcmp(argv[i],"-print")==0){
               // printf("Is to print\n");
                print = 1;
        }
        else if(strcmp(argv[i],"-delete") == 0){
           delete = 1;

        }else if(strcmp(argv[i],"-exec") == 0)
        {
            execute_command = 1;
            strcpy(command,argv[++i]);
        }
        else return -1;
    }
    return 0;
    
   
}

void signalHandler(int signo)
{
    
    printf("Do you want to quit? y/n : ");
    char answer[2];
    fgets(answer,2,stdin);
    if(strcmp(answer,"y") == 0 || strcmp(answer,"Y") == 0) 
    {

           exit(0);
    }
     
    else if(strcmp(answer,"n") == 0 || strcmp(answer,"N") == 0) {
       
        return;
    }

    else 
    {   
        perror("Invalid answer");
        exit(-1);
    }
}

char* concatenateString(char* str1,char* str2)
{
    char* new_str;
    //char* str_aux = "/";
    if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL)
    {
        new_str[0] = '\0';
        strcat(new_str,str1);
        //strcat(new_str,str_aux);
        strcat(new_str,str2);
    }else
    {
        printf("malloc failed!\n");
        exit(-1);
    }
    return new_str;
}

int main(int argc, char *argv[]){
    
   if(verifyArgs(argc,argv) == -1)
   {
       printf("Invalid arguments!\n");
       return -1;
   }
   
   
   char* pwd =   strcat(initial_path,"/");// getcwd(cwd,size);   //getInitialPath(envp);
   struct stat buf;
   struct dirent *direntp;
   DIR *dirp;
   pid_t pid;
   int status;
   
   signal(SIGINT,SIG_IGN);
     
   
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
      
         char *pathname = concatenateString(pwd,direntp->d_name);
        
      
       
       if(lstat(pathname,&buf) == -1)
       {
           perror("lstat ERROR");
           exit(3);
       }
        
       // printf("%s\n",pathname);

      
       if(S_ISREG(buf.st_mode))
       {
          // printf("e regular o %s\n",direntp->d_name);
           if(strcmp(file_type,"f") == 0 || strcmp(direntp->d_name,name_file) == 0)
           {
            
                if(execute_command != 0)
                {
                    printf("%s",command);
                   if( execlp(command,command,pathname,NULL) == -1)
                   {
                        perror("error executing program:");
                       exit(6);
                   }
                }
               if(print != 0)
               printf("%s\n",pathname);
               if(delete != 0)
               {
                  // printf("vai apagar");
                   
                   //TODO apaga ficheiro
                   if(unlink(pathname) != 0)
                   {
                    
                       perror("Error in deleting directory");
                       exit(5);
                   }
               }
           }
       }
       else if(S_ISDIR(buf.st_mode) && strcmp(direntp->d_name,".") != 0 && strcmp(direntp->d_name,"..") != 0)
       {
           //printf("e diretorio o %s\n", direntp->d_name);
           if(strcmp(file_type,"d") == 0 || strcmp(direntp->d_name,name_file) == 0)
           {
                if(execute_command != 0)
                {
                    printf("%s",command);
                   if( execl(command,command,pathname,NULL) == -1)
                   {
                       perror("error executing program:");
                       exit(6);
                   }
                }
                if(print != 0)
               printf("%s\n",pathname);
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
        if((pid = fork()) < 0 ) fprintf(stderr,"fork error\n");
           else if (pid == 0)
           {
               //TODO recursivo usando exec
              /* if( chdir(pathname) != 0)
               {
                   perror("chdir ERROR");
                   exit(4);
               }*/
            
           // main(argc,argv,envp);
           
          
            char **cp_argv ;
            size_t size = sizeof(*cp_argv) * (argc+1);
            cp_argv = malloc(size);
            memcpy(cp_argv,argv,size);
            cp_argv[1] = malloc(sizeof(pathname));
            strcpy(cp_argv[1],pathname);
          //argv[1] = pathname;
              if( execv(cp_argv[0],cp_argv) == -1)
              {
                  perror("execvp ERROR");
                  exit(6);
              }
              signal(SIGINT, SIG_IGN);
        //free(cp_argv);    
               
       }else 
           {
               
               //signal(SIGINT, SIG_IGN);
                wait(&status);
                struct sigaction act;
             act.sa_handler = signalHandler;
            sigemptyset(&act.sa_mask);
             act.sa_flags = 0;
             sigaction(SIGINT,&act,NULL);
                  
          }
       }
       else if(S_ISLNK(buf.st_mode))
       {
         //  printf("e link\n");
           if(strcmp(file_type,"l") == 0 || strcmp(direntp->d_name,name_file ) == 0)
           {
                if(execute_command != 0)
                {
                    printf("%s",command);
                    if( execlp(command,command,pathname,NULL) == -1)
                   {
                        perror("error executing program:");
                       exit(6);
                   }
                }
               if(print != 0)
                printf("%s\n",pathname);
                if(delete != 0)
                {
                   
                     if(unlink(pathname) != 0)
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





