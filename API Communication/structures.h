#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED
#define TAILLEMAX 60
#define TAILLEBAL 10

#include "pthread.h"
typedef struct requestZone;
typedef struct repZone;
typedef struct BaL;
typedef struct Annuaire;
typedef struct idThreadGest;


struct requestZone{
    int numrequest;
    int userid1;
    int userid2;
    answerZone * repzoneaddr;
    pthread_t id_thread;
    char * msg[TAILLEMAX];
    int flag_req;
    pthread_mutex_t mutexreq;
    pthread_cond_t var_cond_req_full;
    pthread_cond_t var_cond_req_empty;
};


struct repZone{
    int flag_rep;
    int code_err;
    union{
        char * msg[TAILLEMAX];
        int nb_msg;
    };
    pthread_mutex_t mutexrep;
    pthread_cond_t var_cond_rep_full;
    pthread_cond_t var_cond_rep_empty;
};


struct BaL{
    char ** msgs[TAILLEBAL][TAILLEMAX];
    int iecriture;
    int ilecture;
    int nb_msg;
    pthread_mutex_t mutex_bal;
    pthread_cond_t var_cond_bal_full;
    pthread_cond_t var_cond_bal_empty;
};


struct Annuaire{
    int id;
    pthread_t idThread;
    BaL * bal;
    repZone * rep;
};

struct idThreadGest{
    pthread_mutex_t mutexgest;
    pthread_t idgest;
};



#endif // STRUCTURES_H_INCLUDED
