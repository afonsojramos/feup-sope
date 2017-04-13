#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <wait.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <signal.h>


int print = 0, delete = 0, execute_command = 0;
char name_file[25];
char file_type[2];
int perm_type;
char initial_path[sizeof(char *)];
char command[25];
 pid_t pid;

int verifyArgs(int argc, char *argv[])
{
    int i = 2;
    //initial_path = argv[1];
    strcpy(initial_path, argv[1]);
    for (; i < argc; i++)
    {
        if (strcmp(argv[i], "-name") == 0)
        {
            //printf("%s\n",argv[i+1]);
            strcpy(name_file, argv[++i]);
        }
        else if (strcmp(argv[i], "-type") == 0)
        {
            if (strcmp(argv[i + 1], "d") != 0 && strcmp(argv[i + 1], "l") != 0 && strcmp(argv[i + 1], "f") != 0)
            {
                return -1;
            }
            else
                //  file_type = *argv[++i];
                strcpy(file_type, argv[++i]);
        }
        else if (strcmp(argv[i], "-perm") == 0)
        {
            char perm[10];
            strcpy(perm, argv[++i]);
            perm_type = atoi(perm);
        }
        else if (strcmp(argv[i], "-print") == 0)
        {
            // printf("Is to print\n");
            print = 1;
        }
        else if (strcmp(argv[i], "-delete") == 0)
        {
            delete = 1;
        }
        else if (strcmp(argv[i], "-exec") == 0)
        {
            execute_command = 1;
            strcpy(command, argv[++i]);
        }
        else
            return -1;
    }
    return 0;
}

void signalHandler(int signo)
{

    printf("Do you want to quit? y/n : ");
    char answer[2];
    fgets(answer, 2, stdin);
    if (strcmp(answer, "y") == 0 || strcmp(answer, "Y") == 0)
    {
        kill(pid,SIGKILL);
        exit(0);
    }

    else if (strcmp(answer, "n") == 0 || strcmp(answer, "N") == 0)
    {

        return;
    }

    else
    {
        perror("Invalid answer");
        signalHandler(SIGINT);
    }
}

int convertOctaltoDecimal(int octalnumber)
{
    int decimalNumber = 0, i = 0;

    while(octalnumber != 0)
    {
        decimalNumber += (octalnumber%10)* pow(8,i);  
        ++i;
        octalnumber /= 10;
    }
    return decimalNumber;
}

char *concatenateString(char *str1, char *str2)
{
    char *new_str;
    //char* str_aux = "/";
    if ((new_str = malloc(strlen(str1) + strlen(str2) + 1)) != NULL)
    {
        new_str[0] = '\0';
        strcat(new_str, str1);
        //strcat(new_str,str_aux);
        strcat(new_str, str2);
    }
    else
    {
        printf("malloc failed!\n");
        exit(-1);
    }
    return new_str;
}

int getPerm(mode_t bits)
{
    return (bits & S_IRUSR) | (bits & S_IWUSR) | (bits & S_IXUSR) | (bits & S_IRGRP) | (bits & S_IWGRP) | (bits & S_IXGRP) | (bits &  S_IROTH) | (bits & S_IWOTH) | (bits & S_IXOTH);
}

int permCheck(int perm, struct stat statRes)
{
   // if (stat(file, &statRes) < 0)
    //    return 1;
    mode_t bits = statRes.st_mode;
    if ((bits & S_IXOTH) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IWOTH) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IROTH) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IRWXO) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IXGRP) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IWGRP) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IRGRP) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IRWXG) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IWUSR) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IRUSR) == 0)
    {
        //Fazer cenas
    }
    if ((bits & S_IRWXU) == 0)
    {
        //Fazer cenas
    }
    return 0;
}

int main(int argc, char *argv[])
{

    if (verifyArgs(argc, argv) == -1)
    {
        printf("Invalid arguments!\n");
        return -1;
    }

    char *pwd = strcat(initial_path, "/"); 
    struct stat buf;
    struct dirent *direntp;
    DIR *dirp;
    struct sigaction act;
    act.sa_handler = signalHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    int status;
    perm_type = convertOctaltoDecimal(perm_type);

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s dir_name\n", pwd);
        exit(1);
    }

    if ((dirp = opendir(pwd)) == NULL)
    {
        perror(pwd);
        exit(2);
    }

    while ((direntp = readdir(dirp)) != NULL)
    {

        char *pathname = concatenateString(pwd, direntp->d_name);

        if (lstat(pathname, &buf) == -1)
        {
            perror("lstat ERROR");
            exit(3);
        }

        

        if (S_ISREG(buf.st_mode))
        {
           
            if (strcmp(file_type, "f") == 0 || strcmp(direntp->d_name, name_file) == 0 || perm_type == getPerm(buf.st_mode))
            {

                if (execute_command != 0)
                {
                    printf("%s", command);
                    if (execlp(command, command, pathname, NULL) == -1)
                    {
                        perror("error executing program:");
                        exit(6);
                    }
                }
                if (print != 0)
                    printf("%s\n", pathname);
                if (delete != 0)
                {
                 if (unlink(pathname) != 0)
                {
                        perror("Error in deleting directory");
                        exit(5);
                    }

                }
            }
        }
        else if (S_ISDIR(buf.st_mode) && strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0 )
        {
            
            if (strcmp(file_type, "d") == 0 || strcmp(direntp->d_name, name_file) == 0 || perm_type == getPerm(buf.st_mode))
            {
                if (execute_command != 0)
                {
                    printf("%s", command);
                    if (execlp(command, command, pathname, NULL) == -1)
                    {
                        perror("error executing program:");
                        exit(6);
                    }
                }
                if (print != 0)
                    printf("%s\n", pathname);
                if (delete != 0)
                {
    
                    if (rmdir(pathname) != 0)
                    {
                        perror("Error in deleting directory");
                        exit(5);
                    }
                }
            }
            if ((pid = fork()) < 0)
                fprintf(stderr, "fork error\n");
            else if (pid == 0)
            {
                signal(SIGINT,SIG_IGN);
                char **cp_argv;
                size_t size = sizeof(*cp_argv) * (argc + 1);
                cp_argv = malloc(size);
                memcpy(cp_argv, argv, size);
                cp_argv[1] = malloc(sizeof(pathname));
                strcpy(cp_argv[1], pathname);
                if (execv(cp_argv[0], cp_argv) == -1)
                {
                    perror("execvp ERROR");
                    exit(6);
                }

                free(cp_argv);
            }
            else
            {

                wait(&status);
                
                
            }
        }
        else if (S_ISLNK(buf.st_mode))
        {
            if (strcmp(file_type, "l") == 0 || strcmp(direntp->d_name, name_file) == 0 || perm_type == getPerm(buf.st_mode))
            {
                if (execute_command != 0)
                {
                    printf("%s", command);
                    if (execlp(command, command, pathname, NULL) == -1)
                    {
                        perror("error executing program:");
                        exit(6);
                    }
                }
                if (print != 0)
                    printf("%s\n", pathname);
                if (delete != 0)
                {

                    if (unlink(pathname) != 0)
                    {

                        perror("Error in deleting directory");
                        exit(5);
                    }
                }
            }
        }
    }

    closedir(dirp);

    return 0;
}
