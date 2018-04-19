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
    data* myData = (data*) arg;
    int timer;
    pthread_mutex_lock(myData->mutex);
    pthread_cond_wait(myData->cond, myData->mutex);
    *myData->phaseDeJeu = 0;
    myData->grille = tirageGrille(myData->grille);
    printf("tirageGrille\n");
    pthread_mutex_unlock(myData->mutex);

    while (1)
    {
        for(timer = 20; timer >= 0; timer--)
        {
            printf("%d : ", timer);
            pthread_mutex_lock(myData->mutex);
            *myData->phaseDeJeu = 1;
            *myData->timer = timer;
            pthread_cond_broadcast(myData->cond);
            pthread_mutex_unlock(myData->mutex);
            sleep(1);
        }

        pthread_mutex_lock(myData->mutex);
        *myData->phaseDeJeu = 0;
        myData->grille = tirageGrille(myData->grille);
        printf("tirageGrille\n");
        pthread_cond_broadcast(myData->cond);
        pthread_mutex_unlock(myData->mutex);

        sleep(10);
    }

    return 0;
}

void* traiteClient(void *arg)
{
    data* myData = (data*) arg;
    char* tmp = malloc(sizeof(char) * 12 * 2);
    int nbSeconde;
    int nbMinute;

    pthread_mutex_lock(myData->mutex);
    pthread_cond_signal(myData->cond);
    pthread_mutex_unlock(myData->mutex);

    sleep(1);

    while(1)
    {
        printf("LectureGrille\n");
        int i;
        write(myData->sock, "newGrille\n", sizeof(char) * 10);
        for(i = 0; i < 16; i++)
        {
            write(myData->sock, &myData->grille[i], sizeof(myData->grille[i]));
        }
        write(myData->sock, "\n", sizeof(char));
        printf("EnvoiGrille\n");

        if(*myData->phaseDeJeu == 0)
        {
            pthread_mutex_lock(myData->mutex);
            pthread_cond_wait(myData->cond, myData->mutex);
            pthread_mutex_unlock(myData->mutex);
        }

        while(*myData->phaseDeJeu == 1)
        {

            nbSeconde = *myData->timer;
            printf("%d\n", nbSeconde);
            nbMinute = nbSeconde/60;
            nbSeconde = nbSeconde%60;

            sprintf(tmp, "%d : %d", nbMinute, nbSeconde);

            write(myData->sock, "newTimer\n", sizeof(char) * 9);
            write(myData->sock, tmp, sizeof(tmp));
            write(myData->sock, "\n", sizeof(char));

            pthread_mutex_lock(myData->mutex);
            pthread_cond_wait(myData->cond, myData->mutex);
            pthread_mutex_unlock(myData->mutex);


        }
    }

    return 0;
}

void* accepteClient(void *arg)
{
    printf("Coucou c'est moi la socket\n");
    data* serv = (data*) arg;
    while(1)
    {
        data* clientData = malloc(sizeof(data));
        socklen_t addrSize;
        addrSize = sizeof(clientData->addr);
        clientData->sock = accept(serv->sock, (struct sockaddr*)&(clientData->addr), &addrSize);
        clientData->grille = serv->grille;
        clientData->cond = serv->cond;
        clientData->mutex = serv->mutex;
        clientData->phaseDeJeu = serv->phaseDeJeu;
        clientData->timer = serv->timer;
        pthread_t pidT;
        pthread_create(&pidT, NULL, traiteClient, clientData);

        pthread_mutex_lock(serv->mutex);
        if(serv->nbJoueur == serv->nbJoueurMax)
        {
            data** tmp = malloc(sizeof(data) * serv->nbJoueurMax * 2);
            tmp = memcpy(tmp, serv->joueurs, serv->nbJoueurMax);
            free(serv->joueurs);
            serv->joueurs = tmp;
            serv->nbJoueurMax = serv->nbJoueurMax * 2;
        }

        serv->joueurs[serv->nbJoueur] = clientData;
        serv->nbJoueur++;
        pthread_mutex_unlock(serv->mutex);
    }

    return 0;
}

int main(int argc, char const *argv[])
{
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

    srand(time(NULL));
    //clock_t temps0;
    int* phaseDeJeu = malloc(sizeof(int));
    int* timer = malloc(sizeof(int));
    char* grille = malloc(sizeof(char) * 16);
    data** joueurs = malloc(sizeof(data) * 10);


    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t pidB;
    data* myData = malloc(sizeof(data));
    myData->grille = grille;
    myData->nbJoueur = 0;
    myData->nbJoueurMax = 10;
    myData->joueurs = joueurs;
    myData->cond = &cond;
    myData->mutex = &mutex;
    myData->phaseDeJeu = phaseDeJeu;
    myData->timer = timer;

    printf("Coucou c'est moi le main\n");
    pthread_create(&pidB, NULL, boggle, myData);

    pthread_t pidAC;
    myData->sock = mySocket;
    myData->addr = serv;

    pthread_create(&pidAC, NULL, accepteClient, myData);

    pthread_join(pidB, NULL);

    return 0;
}
