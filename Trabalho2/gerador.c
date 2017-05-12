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
int fd;
int p;
char g;
int t;
int messagelen;
char message[100];

char *concatStrings(const char *s1, const char *s2);
int checkParameters(int argc, char *argv[]);
void *generate_tickets(void *arg);
char *GENERATOR_FIFO = "/tmp/entrada";
char *REJECTED_FIFO = "/tmp/rejeitados";

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


void *generate_tickets(void *arg)
{

    char sexes[] = {'M', 'F'};
    char *ret = "";
    char timestr[MAX_MSG_LEN];
    int timet = rand() % max_utilizacao + 1;

    char selected_sex = sexes[rand() % 2];
    char sex[2];
    sprintf(sex, "%c", selected_sex);

    sprintf(timestr, "%d", timet);
    strcat(sex, " ");

    ret = concatStrings(ret, sex);
    ret = concatStrings(ret, timestr);
    return ret;
}


int main(int argc, char *argv[])
{
    if (checkParameters(argc, argv) != 0)
    {
        printf("Wrong number of arguments. Recomended usage: program_name <number of requests> <max duration>\n");
    }

    //If pipe already exists
    if (mkfifo(GENERATOR_FIFO, S_IRUSR | S_IWUSR) != 0 && errno != EEXIST)
    {
        perror("Error creating GENERATOR FIFO");
        exit(-1);
    }

    int fd;
    int requests_nr = atoi(argv[1]);
    int maxDuration = atoi(argv[2]);
    char str[MAX_MSG_LEN];
    srand(time(NULL));
    pthread_t t_randomTickets, t_readResponse;
    int ticket_id = 1;
    char *requests;
    if (checkParameters(argc, argv) != 0)
    {
        printf("The parameters are wrong.\n");
    }
    printf("numero pedidos %d\n", n_pedidos);
    printf("tempo %d\n", max_utilizacao);

    // cria FIFO entrada de request para a sauna
    if (mkfifo(GENERATOR_FIFO, 0666) < 0)
    {
        if (errno == EEXIST)
        {
           // printf("FIFO '/tmp/entrada' already exists\n!");
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

    do
    {
        pthread_create(&t_randomTickets, NULL, generate_tickets, NULL);
        if (pthread_join(t_randomTickets, (void **)&requests) != 0)
            return -2;

        char ticket[MAX_MSG_LEN];
        sprintf(ticket, "%d ", ticket_id);

        strcat(ticket, requests);
        write(fd, ticket, MAX_MSG_LEN);
        //printf("ticket: %s\n", ticket);
        ticket_id++;

    } while (ticket_id <= n_pedidos);

    close(fd);
    if (unlink("/tmp/entrada") < 0)
        printf("Error when destroying FIFO /tmp/entrada\n");
    exit(0);
}
