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
#include <sys/time.h>

#define MAX_MSG_LEN 1000
#define READ 0
#define WRITE 1

typedef struct pedido
{
    int n_pedido;
    char sex;
    int time_to_spend;
    int rejected;
} pedido;

typedef struct stats
{
    int n_pedidos_feitos_m;
    int n_pedidos_feitos_f;
    int n_rejeitados_f;
    int n_rejeitados_m;
    int n_servidos_f;
    int n_servidos_m ;
} stats;

int n_lugares;
int n_le;
int lugares_vagos;
char main_sex = 'S';
pthread_t tids[255];
int tid_index = 0;
struct timeval start, end;
struct stats my_stats;

int readline(int fd, char *str);
char *GENERATE_FIFO = "/tmp/entrada";
char *REJECTED_FIFO = "/tmp/rejeitados";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int checkParameters(int argc, char *argv[])
{

    if (argc != 2)
        return -1;
    char ped[MAX_MSG_LEN];
    strcpy(ped, argv[1]);
    n_lugares = atoi(ped);
    lugares_vagos = n_lugares;
    return 0;
}

void updatestats(char sex, int type)
{
    if (type == 0)
    {
        if (sex == 'M')
            my_stats.n_pedidos_feitos_m++;
        else if (sex == 'F')
            my_stats.n_pedidos_feitos_f++;
    }
    else if (type == 1)
    {
        if (sex == 'M')
            my_stats.n_rejeitados_m++;
        else if (sex == 'F')
            my_stats.n_rejeitados_f++;
    }
    else if (type == 2)
    {
        if (sex == 'M')
            my_stats.n_servidos_m++;
        else if (sex == 'F')
            my_stats.n_servidos_f++;
    }
}

void *stay_in_sauna(void *arg)
{
    //void *ret;
    pedido *request = (pedido *)arg;
    //printf("entrou thread id %d no sleep\n",request->n_pedido);

    sleep(request->time_to_spend / 1000);
    // sleep(5);
    // printf("tempo %d\n",request->time_to_spend);
    // printf("saiu o id %d \n",request->n_pedido);
    pthread_mutex_lock(&mutex);
    lugares_vagos++;
    if (lugares_vagos == n_lugares)
        main_sex = 'S';
        gettimeofday(&end, NULL);
    float delta_us = (float)(((float)end.tv_usec - start.tv_usec) / 1000);
    printf("%5.2f - %4d - %20lu - %5d: %c - %5d - SERVIDO\n", delta_us, getpid(), (long)pthread_self(), request->n_pedido, request->sex, request->time_to_spend);
    pthread_mutex_unlock(&mutex);
    updatestats(request->sex, 2);
    
    pthread_exit(NULL);
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

float getTime(clock_t t1, clock_t t2)
{
    return ((float)(t2 - t1) / 1000000.0F) * 1000;
}

int main(int argc, char *argv[])
{

    gettimeofday(&start, NULL);
    int fifo_rejeitado, fifo_entrada;

    int sauna_ficheiro;
    pid_t pid;
    pid = getpid();
    // char main_sex = 'S';

    if (checkParameters(argc, argv) != 0)
    {
        printf("The parameters are wrong on sauna.\n");
        return -1;
    }

    if ((sauna_ficheiro = open("bal.pid", O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0666)) == -1)
    {
        printf("Error trying to create file\n");
        return -1;
    }

    int saved_stdout;

    if ((saved_stdout = dup(1)) == -1)
    {
        return -1;
    }

    if (dup2(sauna_ficheiro, STDOUT_FILENO) == -1)
    {
        printf("Error trying to duplicate\n");
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

    read(fifo_entrada, &n_le, sizeof(n_le));
    // printf("n_le: %d\n",n_le);

    int n = 1;
    do
    {

        pedido *ticket = malloc(sizeof(pedido));
        pthread_t entrance;

        n = read(fifo_entrada, ticket, sizeof(*ticket));
        n_le--;

        gettimeofday(&end, NULL);
        float delta_us = (float)(((float)end.tv_usec - start.tv_usec) / 1000);
        printf("%5.2f - %4d - %20d - %5d: %c - %5d - RECEBIDO\n", delta_us, pid, pid, ticket->n_pedido, ticket->sex, ticket->time_to_spend);
        updatestats(ticket->sex, 0);

        //printf("%d",pid);
        //write(sauna_ficheiro,"dick\n",5);
        //printf("escreve\n");
        // char *cena = malloc(sizeof(char*));
        // read(sauna_ficheiro,cena,sizeof(cena));
        //  printf("%s",cena);
        //printf("\n");
        //  printf("sauna n: %d\n",n);
        if (n == 0)
            break;
        // printf("id: %d ",ticket->n_pedido);
        // printf("sex: %c ",ticket->sex);
        // printf("time: %d ",ticket->time_to_spend);
        // printf("rejected: %d\n",ticket->rejected);

        if (main_sex == 'S')
        {
            //printf("entrou sem sexo\n");
            main_sex = ticket->sex;
            pthread_mutex_lock(&mutex);
            lugares_vagos--;
            pthread_mutex_unlock(&mutex);
            if (pthread_create(&entrance, NULL, stay_in_sauna, ticket) != 0)
            {
                printf("error creating thread\n");
                return -1;
            }

            tids[tid_index] = entrance;
            tid_index++;
            // printf("criou thread\n");
        }
        else
        {
            if (main_sex == ticket->sex && lugares_vagos != 0)
            {

                if (pthread_create(&entrance, NULL, stay_in_sauna, ticket) != 0)
                {
                    printf("erro creating thread");
                    return -1;
                }
                tids[tid_index] = entrance;
                // printf("criou thread\n");
            }
            else
            {
                ticket->rejected++;
                gettimeofday(&end, NULL);
                float delta_us = (float)(((float)end.tv_usec - start.tv_usec) / 1000);
                printf("%5.2f - %4d - %20d - %5d: %c - %5d - REJEITADO\n", delta_us, pid, pid, ticket->n_pedido, ticket->sex, ticket->time_to_spend);
                updatestats(ticket->sex, 1);
                if (ticket->rejected <= 3)
                {
                    n_le++;
                }
                // printf("mandei para rejeitado\n");
                if (write(fifo_rejeitado, ticket, sizeof(pedido)) == -1)
                {
                    printf("Error on writing\n");
                    return -1;
                }
            }
        }
        // printf("N_LE: %d\n",n_le);
        if (n_le == 0)
        {
            pedido *ultimo = malloc(sizeof(pedido));
            ultimo->time_to_spend = -1;
            //printf("FEZ O \n");
            write(fifo_rejeitado, ultimo, sizeof(pedido));
        }

    } while (n_le != 0);
    //printf("saiu\n");
    close(fifo_rejeitado);
    close(fifo_entrada);
    
    int i = 0;
    while (i <= tid_index)
    {

        if(pthread_join(tids[i], NULL) != 0)
            return -2;
        i++;
    }
    close(sauna_ficheiro);
   /*do{
        
    }while(n_lugares != lugares_vagos);*/
 /* if (dup2(saved_stdout, STDOUT_FILENO) == -1)
    {
        printf("Error trying to duplicate\n");
        return -1;
    }*/



    close(saved_stdout);
   
    printf("N Pedidos: %d em que %d sao masculinos e %d sao femininos\n",my_stats.n_pedidos_feitos_f+my_stats.n_pedidos_feitos_m,my_stats.n_pedidos_feitos_m,my_stats.n_pedidos_feitos_f);
    printf("N Rejeitados: %d em que %d sao masculinos e %d sao femininos\n",my_stats.n_rejeitados_f+my_stats.n_rejeitados_m,my_stats.n_rejeitados_m,my_stats.n_rejeitados_f);
    printf("N Servidos: %d em que %d sao masculinos e %d sao femininos\n",my_stats.n_servidos_f+my_stats.n_servidos_m,my_stats.n_servidos_m,my_stats.n_servidos_f);

    if (unlink("/tmp/rejeitados") < 0)
    {
        //printf("Erro in destroying /tmp/rejeitados");
        return -1;
    }
    else
        // printf("/tmp/rejeitados destroyed successfuly");

        /*while(readline(fd,str))
      printf("%s",str);*/
        exit(0);
}
