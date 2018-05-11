typedef struct message message;
struct message
{
    char* source;
    char* destinataire;
    char* contenu;

    message* suivant;
};

typedef struct fileMessage fileMessage;
struct fileMessage
{
    message* premier;
};

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

    fileMessage* file;
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
    pthread_cond_t* condTemps;

    int* timer;
    int* phaseDeJeu;
    int* sizeMot;

    FILE* dico;

    fileMessage* file;
};


void enfiler(fileMessage* file, char* source, char* destinataire, char* contenu);
message* defiler(fileMessage* file);
void enleveAccent(FILE* dico);
int rechercheDansDico(char* mot, FILE* dico);
int recherchePseudo(dataClient** joueurs, int nbJoueur, char* pseudo);
void extractPseudo(char* tab);
int dejaProposer(char* mot, char* listeMot, int size);
void ajouterMot(char* mot, char* listeMot, int* sizeMot);
int tailleMot(char* mot);
int valideTrajectoire(char* mot, char* trajectoire);
void valideMot(int sock, char* mot, char* listeMot, int* sizeMot, char* grille, FILE* dico);
void messageBroadcast(message* msg, dataClient** joueurs, int nbJoueur);
void messagePrive(message* msg, int sock);
void* boggle(void *arg);
void* traiteClient(void *arg);
void* accepteClient(void* arg);
