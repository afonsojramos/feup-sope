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

int n_pedidos;
int max_utilizacao;
int fd;
int p;
char g;
int t;
int messagelen;
char message[100];

int checkParameters(int argc,char[] *argv)
{
    if(argc != 3)
        return -1;
    n_pedidos = argv[1];
    max_utilizacao = argv[2];
    return 0;
}

int createFifoEntrance()
{
    if(mkfifo("/tmp/entrada",0660) < 0)
    {
        if (errno == EEXIST){
          printf("FIFO 'tmp/entrada' already exists\n!");
        return -1;
    } else
    {
        printf("Can't create FIFO\n");
        return -1;
    }
    }else return 0;
    do {
      fd=open("/tmp/entrada",O_WRONLY);
      if (fd==-1) sleep(1);
    } while (fd==-1);

    //sprintf(message,"Serial Number: %d\nGender: %c\nRequested Duration: %d", p, g, t);

    messagelen=strlen(message)+1;
    write(fd,message,messagelen);

    close(fd);
}


int main(int argc,char[] *argv)
{
    int fd,n;
    char str[MAX_MSG_LEN];

    if(createFifoEntrance() != 0)
        return -1;
    if(fd = open("/tmp/entrada",O_WRONLY)) == -1)
    {
        printf("Erro opening FIFO\n");
        return -1;
    }

}
