typedef struct data data;
struct data
{
    int sock;
    struct sockaddr_in addr;

    char* grille;
};

void* accepteClient(void* arg);
