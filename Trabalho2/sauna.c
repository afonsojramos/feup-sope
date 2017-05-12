#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


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


void *stay_in_sauna(void *arg)
{
    //void *ret;
    int value;

    value = *(int*) arg;
    sleep(value);
    pthread_exit(NULL);
    return NULL;
}

int readline(int fd, char *str)
{
    int n = 1;
    //char gender;

    do
    {
        n = read(fd, str, MAX_MSG_LEN);
    } while (n != 0);

    return (n > 0);
}

int main(int argc, char *argv[])
{

    int fifo_rejeitado, fifo_entrada;
    char str[MAX_MSG_LEN];
    char *main_sex = "S";

    if (checkParameters(argc, argv) != 0)
    {
        printf("The parameters are wrong on sauna.\n");
        return -1;
    }

    if (mkfifo(REJECTED_FIFO, 0666) < 0)
    {
        if (errno == EEXIST)
        {
            //printf("FIFO 'tmp/rejeitados' already exists\n!");
            
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

    if ((fifo_entrada = open(GENERATE_FIFO, O_RDONLY)) == -1)
    {
        printf("Error opening FIFO\n");
        return -1;
    }

    if ((fifo_rejeitado = open(REJECTED_FIFO, O_WRONLY)) == -1)
    {
        printf("Error opening FIFO\n");
        return -1;
    }

    int n = 1;
    do
    {
        char aux_str[MAX_MSG_LEN];
        int time_to_spend;
        char *sex;
        pthread_t entrance;
        n = read(fifo_entrada, str, MAX_MSG_LEN);
        strncpy(aux_str,str,n+1);
        if(strcmp(main_sex,"S") == 0)
        {
            main_sex = strtok(aux_str," ");
            main_sex = strtok(NULL," ");
            time_to_spend = atoi(strtok(NULL," "));
           pthread_create(&entrance,NULL,stay_in_sauna,&time_to_spend);
            printf("o primeiero sexo a entrar e %s\n",main_sex);
            printf("o primeiero tempo a entrar e %d\n",time_to_spend);
        }else
        {
            sex = strtok(aux_str," ");
            sex = strtok(NULL," ");
            if(sex == main_sex)
            {
                time_to_spend = atoi(strtok(NULL," "));
               pthread_create(&entrance,NULL,stay_in_sauna,&time_to_spend);
            }else
            {
              /* if( write(fifo_rejeitado,str,MAX_MSG_LEN) == -1)
                {
                    perror("Error on writing\n");
                    return -1;
                }*/
            }
        }
            if (n != 0)
                printf("%s\n", str);

    } while (n != 0);

    close(fifo_rejeitado);
    close(fifo_entrada);
    if (unlink("/tmp/rejeitados") < 0)
    {
        printf("Erro in destroying /tmp/rejeitados");
    }
    else
        printf("/tmp/rejeitados destroyed successfuly");

    /*while(readline(fd,str))
      printf("%s",str);*/
    exit(0);
}
