typedef struct dataClient dataClient;
struct dataClient
{
    int sock;
    int nbSession;
    struct sockaddr_in addr;

    int* timer;
    int* phaseDeJeu;

    char* grille;
    char* pseudo;

    pthread_cond_t* cond;
    pthread_mutex_t* mutex;
};

typedef struct dataServ dataServ;
struct dataServ
{
    int sock;
    int nbJoueur;
    int nbJoueurMax;
    int nbSession;
    int nbMinute;
    dataClient** joueurs;
    struct sockaddr_in addr;
    char* grille;
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;

    int* timer;
    int* phaseDeJeu;

    FILE* dico;
};

void* accepteClient(void* arg);
