#define _REENTRANT

#include "gestionnaire.h"
#include "varglobal.h"
#include "structures.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#define DEBUGGEST


void* Gestionnaire (void *arg){
    int * nbthreadmax = arg;
    int i; /*itérateur des boucles for*/
    Annuaire annuaire[(int)*nbthreadmax]; /*déclaration de l'annuaire*/
    int nbthreadabonne = 0;
    for (i=0; i < *nbthreadmax; i++){ /*Initialisation de l'annuaire*/
        annuaire[i].id = 0;
    }

    #ifdef DEBUGGEST
    printf("Thread gest init OK, nombre max :%d\n", *nbthreadmax);
    #endif // DEBUGGEST

    while(1){
        pthread_mutex_lock(&(_zoneRequete.mutexreq)); /*On prend le mutex de la zone de requête*/
        #ifdef DEBUGGEST
        printf("le gestionnaire à le mutex de la requête\n");
        #endif // DEBUGGEST

        if(_zoneRequete.flag_req==0){
            #ifdef DEBUGGEST
            printf("Le gestionnaire attend\n");
            #endif // DEBUGGEST
            pthread_cond_wait(&(_zoneRequete.var_cond_req_empty), &(_zoneRequete.mutexreq)); /*si il n'y a pas de requête on attend en libérant le mutex*/
        }

        #ifdef DEBUGGEST
        printf("Le gestionnaire est réveillé et va traiter la requête\nLa requête est : %d\n", _zoneRequete.numrequest);
        #endif // DEBUGGEST
        switch(_zoneRequete.numrequest){
            case 1: /*abonnement*/
                #ifdef DEBUGGEST
                printf("Le gestionnaire va traiter la requête abo\n");
                #endif // DEBUGGEST
                if (nbthreadabonne == *nbthreadmax){
                    #ifdef DEBUGGEST
                    printf("Plus de place dans l'annuaire\n");
                    #endif // DEBUGGEST
                    /*TODO erreur aucune place libre */
                    break;
                }

                for(i=0; i<*nbthreadmax; i++){
                    if (annuaire[i].id == 0){
                        if((annuaire[i].bal = malloc(sizeof(BaL)))==NULL){
                            /*TODO écrire erreur d'allocation de Bal in rep zone*/
                        }
                        annuaire[i].id = _zoneRequete.userid1;
                        annuaire[i].idThread = _zoneRequete.id_thread;
                        #ifdef DEBUGGEST
                        printf("J'ai un abo réussi %d\n", _zoneRequete.userid1);
                        #endif // DEBUGGEST
                        /*TODO stockage de la repzone dans l'annuaire*/
                        break;
                    }
                }break;

        }
        _zoneRequete.flag_req = 0;
        pthread_cond_signal(&(_zoneRequete.var_cond_req_full));
        pthread_mutex_unlock(&(_zoneRequete.mutexreq));

    }

}
