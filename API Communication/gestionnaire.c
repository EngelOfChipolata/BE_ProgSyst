#define _REENTRANT

#include "gestionnaire.h"
#include "varglobal.h"
#include "structures.h"
#include "pthread.h"

void* Gestionnaire (void *nbthreadmax){
    Annuaire annuaire[*nbthreadmax]; /*déclaration de l'annuaire*/
    for (i=0; i < *nbthreadmax; i++){ /*Initialisation de l'annuaire*/
        annuaire[i].id = 0;
    }

    while(1){
        pthread_mutex_lock(&(_zoneRequete.mutexreq));


    }

}
