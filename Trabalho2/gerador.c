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

int n_pedidos;
int max_utilizacao;

int checkParameters(int argc,char *argv[])
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


}

void *generate_tickets(void *arg)
{
    int pedido_id = 1;
    char sexes[] = {'M','F'};
    char* ret;
    do{
    char pedido[MAX_MSG_LEN];
    char timestr[MAX_MSG_LEN]; 
    int timet = rand() % max_utilizacao + 1;
    char selected_sex = sexes[rand() %2];
    char sex;
    sprintf(pedido,"%d",pedido_id);
    sprintf(sex,"%c\0",selected_sex);
    sprintf(timestr,"%d",timet);
    strcat(pedido," ");
    strcat(sex," ");
    strcat(timestr,"\n");
    ret = concatStrings(pedido,sex);
    ret = concatStrings(ret,timestr);
    pedido_id++; 
    }while(pedido_id != n_pedidos);
    return ret;
}

char* concatStrings(const char *s1,const char *s2)
{
    char* result = malloc(strlen(s1)+strlen(s2)+1); //+1 for the \0 terminator
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

int main(int argc,char *argv[])
{
    int fd,n;
    char str[MAX_MSG_LEN];
    srand(time(NULL));
    pthread_t t_randomTickets, t_readResponse;

    pthread_create(&t_randomTickets,NULL,generate_tickets,NULL);
    char *requests;
    if(pthread_join(t_randomTickets,(void**)&requests) != 0)
        return -2;
    if(checkParameters(argc, argv) != 0){
      printf("The parameters are wrong.\n", );
    }

    if(createFifoEntrance() != 0)
        return -1;
    if(fd = open("/tmp/entrada",O_WRONLY)) == -1)
    {
        printf("Error opening FIFO\n");
        return -1;
    }

}
