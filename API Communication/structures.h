#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED
#define TAILLEMAX 60
#define TAILLEBAL 10

#include "pthread.h"


typedef struct repzone{
    int flag_rep;
    int code_err;
    union{
        char msg[TAILLEMAX];
        int nb_msg;
    };
    pthread_mutex_t mutexrep;
    pthread_cond_t var_cond_rep;
} repZone;
/**
Structure utilisée par le thread gestionnaire pour passer les réponses,
le thread gestionnaire écrit et les fonctions invoqués par l'utilisateur lisent
*/

typedef struct requestzone{
    int numrequest; //1 : abonnement, 2 : écriture, 3: lecture, 4: consultation, 5: desabonnement, 6: findouce
    int userid1;
    int userid2;
    repZone * repzoneaddr;
    pthread_t id_thread;
    char msg[TAILLEMAX];
    int flag_req; //0: pas de requêtes (on peut écrire), 1: requête en attente (on peut lire)
    pthread_mutex_t mutexreq;
    pthread_cond_t var_cond_req_full;
    pthread_cond_t var_cond_req_empty;
}requestZone;

/**
La structure instanciée une seule fois permet au fonctions invoquées par l'utilisateur d'écrire au
thread gestionnaire,
les fonctions écrivent et le thread gestionnaire lit*/


typedef struct bal{
    char msgs[TAILLEBAL][TAILLEMAX];
    int iecriture;
    int ilecture;
    int nb_msg;
    pthread_mutex_t mutex_bal;
    pthread_cond_t var_cond_bal_full;
    pthread_cond_t var_cond_bal_empty;
}BaL;
/**
La structure servant à stocker les messages de façon FIFO implémantée en buffer tournant
*/


typedef struct annuaire{
    int id;
    pthread_t idThread;
    BaL * bal;
    char * exist;
}Annuaire;
/**
Une entrée de l'annuaire tenu à jour par le thread gestionnaire
*/

typedef struct idthreadgest{
    pthread_mutex_t mutexgest;
    pthread_t idgest;
}idThreadGest;
/**
Permet aux fonctions de savoir si le thread gestionnaire est lancé ou non
*/

typedef struct argsthreadecriture{
    BaL * boitealettres;
    char message[TAILLEMAX];
    char * exist;
} argThreadEcriture;
/**
Permet au thread gestionnaire de renseigner les arguments à passer aux threads d'écriture
*/

typedef struct argsthreadlecture{
    BaL * boitealettres;
    repZone * repzoneaddr;
    int flag;
    char * exist;
} argThreadLecture;
/**
Permet au thread gestionnaire de renseigner les arguments à passer au threads de lecture
*/



#endif // STRUCTURES_H_INCLUDED
