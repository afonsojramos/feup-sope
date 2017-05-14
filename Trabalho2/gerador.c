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
#define GENERATOR_FIFO "/tmp/entrada"
#define REJECTED_FIFO "/tmp/rejeitados"

int n_pedidos;
int max_utilizacao;
int ticket_id = 1;
struct timeval start, end;
struct stats my_stats;
FILE *gerador_ficheiro;

char *concatStrings(const char *s1, const char *s2);
int checkParameters(int argc, char *argv[]);

typedef struct stats
{
    int n_pedidos_m;
    int n_pedidos_f;
    int n_rejeitados_f;
    int n_rejeitados_m;
    int n_descartados_f;
    int n_descartados_m;
} stats;

typedef struct pedido
{
    int n_pedido;
    char sex;
    int time_to_spend;
    int rejected;
} pedido;

typedef struct descriptors
{
    int fifo_entrada;
    int fifo_rejeitados;
} descriptors;

int checkParameters(int argc, char *argv[])
{
    if (argc != 3)
        return -1;
    char ped[MAX_MSG_LEN];
    strcpy(ped, argv[1]);
    n_pedidos = atoi(ped);
    max_utilizacao = atoi(argv[2]);
    return 0;
}

char *concatStrings(const char *str1, const char *str2)
{
    char *new_str;
    if ((new_str = malloc(strlen(str1) + strlen(str2) + 1)) != NULL)
    {

        new_str[0] = '\0';

        strcat(new_str, str1);
        strcat(new_str, str2);
    }
    else
    {
        printf("malloc failed!\n");
        exit(-1);
    }
    return new_str;
}

void updatestats(char sex, int type)
{
    if (type == 0)
    {
        if (sex == 'M')
            my_stats.n_pedidos_m++;
        else if (sex == 'F')
            my_stats.n_pedidos_f++;
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
            my_stats.n_descartados_m++;
        else if (sex == 'F')
            my_stats.n_descartados_f++;
    }
}

void *generate_tickets(void *arg)
{

    pedido *ticket = malloc(sizeof(pedido));
    descriptors *descriptor = malloc(sizeof(descriptors));
    descriptor = (descriptors *)arg;
    char sexes[] = {'M', 'F'};
    int timet = rand() % max_utilizacao + 1;

    char selected_sex = sexes[rand() % 2];
    ticket->sex = selected_sex;
    ticket->n_pedido = ticket_id;
    ticket->time_to_spend = timet;
    ticket->rejected = 0;

    write(descriptor->fifo_entrada, ticket, sizeof(*ticket));
    gettimeofday(&end, NULL);
    double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
    fprintf(gerador_ficheiro, "%8.2f - %4d - %5d: %c - %5d - REQUEST\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
    printf("%8.2f - %4d - %5d: %c - %5d - REQUEST\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
    updatestats(ticket->sex, 0);
    pthread_exit(NULL);
    return NULL;
}

void *response_ticket(void *arg)
{
    int n = 1;
    pedido *ticket = malloc(sizeof(pedido));
    descriptors *descriptor = malloc(sizeof(descriptors));
    descriptor = (descriptors *)arg;

    do
    {
        n = read(descriptor->fifo_rejeitados, ticket, sizeof(pedido));
        if (ticket->time_to_spend == -1)
            break;
        gettimeofday(&end, NULL);
        double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
        fprintf(gerador_ficheiro, "%8.2f - %4d - %5d: %c - %5d - REJECTED\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
        printf("%8.2f - %4d - %5d: %c - %5d - REJECTED\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
        updatestats(ticket->sex, 1);

        if (ticket->rejected <= 3)
        {
            write(descriptor->fifo_entrada, ticket, sizeof(pedido));
            gettimeofday(&end, NULL);
            double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
            fprintf(gerador_ficheiro, "%8.2f - %4d - %5d: %c - %5d - REQUESTED\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
            printf("%8.2f - %4d - %5d: %c - %5d - REQUESTED\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
            updatestats(ticket->sex, 0);
        }
        else
        {
            gettimeofday(&end, NULL);
            double delta_us = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
            fprintf(gerador_ficheiro, "%8.2f - %4d - %5d: %c - %5d - DISCARDED\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
            printf("%8.2f - %4d - %5d: %c - %5d - DISCARDED\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
            updatestats(ticket->sex, 2);
        }
    } while (n > 0);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    gettimeofday(&start, NULL);
    
    if (checkParameters(argc, argv) != 0)
    {
        printf("Wrong number of arguments. Recomended usage: gerador <number of requests> <max duration>\n");
        return -1;
    }

    int fd;
    int rejeitados_fifo;

    
    descriptors *descritores = malloc(sizeof(descriptors));
    pthread_t t_randomTickets, t_readResponse;
    printf("Number of Requests %d\n", n_pedidos);
    printf("Maximum Run Time %d\n", max_utilizacao);

    char file_name[MAX_MSG_LEN];
    sprintf(file_name, "/tmp/ger.%d", getpid());
    if ((gerador_ficheiro = fopen(file_name, "a")) == NULL)
    {
        printf("Error trying to create file\n");
        return -1;
    }

    //Created entrance FIFO request for the sauna
    if (mkfifo(GENERATOR_FIFO, 0666) < 0)
    {
        if (errno != EEXIST)
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }
    //Created rejected FIFO request for the sauna
    if (mkfifo(REJECTED_FIFO, 0666) < 0)
    {
        if (errno != EEXIST)
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }

    //Opens created FIFOs
    if ((fd = open(GENERATOR_FIFO, O_WRONLY)) == -1)
    {
        return -1;
    }
    if ((rejeitados_fifo = open(REJECTED_FIFO, O_RDONLY)) == -1)
    {
        return -1;
    }

    descritores->fifo_entrada = fd;
    descritores->fifo_rejeitados = rejeitados_fifo;

    write(descritores->fifo_entrada, &n_pedidos, sizeof(n_pedidos));

    while (ticket_id <= n_pedidos)
    {
        pthread_create(&t_randomTickets, NULL, generate_tickets, descritores);
        if (pthread_join(t_randomTickets, NULL) != 0)
            return -2;

        ticket_id++;
    }

    pthread_create(&t_readResponse, NULL, response_ticket, descritores);
    if (pthread_join(t_readResponse, NULL) != 0)
        return -2;

    fclose(gerador_ficheiro);

    printf("\n---------------------[FINAL GENERATOR STATISTICS]--------------------\n");
    printf("Number of Requests: %d in which %d are Male and %d are Female\n", my_stats.n_pedidos_f + my_stats.n_pedidos_m, my_stats.n_pedidos_m, my_stats.n_pedidos_f);
    printf("Number of Rejected: %d in which %d are Male and %d are Female\n", my_stats.n_rejeitados_f + my_stats.n_rejeitados_m, my_stats.n_rejeitados_m, my_stats.n_rejeitados_f);
    printf("Number of Discarded: %d in which %d are Male and %d are Female\n", my_stats.n_descartados_f + my_stats.n_descartados_m, my_stats.n_descartados_m, my_stats.n_descartados_f);
    printf("---------------------------------------------------------------------\n");

    if (unlink("/tmp/entrada") < 0)
    {
        return -1;
    }
    if (unlink("/tmp/rejeitados") < 0)
    {
        return -1;
    }

    return 0;
}
