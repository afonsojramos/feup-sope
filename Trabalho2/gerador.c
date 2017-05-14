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

int n_pedidos;
int n_le;
int max_utilizacao;
int fd;
int p;
char g;
int t;
int messagelen;
int ticket_id = 1;
char message[100];
struct timeval start, end;
struct stats my_stats;

char *concatStrings(const char *s1, const char *s2);
int checkParameters(int argc, char *argv[]);
void *generate_tickets(void *arg);
char *GENERATOR_FIFO = "/tmp/entrada";
char *REJECTED_FIFO = "/tmp/rejeitados";

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
    n_le = n_pedidos;
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
    float delta_us = (float)((float)end.tv_usec - start.tv_usec) / 1000;
    printf("%5.2f - %4d - %5d: %c - %5d - PEDIDO\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
    updatestats(ticket->sex, 0);
    pthread_exit(NULL);
    return NULL;
}

void *stay_opened(void *arg)
{

    open(GENERATOR_FIFO, O_WRONLY);
    printf("entro no sleep\n");
    sleep(999);
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

        // printf("tentar read\n");
        n = read(descriptor->fifo_rejeitados, ticket, sizeof(pedido));
        if (ticket->time_to_spend == -1)
            break;
        gettimeofday(&end, NULL);
        float delta_us = (float)((float)end.tv_usec - start.tv_usec) / 1000;
        printf("%5.2f - %4d - %5d: %c - %5d - REJEITADO\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
        updatestats(ticket->sex, 1);
        // printf("n: %d\n",n);
        //printf("rejeitados: %d\n",ticket->rejected);
        // printf("leu\n");
        // printf("rejeitado\n id: %d, sexo: %c, tempo: %d, rejeitados: %d",ticket->n_pedido,ticket->sex,ticket->time_to_spend,ticket->rejected);
        if (ticket->rejected <= 3)
        {
            // printf("escreveu");
            write(descriptor->fifo_entrada, ticket, sizeof(pedido));
            gettimeofday(&end, NULL);
            float delta_us = (float)((float)end.tv_usec - start.tv_usec) / 1000;
            printf("%5.2f - %4d - %5d: %c - %5d - PEDIDO\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
            updatestats(ticket->sex, 0);
        }
        else
        {
            gettimeofday(&end, NULL);
            float delta_us = (float)((float)end.tv_usec - start.tv_usec) / 1000;
            printf("%5.2f - %4d - %5d: %c - %5d - DESCARTADO\n", delta_us, getpid(), ticket->n_pedido, ticket->sex, ticket->time_to_spend);
            updatestats(ticket->sex, 2);
        }
        // printf("iteracao2\n");
    } while (n > 0);
    // printf("resposta da cena\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    gettimeofday(&start, NULL);
    if (checkParameters(argc, argv) != 0)
    {
        printf("Wrong number of arguments. Recomended usage: program_name <number of requests> <max duration>\n");
        return -1;
    }

    int fd;
    int rejeitados_fifo;
    int gerador_ficheiro;
    //int requests_nr = atoi(argv[1]);
    //int maxDuration = atoi(argv[2]);
    srand(time(NULL));
    descriptors *descritores = malloc(sizeof(descriptors));
    pthread_t t_randomTickets, t_readResponse;
    //int ticket_id = 1;
    printf("numero pedidos %d\n", n_pedidos);
    printf("tempo %d\n", max_utilizacao);

    if ((gerador_ficheiro = open("ger.pid", O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0666)) == -1)
    {
        printf("Error trying to create file\n");
        return -1;
    }

    int saved_stdout;

    if ((saved_stdout = dup(1)) == -1)
    {
        return -1;
    }
    if (dup2(gerador_ficheiro, STDOUT_FILENO) == -1)
    {
        printf("Error trying to duplicate\n");
        return -1;
    }
    // cria FIFO entrada de request para a sauna
    if (mkfifo(GENERATOR_FIFO, 0666) < 0)
    {
        if (errno == EEXIST)
        {
            printf("FIFO '/tmp/entrada' already exists!\n");
            return -1;
        }
        else
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }

    if (mkfifo(REJECTED_FIFO, 0666) < 0)
    {
        if (errno == EEXIST)
        {
            printf("FIFO '/tmp/rejeitados' already exists!\n");
            return -1;
        }
        else
        {
            printf("Can't create FIFO\n");
            return -1;
        }
    }

    //abre o FIFO criado no modo de leitura
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

    write(descritores->fifo_entrada, &n_le, sizeof(n_le));
    /*pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    pthread_create(&t_open,&attr,stay_opened,NULL);*/
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

    //printf("saiu gerador\n");
    //close(rejeitados_fifo);
    //  close(fd);
    close(gerador_ficheiro);

     if (dup2(saved_stdout, STDOUT_FILENO) == -1)
    {
        printf("Error trying to duplicate\n");
        return -1;
    }

    close(saved_stdout);
    printf("N Pedidos: %d em que %d sao masculinos e %d sao femininos\n",my_stats.n_pedidos_f+my_stats.n_pedidos_m,my_stats.n_pedidos_m,my_stats.n_pedidos_f);
    printf("N Rejeitados: %d em que %d sao masculinos e %d sao femininos\n",my_stats.n_rejeitados_f+my_stats.n_rejeitados_m,my_stats.n_rejeitados_m,my_stats.n_rejeitados_f);
    printf("N Descartados: %d em que %d sao masculinos e %d sao femininos\n",my_stats.n_descartados_f+my_stats.n_descartados_m,my_stats.n_descartados_m,my_stats.n_descartados_f);
    if (unlink("/tmp/entrada") < 0)
    {
        //printf("Error when destroying FIFO /tmp/entrada\n");
        return -1;
    }

    // else printf("Entrada fifo destryed succesfully\n");
    exit(0);
}
