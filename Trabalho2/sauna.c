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

int checkParameters(int argc, char[] *argv){

    if(argc != 2)
    return -1;
    n_lugares = argv[1];
    return 0;
}

int createFifoEntrance()
{
    if(mkfifo("/tmp/rejeitados",0660) < 0){
       if (errno == EEXIST){
          printf("FIFO 'tmp/rejeitados' already exists\n!");
          return -1;
       } else {
           printf("Can't create FIFO\n");
           return -1;
       }
    } else
        return 0;

}

int readline(int fd, char *str) {

    int n;
    char gender;

    do {
      n = read(fd,str,1);
    } while (n>0 && *str++ != '\n');

    return (n>0);
}

int main(int argc, char[] *argv){
    if(argc != 2)

  int fd;
  char str[MAX_MSG_LEN];

  if(checkParameters(argc, argv) != 0){
    printf("The parameters are wrong.\n");
    return -1;
  }

  if(createFifoEntrance() != 0)
      return -1;

  if(fd = open("/tmp/rejeitados",O_RDONLY)) == -1)
  {
      printf("Error opening FIFO\n");
      return -1;
  }

  while(readline(fd,str))
      printf("%s",str);

}
