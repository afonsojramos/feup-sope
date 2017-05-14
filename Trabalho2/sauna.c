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
#define GENERATE_FIFO "/tmp/entrada"
#define REJECTED_FIFO "/tmp/rejeitados"

int n_lugares;
int n_le;
int lugares_vagos;
char main_sex = 'S';
pthread_t tids[100000];
int tid_index = 0;
struct timeval start, end;
struct stats my_stats;
FILE *sauna_ficheiro;

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
    int n_servidos_m;
} stats;

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
    pedido *request = (pedido *)arg;

    usleep(request->time_to_spend * 1000);

    pthread_mutex_lock(&mutex);
    lugares_vagos++;
    if (lugares_vagos == n_lugares)
        main_sex = 'S';
    gettimeofday(&end, NULL);

    pthread_mutex_unlock(&mutex);
    double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
    printf("%6.2f - %4d - %20lu - %5d: %c - %5d - SERVED\n", delta_us, getpid(), (long)pthread_self(), request->n_pedido, request->sex, request->time_to_spend);
    fprintf(sauna_ficheiro, "%6.2f - %4d - %20lu - %5d: %c - %5d - SERVED\n", delta_us, getpid(), (long)pthread_self(), request->n_pedido, request->sex, request->time_to_spend);
    updatestats(request->sex, 2);

    pthread_exit(NULL);
}

float getTime(clock_t t1, clock_t t2)
{
    return ((float)(t2 - t1) / 1000000.0F) * 1000;
}

int main(int argc, char *argv[])
{

    gettimeofday(&start, NULL);
    int fifo_rejeitado, fifo_entrada;

    pid_t pid;
    pid = getpid();

    if (checkParameters(argc, argv) != 0)
    {
        printf("Wrong number of arguments. Recomended usage: sauna <number of spots>\n");
        return -1;
    }

    char file_name[MAX_MSG_LEN];
    sprintf(file_name, "/tmp/bal.%d", pid);

    if ((sauna_ficheiro = fopen(file_name, "a")) == NULL)
    {
        printf("Error trying to create file\n");
        return -1;
    }

    if (mkfifo(REJECTED_FIFO, 0666) < 0)
    {
        if (errno != EEXIST)
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }

    if (mkfifo(GENERATE_FIFO, 0666) < 0)
    {
        if (errno != EEXIST)
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

    do
    {

        pedido *ticket = malloc(sizeof(pedido));
        pthread_t entrance;

        if (read(fifo_entrada, ticket, sizeof(*ticket)) < 0)
            return -1;
        n_le--;

        gettimeofday(&end, NULL);
        double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
        printf("%6.2f - %4d - %20d - %5d: %c - %5d - RECEIVED\n", delta_us, pid, pid, ticket->n_pedido, ticket->sex, ticket->time_to_spend);
        fprintf(sauna_ficheiro, "%6.2f - %4d - %20d - %5d: %c - %5d - RECEIVED\n", delta_us, pid, pid, ticket->n_pedido, ticket->sex, ticket->time_to_spend);
        updatestats(ticket->sex, 0);

        if (main_sex == 'S')
        {
            //Entered without sex
            main_sex = ticket->sex;
            pthread_mutex_lock(&mutex);
            lugares_vagos--;
            pthread_mutex_unlock(&mutex);
            if (pthread_create(&entrance, NULL, stay_in_sauna, ticket) != 0)
            {
                printf("Error creating thread\n");
                return -1;
            }

            //Created Thread
            tids[tid_index] = entrance;
            tid_index++;
        }
        else
        {
            if (main_sex == ticket->sex)
            {

                if(lugares_vagos == 0)
                {

                }else if(lugares_vagos != 0)
                {
                     if (pthread_create(&entrance, NULL, stay_in_sauna, ticket) != 0)
                    {
                    printf("Error creating thread\n");
                    return -1;
                    }
                }
               
                tids[tid_index] = entrance;
                tid_index++;
                //Created Thread
            }
            else
            {  
                ticket->rejected++;
                gettimeofday(&end, NULL);
                double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
                printf("%6.2f - %4d - %20d - %5d: %c - %5d - REJECTED\n", delta_us, pid, pid, ticket->n_pedido, ticket->sex, ticket->time_to_spend);
                fprintf(sauna_ficheiro, "%6.2f - %4d - %20d - %5d: %c - %5d - RECEIVED\n", delta_us, pid, pid, ticket->n_pedido, ticket->sex, ticket->time_to_spend);
                updatestats(ticket->sex, 1);
                if (ticket->rejected <= 3)
                {
                    n_le++;
                }
                //Rejected
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
    //Left
    close(fifo_rejeitado);
    close(fifo_entrada);

    int i = 0;
    for (; i <= tid_index; i++)
    {
        pthread_join(tids[i], NULL);
            
    }

    printf("\n----------------------[FINAL SAUNA STATISTICS]-----------------------\n");
    printf("Number of Requested: %d in which %d are Male and  %d are Female\n", my_stats.n_pedidos_feitos_f + my_stats.n_pedidos_feitos_m, my_stats.n_pedidos_feitos_m, my_stats.n_pedidos_feitos_f);
    printf("Number of Rejected: %d in which %d are Male and  %d are Female\n", my_stats.n_rejeitados_f + my_stats.n_rejeitados_m, my_stats.n_rejeitados_m, my_stats.n_rejeitados_f);
    printf("Number of Served: %d in which %d are Male and  %d are Female\n", my_stats.n_servidos_f + my_stats.n_servidos_m, my_stats.n_servidos_m, my_stats.n_servidos_f);
    printf("---------------------------------------------------------------------\n");
    fclose(sauna_ficheiro);
    if (unlink("/tmp/rejeitados") < 0)
    {
        printf("Erro in destroying /tmp/rejeitados");
        return -1;
    }
    if (unlink("/tmp/entrada") < 0)
    {
        printf("Erro in destroying /tmp/entrada");
        return -1;
    }

    return 0;
}
