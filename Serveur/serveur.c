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

typedef struct data
{
    int mySocket;
    struct sockaddr_in addrClient;
}data;

void *threadFunction(void *arg)
{
    data *myData = arg;
    ssize_t nbLu;
    char buffer[256];

    while((nbLu = recv(myData->mySocket, buffer, 256, 0)) > 0)
    {
        int port = myData->addrClient.sin_port;
        char *addr = inet_ntoa(myData->addrClient.sin_addr);
        printf("%s\n", addr);
        printf("%d\n", port);
        printf("%s\n", buffer);
    }
    return(0);
}

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        perror("Pas assez d'arguments");
        return 0;
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

    socklen_t addrSize;

    while(1)
    {
        data *myData = malloc(sizeof(data));
        addrSize = sizeof(myData->addrClient);
        myData->mySocket = accept(mySocket, (struct sockaddr*)&(myData->addrClient), &addrSize);
        pthread_t pidT;
        pthread_create(&pidT, NULL, threadFunction, myData);
    }

    return 0;
}
