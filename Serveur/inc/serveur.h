typedef struct data data;
struct data
{
    int sock;
    struct sockaddr_in addr;
    char* grille;
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
};

typedef struct dataB dataB;
struct dataB
{
    int nbJoueur;
    char* grille;

    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
};

void* accepteClient(void* arg);
