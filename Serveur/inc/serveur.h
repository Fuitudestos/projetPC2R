typedef struct data data;
struct data
{
    int sock;
    int nbJoueur;
    int nbJoueurMax;
    int nbSession;
    int nbMinute;
    data** joueurs;
    struct sockaddr_in addr;
    char* grille;
    pthread_cond_t* cond;
    pthread_mutex_t* mutex;

    int* timer;
    int* phaseDeJeu;

    FILE* dico;
};

void* accepteClient(void* arg);
