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

    fclose(newDico);
}

int rechercheDansDico(char* mot, FILE* dico)
{
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
    }

    return 0;
}

int recherchePseudo(dataClient** joueurs, int nbJoueur, char* pseudo)
{
    int i;

    for(i = 0; i < nbJoueur; i++)
    {
        if(strcmp(pseudo, joueurs[i]->pseudo) == 0)
        {
            return 1;
        }
    }

    return 0;
}

void extractPseudo(char* tab)
{
    int i;
    int debut = 257;
    int fin = 0;

    for(i = 0; i < TAILLEBUFFER; i++)
    {
        if(debut == 257 && tab[i] == '/')
        {
            debut = ++i;
        }
        else if(fin == 0 && tab[i] == '/')
        {
            fin = i - 1;
        }

        if(tab[i] == 0)break;
    }

    for(i = 0; i < TAILLEBUFFER; i++)
    {
        if(tab[i] == 0)break;
        if(i > fin-debut)tab[i] = 0;
        else tab[i] = tab[debut + i];
    }
}

void* boggle(void *arg)
{
    printf("Coucou c'est moi le jeu\n");
    dataServ* myData = (dataServ*) arg;
    int timer;
    int nbSession = 0;
    int nbJoueur = 0;
    pthread_mutex_lock(myData->mutex);
    *myData->phaseDeJeu = 0;
    myData->grille = tirageGrille(myData->grille);
    printf("tirageGrille\n");
    pthread_cond_wait(myData->cond, myData->mutex);
    pthread_mutex_unlock(myData->mutex);

    printf("%d : %d\n", myData->nbJoueur, nbJoueur);

    while(nbSession < myData->nbSession)
    {
        for(timer = myData->nbMinute * 60; timer >= 0; timer--)
        {
            if(myData->nbJoueur > nbJoueur)
            {
                if(recherchePseudo(myData->joueurs, nbJoueur, myData->joueurs[nbJoueur]->pseudo) == 0)
                {
                    printf("C'est Bon !\n");
                    myData->joueurs[nbJoueur]->valide = 1;
                    pthread_mutex_lock(myData->mutex);
                    pthread_cond_signal(myData->joueurs[nbJoueur]->condClient);
                    pthread_mutex_unlock(myData->mutex);
                    nbJoueur++;
                }
                else
                {
                    printf("C'est pas Bon !\n");
                    pthread_mutex_lock(myData->mutex);
                    pthread_cond_signal(myData->joueurs[nbJoueur]->condClient);
                    pthread_mutex_unlock(myData->mutex);
                }
            }

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
        nbSession++;
    }

    return 0;
}

void* traiteClient(void *arg)
{
    dataClient* myData = (dataClient*) arg;
    myData->pseudo = malloc(sizeof(char) * TAILLEBUFFER);
    char* buffer = malloc(sizeof(char) * TAILLEBUFFER);
    int nbSeconde;
    int nbMinute;
    int nbSession = 0;

    buffer = memset(buffer, 0, TAILLEBUFFER);
    read(myData->sock, buffer, TAILLEBUFFER);

    extractPseudo(buffer);
    memcpy(myData->pseudo, buffer, TAILLEBUFFER);
    printf("Coucou toi : %s\n", myData->pseudo);

    pthread_mutex_lock(myData->mutexServ);
    pthread_cond_signal(myData->condServ);
    pthread_cond_wait(myData->condClient, myData->mutexServ);
    pthread_mutex_unlock(myData->mutexServ);

    while(myData->valide == 0)
    {
        write(myData->sock, "newPseudoRequired\n", sizeof(char) * 18);
        buffer = memset(buffer, 0, TAILLEBUFFER);
        read(myData->sock, buffer, TAILLEBUFFER);

        extractPseudo(buffer);
        memcpy(myData->pseudo, buffer, TAILLEBUFFER);

        pthread_mutex_lock(myData->mutexClient);
        pthread_cond_wait(myData->condClient, myData->mutexClient);
        pthread_mutex_unlock(myData->mutexClient);
    }

    fcntl(myData->sock, F_SETFL, O_NONBLOCK);

    while(nbSession < myData->nbSession)
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
            pthread_mutex_lock(myData->mutexServ);
            pthread_cond_wait(myData->condServ, myData->mutexServ);
            pthread_mutex_unlock(myData->mutexServ);

            nbSeconde = *myData->timer;
            nbMinute = nbSeconde/60;
            nbSeconde = nbSeconde%60;
            buffer = memset(buffer, 0, TAILLEBUFFER);
            sprintf(buffer, "%d : %d", nbMinute, nbSeconde);

            write(myData->sock, "newTimer\n", sizeof(char) * 9);
            write(myData->sock, buffer, sizeof(buffer));
            write(myData->sock, "\n", sizeof(char));

            buffer = memset(buffer, 0, TAILLEBUFFER);
            read(myData->sock, buffer, sizeof(buffer));

            printf("%s", buffer);
        }

        while(*myData->phaseDeJeu == 0)
        {
            read(myData->sock, buffer, sizeof(buffer));
        }
        nbSession++;
    }

    return 0;
}

void* accepteClient(void *arg)
{
    printf("Coucou c'est moi la socket\n");
    dataServ* serv = (dataServ*) arg;
    while(1)
    {
        dataClient* clientData = malloc(sizeof(dataClient));
        socklen_t addrSize;
        addrSize = sizeof(clientData->addr);
        clientData->sock = accept(serv->sock, (struct sockaddr*)&(clientData->addr), &addrSize);
        clientData->grille = serv->grille;
        clientData->condServ = serv->cond;
        clientData->mutexServ = serv->mutex;
        clientData->phaseDeJeu = serv->phaseDeJeu;
        clientData->timer = serv->timer;
        clientData->nbSession = serv->nbSession;
        clientData->valide = 0;

        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        clientData->condClient = &cond;
        clientData->mutexClient = &mutex;

        pthread_t pidT;
        pthread_create(&pidT, NULL, traiteClient, clientData);

        pthread_mutex_lock(serv->mutex);
        if(serv->nbJoueur == serv->nbJoueurMax)
        {
            dataClient** tmp = malloc(sizeof(dataClient) * serv->nbJoueurMax * 2);
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

int main(int argc, char * const argv[])
{
    int opt;
    int nbSession = 5;
    int nbMinute = 1;
    int port = 2018;
    char* pathDico = NULL;

    while((opt = getopt(argc, argv, "s:t:p:d:")) != -1)
    {
        switch(opt)
        {
            case 's':
                nbSession = atoi(optarg);
                break;
            case 't':
                nbMinute = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                pathDico = optarg;
                break;
            case '?':
                printf("Argument d'option manquant, la valeur par defaut sera utiliser !\n");
        }
    }

    if(pathDico != NULL)
    {
        enleveAccent(fopen(pathDico, "r"));
    }

    FILE* dico = fopen("../ressources/sansaccent.txt", "r");

    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == -1)
    {
        perror("socket()");
        exit(errno);
    }

    struct sockaddr_in serv = { 0 };
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
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
    dataClient** joueurs = malloc(sizeof(dataClient*) * 10);


    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t pidB;
    dataServ* myData = malloc(sizeof(dataServ));
    myData->grille = grille;
    myData->nbJoueur = 0;
    myData->nbJoueurMax = 10;
    myData->joueurs = joueurs;
    myData->cond = &cond;
    myData->mutex = &mutex;
    myData->phaseDeJeu = phaseDeJeu;
    myData->timer = timer;
    myData->nbSession = nbSession;
    myData->nbMinute = nbMinute;
    myData->dico = dico;

    printf("Coucou c'est moi le main\n");
    pthread_create(&pidB, NULL, boggle, myData);

    pthread_t pidAC;
    myData->sock = mySocket;
    myData->addr = serv;

    pthread_create(&pidAC, NULL, accepteClient, myData);

    pthread_join(pidB, NULL);

    return 0;
}
