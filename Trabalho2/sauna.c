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
int readline(int fd, char *str);
char *GENERATE_FIFO = "/tmp/entrada";
char *REJECTED_FIFO = "/tmp/rejeitados";

int checkParameters(int argc, char *argv[])
{

    if (argc != 2)
        return -1;
    n_lugares = atoi(argv[1]);
    return 0;
}


int readline(int fd, char *str)
{
    int n;
    char gender;

    do
    {
        n = read(fd, str, 1);
    } while (n > 0 && *str++ != '\n');

    return (n > 0);
}

int main(int argc, char *argv[])
{

    int fd, fifo_entrada;
    char str[MAX_MSG_LEN];

    if (checkParameters(argc, argv) != 0)
    {
        printf("The parameters are wrong on sauna.\n");
        return -1;
    }

    if (mkfifo("/tmp/rejeitados", 0666) < 0)
    {
        if (errno == EEXIST)
        {
            //printf("FIFO 'tmp/rejeitados' already exists\n!");
            return -1;
        }
        else
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }

    if (mkfifo(GENERATE_FIFO, 0666) < 0)
    {
        if (errno == EEXIST)
        {
           // printf("FIFO 'tmp/rejeitados' already exists!\n");
            
        }
        else
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }

   
   
   if((fifo_entrada = open(GENERATE_FIFO, O_RDONLY)) == -1)
   {
         printf("Error opening FIFO\n");
        return -1;
   } 

        
    
    int n = 1;
    do{
        n = read(fifo_entrada, str, MAX_MSG_LEN); 
        if(n != 0)
            printf("%s\n",str);
    }while(n!= 0);
    
    //close(fd);
    close(fifo_entrada);
 if(unlink("/tmp/rejeitados") < 0)
  {
      printf("Erro in destroying /tmp/rejeitados");
  }else printf("/tmp/rejeitados destroyed successfuly");

    /*while(readline(fd,str))
      printf("%s",str);*/
    exit(0);
}
