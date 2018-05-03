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

#define TAILLEBUFFER 256

void* boggle(void *arg)
{
    printf("Coucou c'est moi le jeu\n");
    data* myData = (data*) arg;
    int timer;
    pthread_mutex_lock(myData->mutex);
    *myData->phaseDeJeu = 0;
    myData->grille = tirageGrille(myData->grille);
    printf("tirageGrille\n");
    pthread_cond_wait(myData->cond, myData->mutex);
    pthread_mutex_unlock(myData->mutex);

    while (1)
    {
        for(timer = 20; timer >= 0; timer--)
        {
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
    char* buffer = malloc(sizeof(char) * TAILLEBUFFER);
    int nbSeconde;
    int nbMinute;

    pthread_mutex_lock(myData->mutex);
    pthread_cond_signal(myData->cond);
    pthread_mutex_unlock(myData->mutex);

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

        while(*myData->phaseDeJeu == 1)
        {
            pthread_mutex_lock(myData->mutex);
            pthread_cond_wait(myData->cond, myData->mutex);
            pthread_mutex_unlock(myData->mutex);

            nbSeconde = *myData->timer;
            nbMinute = nbSeconde/60;
            nbSeconde = nbSeconde%60;
            buffer = memset(buffer, 0, TAILLEBUFFER);
            sprintf(buffer, "%d : %d", nbMinute, nbSeconde);

            write(myData->sock, "newTimer\n", sizeof(char) * 9);
            write(myData->sock, buffer, sizeof(buffer));
            write(myData->sock, "\n", sizeof(char));

            buffer = memset(buffer, 0, TAILLEBUFFER);
            if(fcntl(myData->sock, F_SETFL, O_NONBLOCK) != -1)
            {
                read(myData->sock, buffer, sizeof(buffer));
                printf("%s", buffer);
            }
        }

        while(*myData->phaseDeJeu == 0)
        {
            buffer = memset(buffer, 0, TAILLEBUFFER);
            if(fcntl(myData->sock, F_SETFL, O_NONBLOCK) != -1)
            {
                read(myData->sock, buffer, sizeof(buffer));
            }
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

void enleveAccent(FILE* dico)
{
    char tmp;
    char* monAccent = malloc(sizeof(char) * 2);

    FILE* newDico = fopen("../ressources/sansaccent.txt", "w");

    tmp = fgetc(dico);

    while(feof(dico) == 0)
    {
        while(tmp != '|')
        {
            if(tmp < 0)
            {
                monAccent[0] = tmp;
                monAccent[1] = fgetc(dico);

                if(strcmp(monAccent, "ü") == 0 || strcmp(monAccent, "û") == 0 || strcmp(monAccent, "ù") == 0){fputc('u', newDico);}
                else if(strcmp(monAccent, "é") == 0 || strcmp(monAccent, "ê") == 0 || strcmp(monAccent, "ë") == 0 || strcmp(monAccent, "è") == 0){fputc('e', newDico);}
                else if(strcmp(monAccent, "ï") == 0 || strcmp(monAccent, "î") == 0){fputc('i', newDico);}
                else if(strcmp(monAccent, "ç") == 0){fputc('c', newDico);}
                else if(strcmp(monAccent, "ô") == 0 || strcmp(monAccent, "ö") == 0){fputc('o', newDico);}
                else if(strcmp(monAccent, "â") == 0 || strcmp(monAccent, "ä") == 0 || strcmp(monAccent, "à") == 0){fputc('a', newDico);}
            }
            else
            {
                fputc(tmp, newDico);
            }

            tmp = fgetc(dico);
        }

        while(tmp != '\n')
        {
            tmp = fgetc(dico);
        }

        fputc(tmp, newDico);

        while(tmp == '\n')
        {
            tmp = fgetc(dico);
        }
    }
}

int rechercheDansDico(char* mot, FILE* dico)
{
    int i = 0;
    int index = 0;
    char tmp;

    while(feof(dico) == 0)
    {
        tmp = fgetc(dico);

        while(mot[index] == tmp)
        {
            tmp = fgetc(dico);
            if(tmp == '\n' && mot[index + 1] == 0) return 1;
            else if(tmp == '\n' || mot[index + 1] == 0) break;
            index++;
        }

        while(tmp != '\n')
        {
            tmp = fgetc(dico);
            if(feof(dico) != 0) return 0;
        }
        printf("%d\n", i);
        i++;
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

    FILE* dico = fopen("../ressources/sansaccent.txt", "r");

    if(dico == NULL)
    {
        dico = fopen("../ressources/GLAFF-1.2.2/glaff-1.2.2.txt", "r");
        enleveAccent(dico);
        printf("fini\n");
    }

    dico = fopen("../ressources/sansaccent.txt", "r");

    printf("%d\n", rechercheDansDico("cyhjbkjgoczhpiefhkoazjsqbv", dico));

    pthread_join(pidB, NULL);

    return 0;
}
