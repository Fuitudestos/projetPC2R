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

void enfiler(fileMessage* file, char* source, char* destinataire, char* contenu)
{
    message* nouveau = malloc(sizeof(message));

    nouveau->source = malloc(sizeof(char) * TAILLEBUFFER);
    nouveau->contenu = malloc(sizeof(char) * TAILLEBUFFER);

    memcpy(nouveau->source, source, TAILLEBUFFER);
    memcpy(nouveau->contenu, contenu, TAILLEBUFFER);

    if(destinataire != NULL)
    {
        nouveau->destinataire = malloc(sizeof(char) * TAILLEBUFFER);
        memcpy(nouveau->destinataire, destinataire, TAILLEBUFFER);
    }

    nouveau->suivant = NULL;

    if(file->premier != NULL)
    {
        message* tmp = file->premier;

        while(tmp->suivant != NULL) tmp = tmp->suivant;
        tmp->suivant = nouveau;
    }
    else file->premier = nouveau;
}

message* defiler(fileMessage* file)
{
    message* res = NULL;

    if(file->premier != NULL)
    {
        res = file->premier;
        file->premier = res->suivant;
    }

    return res;
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

                if(strcmp(monAccent, "ü") == 0 || strcmp(monAccent, "û") == 0 || strcmp(monAccent, "ù") == 0){fputc('U', newDico);}
                else if(strcmp(monAccent, "é") == 0 || strcmp(monAccent, "ê") == 0 || strcmp(monAccent, "ë") == 0 || strcmp(monAccent, "è") == 0){fputc('E', newDico);}
                else if(strcmp(monAccent, "ï") == 0 || strcmp(monAccent, "î") == 0){fputc('I', newDico);}
                else if(strcmp(monAccent, "ç") == 0){fputc('C', newDico);}
                else if(strcmp(monAccent, "ô") == 0 || strcmp(monAccent, "ö") == 0){fputc('O', newDico);}
                else if(strcmp(monAccent, "â") == 0 || strcmp(monAccent, "ä") == 0 || strcmp(monAccent, "à") == 0){fputc('A', newDico);}
            }
            else
            {
                fputc(toupper(tmp), newDico);
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
        index = 0;

        while(mot[index] == tmp)
        {
            tmp = fgetc(dico);
            if(tmp == '\n' && mot[index + 1] == '/')
            {
                rewind(dico);
                return 1;
            }
            else if(tmp == '\n' || mot[index + 1] == '/') break;
            index++;
        }

        while(tmp != '\n')
        {
            tmp = fgetc(dico);
            if(feof(dico) != 0) return 0;
        }
    }

    rewind(dico);
    return 0;
}

int recherchePseudo(dataClient** joueurs, int nbJoueur, char* pseudo)
{
    int i;

    for(i = 0; i < nbJoueur; i++)
    {
        if(strcmp(pseudo, joueurs[i]->pseudo) == 0)
        {
            return i;
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
            break;
        }
    }

    for(i = 0; i < TAILLEBUFFER; i++)
    {
        if(tab[i] == 0)break;
        if(i > fin-debut)tab[i] = 0;
        else tab[i] = tab[debut + i];
    }
}

char* extractDestinataire(char* tab)
{
    char* res = malloc(sizeof(char) * TAILLEBUFFER);
    memset(res, 0, TAILLEBUFFER);

    int i;
    int debut = 257;
    int fin = 0;

    for(i = 256; i > 0; i--)
    {
        if(fin == 0 && tab[i] == '/')
        {
            fin = --i;
        }
        else if(debut == 257 && tab[i] == '/')
        {
            debut = i + 1;
            break;
        }
    }

    int index = 0;

    for(i = debut; i < fin; i++)
    {
        res[index] = tab[i];
        index++;
    }

    return res;
}

int dejaProposer(char* mot, char* listeMot, int size)
{
    int i;
    int index = 0;

    for(i = 0; i < size; i++)
    {
        if(mot[index] == '/' && listeMot[i] == '/') return 1;
        else if(mot[index] == '/' || listeMot[i] == '/') index = 0;
        else if(mot[index] == listeMot[i]) index++;
    }

    return 0;
}

void ajouterMot(char* mot, char* listeMot, int* sizeMot)
{
    int i;
    int size = *sizeMot;
    int index = 0;

    for(i = 0; i < size; i++)
    {
        if(mot[index] == '/') break;
        if(listeMot[i] == 0)
        {
            listeMot[i] = mot[index];
            index++;
        }
    }

    if(mot[index] != '/')
    {
        *sizeMot = *sizeMot * 2;
        char* tmp = malloc(sizeof(char) * size * 2);
        memset(tmp, 0, size * 2);
        memcpy(tmp, listeMot, size);

        for(i = size; i < size * 2; i++)
        {
            if(mot[index] == '/') break;
            tmp[i] = mot[index];
            index++;
        }

        listeMot = tmp;
    }

    listeMot[i] = '/';
}

int tailleMot(char* mot)
{
    int i = 0;

    while(mot[i] != '/' && mot[i] != 0) i++;

    return i;
}

int valideTrajectoire(char* mot, char* grille)
{
    return 1;
}

void valideMot(int sock, char* mot, char* listeMot, int* sizeMot, char* grille, FILE* dico)
{
    int i = 0;
    int score;

    while(mot[i] != '/') i++;

    i++;

    int size = tailleMot(&mot[i]);

    if(size >= 3)
    {
        if(rechercheDansDico(&mot[i], dico) == 1)
        {
            if(dejaProposer(&mot[i], listeMot, *sizeMot) == 0)
            {
                if(valideTrajectoire(&mot[i], grille) == 1)
                {
                    switch(size)
                    {
                        case 3: score = 1;break;
                        case 4: score = 1;break;
                        case 5: score = 2;break;
                        case 6: score = 3;break;
                        case 7: score = 5;break;
                        default: score = 11;break;
                    }
                    ajouterMot(&mot[i], listeMot, sizeMot);
                    extractPseudo(mot);
                    char* buffer = malloc(sizeof(char) * TAILLEBUFFER);
                    memset(buffer, 0, TAILLEBUFFER);
                    sprintf(buffer, "MVALIDE/%s/%d/\n", mot, score);
                    write(sock, buffer, sizeof(char) * 11 + size + sizeof(score));
                }
                else write(sock, "MINVALIDE/TRAJECTOIRE/\n", sizeof(char) * 23);
            }
            else write(sock, "MINVALIDE/PRI/\n", sizeof(char) * 15);
        }
        else write(sock, "MINVALIDE/DICTIONNAIRE/\n", sizeof(char) * 24);
    }
    else write(sock, "MINVALIDE/LONGUEUR/\n", sizeof(char) * 20);
}

void messageBroadcast(message* msg, dataClient** joueurs, int nbJoueur)
{
    int i;
    char* buffer = malloc(sizeof(char) * TAILLEBUFFER);
    memset(buffer, 0, TAILLEBUFFER);
    sprintf(buffer, "RECEPTION/%s/%s/\n", msg->contenu, msg->source);
    //printf("contenu : %s, Source : %s\n", msg->contenu, msg->source);

    for(i = 0; i < nbJoueur; i++)
    {
        write(joueurs[i]->sock, buffer, 15 + tailleMot(msg->contenu) + tailleMot(msg->source));
    }

    free(msg->source);
    free(msg->contenu);
    free(msg);
}

void messagePrive(message* msg, int sock)
{
    char* buffer = malloc(sizeof(char) * TAILLEBUFFER);
    memset(buffer, 0, TAILLEBUFFER);
    sprintf(buffer, "PRECEPTION/%s/%s/\n", msg->contenu, msg->source);

    write(sock, buffer, 15 + tailleMot(msg->contenu) + tailleMot(msg->source));

    free(msg->source);
    free(msg->destinataire);
    free(msg->contenu);
    free(msg);
}

void* boggle(void *arg)
{
    printf("Coucou c'est moi le jeu\n");
    dataServ* myData = (dataServ*) arg;
    int timer;
    int nbSession = 0;
    int nbJoueur = 0;
    int index;
    pthread_mutex_lock(myData->mutex);
    *myData->phaseDeJeu = 0;
    myData->grille = tirageGrille(myData->grille);
    printf("tirageGrille\n");
    pthread_cond_wait(myData->cond, myData->mutex);
    pthread_mutex_unlock(myData->mutex);

    message* msg;

    while(nbSession < myData->nbSession)
    {
        for(timer = myData->nbMinute * 60; timer >= 0; timer--)
        {
            while(myData->nbJoueur > nbJoueur)
            {
                if(recherchePseudo(myData->joueurs, nbJoueur, myData->joueurs[nbJoueur]->pseudo) == 0)
                {
                    printf("C'est Bon : %s\n", myData->joueurs[nbJoueur]->pseudo);
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

            msg = defiler(myData->file);
            while(msg != NULL)
            {
                if(msg->destinataire == NULL) messageBroadcast(msg, myData->joueurs, myData->nbJoueur);
                else
                {
                    index = recherchePseudo(myData->joueurs, myData->nbJoueur, msg->destinataire);
                    messagePrive(msg, myData->joueurs[index]->sock);
                }

                msg = defiler(myData->file);
            }

            printf("RC : %s\n", myData->joueurs[0]->pseudo);

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

    memset(myData->pseudo, 0, TAILLEBUFFER);
    memset(buffer, 0, TAILLEBUFFER);
    read(myData->sock, buffer, TAILLEBUFFER);

    extractPseudo(buffer);
    memcpy(myData->pseudo, buffer, TAILLEBUFFER);

    pthread_mutex_lock(myData->mutexServ);
    pthread_cond_signal(myData->condServ);
    pthread_cond_wait(myData->condClient, myData->mutexServ);
    pthread_mutex_unlock(myData->mutexServ);

    while(myData->valide == 0)
    {
        write(myData->sock, "CONNEXION/BADPSEUDO/\n", sizeof(char) * 21);
        memset(buffer, 0, TAILLEBUFFER);
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
        write(myData->sock, "TOUR/tirage/", sizeof(char) * 12);
        for(i = 0; i < 16; i++)
        {
            write(myData->sock, &myData->grille[i], sizeof(char));
        }
        write(myData->sock, "/\n", sizeof(char) * 2);
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
            sprintf(buffer, "TOUR/newTime/%d : %d/\n", nbMinute, nbSeconde);
            write(myData->sock, buffer, sizeof(char) * 25);

            buffer = memset(buffer, 0, TAILLEBUFFER);
            read(myData->sock, buffer, TAILLEBUFFER);

            if(buffer[0] == 'T')
            {
                valideMot(myData->sock, buffer, myData->motProposer, myData->sizeMot, myData->grille, myData->dico);
            }

            if(buffer[0] == 'E')
            {
                char* source = malloc(sizeof(char) * TAILLEBUFFER);
                memset(source, 0, TAILLEBUFFER);
                extractPseudo(buffer);
                memcpy(source, buffer, TAILLEBUFFER);
                printf("Pseudo : %s, Message : %s\n", myData->pseudo, buffer);
                enfiler(myData->file, myData->pseudo, NULL, buffer);
            }

            if(buffer[0] == 'P')
            {
                char* destinataire = extractDestinataire(buffer);
                char* source = malloc(sizeof(char) * TAILLEBUFFER);
                memset(source, 0, TAILLEBUFFER);
                extractPseudo(buffer);
                memcpy(source, buffer, TAILLEBUFFER);
                enfiler(myData->file, myData->pseudo, destinataire, buffer);
            }
        }

        while(*myData->phaseDeJeu == 0)
        {
            buffer = memset(buffer, 0, TAILLEBUFFER);
            read(myData->sock, buffer, sizeof(buffer));

            if(buffer[0] == 'E')
            {
                char* source = malloc(sizeof(char) * TAILLEBUFFER);
                memset(source, 0, TAILLEBUFFER);
                extractPseudo(buffer);
                memcpy(source, buffer, TAILLEBUFFER);
                printf("Pseudo : %s, Message : %s\n", myData->pseudo, buffer);
                enfiler(myData->file, myData->pseudo, NULL, buffer);
            }

            if(buffer[0] == 'P')
            {
                char* destinataire = extractDestinataire(buffer);
                char* source = malloc(sizeof(char) * TAILLEBUFFER);
                memset(source, 0, TAILLEBUFFER);
                extractPseudo(buffer);
                memcpy(source, buffer, TAILLEBUFFER);
                enfiler(myData->file, myData->pseudo, destinataire, buffer);
            }
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
        clientData->motProposer = serv->motProposer;
        clientData->sizeMot = serv->sizeMot;
        clientData->dico = serv->dico;
        clientData->file = serv->file;

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
    int* size = malloc(sizeof(int));
    int* phaseDeJeu = malloc(sizeof(int));
    int* timer = malloc(sizeof(int));
    char* grille = malloc(sizeof(char) * 16);
    char* motProposer = malloc(sizeof(char) * TAILLEBUFFER);
    fileMessage* file = malloc(sizeof(fileMessage));
    file->premier = NULL;
    memset(motProposer, 0, TAILLEBUFFER);

    dataClient** joueurs = malloc(sizeof(dataClient*) * 10);

    *size = TAILLEBUFFER;

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
    myData->motProposer = motProposer;
    myData->sizeMot = size;
    myData->file = file;

    printf("Coucou c'est moi le main\n");
    pthread_create(&pidB, NULL, boggle, myData);

    pthread_t pidAC;
    myData->sock = mySocket;
    myData->addr = serv;

    pthread_create(&pidAC, NULL, accepteClient, myData);

    pthread_join(pidB, NULL);

    return 0;
}
