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

void* boggle(void *arg)
{
    printf("Coucou c'est moi le jeu\n");
    dataB* myData = (dataB*) arg;

    pthread_mutex_lock(myData->mutex);
    printf("Suspendu\n");
    pthread_cond_wait(myData->cond, myData->mutex);
    printf("Reprise\n");
    pthread_mutex_unlock(myData->mutex);

    return 0;
}

void* traiteClient(void *arg)
{
    data* myData = (data*) arg;

    pthread_mutex_lock(myData->mutex);
    printf("Je vasi te liberer\n");
    pthread_cond_signal(myData->cond);
    printf("Tu est libre\n");
    pthread_mutex_unlock(myData->mutex);

    return 0;
}

void* accepteClient(void *arg)
{
    printf("Coucou c'est moi la socket\n");
    data* serv = (data*) arg;
    //while(1)
    //{
        data* clientData = malloc(sizeof(data));
        socklen_t addrSize;
        addrSize = sizeof(clientData->addr);
        clientData->sock = accept(serv->sock, (struct sockaddr*)&(clientData->addr), &addrSize);
        clientData->grille = serv->grille;
        clientData->cond = serv->cond;
        clientData->mutex = serv->mutex;
        pthread_t pidT;
        pthread_create(&pidT, NULL, traiteClient, clientData);
    //}

    return 0;
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
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
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

    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t pidB;
    dataB* dataJeu = malloc(sizeof(dataB));
    dataJeu->grille = grille;
    dataJeu->nbJoueur = 0;
    dataJeu->cond = &cond;
    dataJeu->mutex = &mutex;

    printf("Coucou c'est moi le main\n");
    pthread_create(&pidB, NULL, boggle, dataJeu);

    pthread_t pidAC;
    data* myData = malloc(sizeof(data));
    myData->sock = mySocket;
    myData->addr = serv;
    myData->grille = grille;
    myData->cond = &cond;
    myData->mutex = &mutex;

    pthread_create(&pidAC, NULL, accepteClient, myData);

    pthread_join(pidB, NULL);

    return 0;
}
