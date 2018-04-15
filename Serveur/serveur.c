#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

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

    int nbClient = 0;
    int socketClient[5];
    int size[5];
    struct sockaddr_in clients[5];

    while(1)
    {
        size[nbClient] = sizeof(clients[nbClient]);
        socketClient[nbClient] = accept(mySocket, (struct sockaddr*) &clients[nbClient], &size[nbClient]);

        if(socketClient[nbClient] == -1)
        {
            perror("accept()");
            continue;
        }

        nbClient++;
    }

    return 0;
}
