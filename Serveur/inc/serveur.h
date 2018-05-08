typedef struct dataClient dataClient;
struct dataClient
{
    int sock;
    int nbSession;
    int valide;
    struct sockaddr_in addr;

    int* timer;
    int* phaseDeJeu;
    int* sizeMot;

    char* grille;
    char* pseudo;
    char* motProposer;

    pthread_cond_t* condServ;
    pthread_cond_t* condClient;
    pthread_mutex_t* mutexServ;
    pthread_mutex_t* mutexClient;

    FILE* dico;
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
    char* motProposer;
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;

    int* timer;
    int* phaseDeJeu;
    int* sizeMot;

    FILE* dico;
};

void* accepteClient(void* arg);
