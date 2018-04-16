#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <serveur.h>
#include <boggle.h>

void* traiteClient(void *arg)
{
    pid_t mainProcess = getppid();
    kill(mainProcess, SIGUSR1);

    return 0;
}

void* accepteClient(void *arg)
{
    data* serv = (data*) arg;
    while(1)
    {
        data* clientData = malloc(sizeof(data));
        socklen_t addrSize;
        addrSize = sizeof(clientData->addr);
        clientData->sock = accept(serv->sock, (struct sockaddr*)&(clientData->addr), &addrSize);
        clientData->grille = serv->grille;
        pthread_t pidT;
        pthread_create(&pidT, NULL, traiteClient, clientData);
    }
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    //clock_t temps0;
    char* grille = malloc(sizeof(char) * 16);

    if(argc < 2)
    {
        perror("Pas assez d'arguments");
        exit(errno);
    }

    int PORT = atoi(argv[1]);

    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == -1)
    {
        perror("socket()");
        exit(errno);
    }

    struct sockaddr_in serv = { 0 };
    serv.sin_addr.s_addr = htonl(INADDR_ANY); /* nous sommes un serveur, nous acceptons n'importe quelle adresse */
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    memset(serv.sin_zero, '\0', sizeof(serv.sin_zero));

    if(bind(mySocket, (struct sockaddr*) &serv, sizeof(serv)) == -1)
    {
        perror("bind()");
        exit(errno);
    }

    if(listen(mySocket, 5) == -1)
    {
        perror("listen()");
        exit(errno);
    }

    pthread_t pidT;
    data* myData = malloc(sizeof(data));

    myData->sock = mySocket;
    myData->addr = serv;
    myData->grille = grille;

    printf("coucou\n");
    pthread_create(&pidT, NULL, accepteClient, myData);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    printf("Suspendu\n");
    sigsuspend(&mask);
    printf("Reprise\n");

    return 0;
}
