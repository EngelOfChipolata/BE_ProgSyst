#include "gestionnaire.h"
#include "varglobal.h"
#include "structures.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define DEBUGGEST


int findid(Annuaire* annuaire,int tailleannuaire, int id){ /*Fonction qui parcourt l'annuaire et qui renvoie l'index de l'id demandé ou -1 si non trouvé*/
    int i;
    for (i =0; i<tailleannuaire; i++){
        if (annuaire[i].id == id){
            return i;
        }
    }
    return -1;
}

void writerepcode(repZone* zone_reponse, int errno){
    pthread_mutex_lock(&(zone_reponse->mutexrep));
    zone_reponse->code_err = errno;
    zone_reponse->flag_rep = 1;
    pthread_cond_signal(&(zone_reponse->var_cond_rep));
    pthread_mutex_unlock(&(zone_reponse->mutexrep));

}


void* Gestionnaire (void *arg){
    int * nbthreadmax = arg;
    int i; /*itérateur des boucles for*/
    Annuaire annuaire[(int)*nbthreadmax]; /*déclaration de l'annuaire*/
    int nbthreadabonne = 0;
    repZone * zone_reponse;

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

        while(_zoneRequete.flag_req==0){
            #ifdef DEBUGGEST
            printf("Le gestionnaire attend\n");
            #endif // DEBUGGEST
            pthread_cond_wait(&(_zoneRequete.var_cond_req_empty), &(_zoneRequete.mutexreq)); /*si il n'y a pas de requête on attend en libérant le mutex*/
        }

        #ifdef DEBUGGEST
        printf("Le gestionnaire est réveillé et va traiter la requête\nLa requête est : %d\n", _zoneRequete.numrequest);
        #endif // DEBUGGEST
        zone_reponse = _zoneRequete.repzoneaddr; /*On lit et on stocke la zone dans laquelle on doit répondre a cette requête*/
        switch(_zoneRequete.numrequest){
            case 1: /*abonnement*/
                #ifdef DEBUGGEST
                printf("Le gestionnaire va traiter la requête abo\n");
                #endif // DEBUGGEST
                if (nbthreadabonne >=*nbthreadmax){ /*Si le nombre maximal est atteint on écrit une erreur dans la zone réponse*/
                    #ifdef DEBUGGEST
                    printf("Plus de place dans l'annuaire\n");
                    #endif // DEBUGGEST
                    writerepcode(zone_reponse, -3);
                    break;
                }

                if(findid(annuaire,*nbthreadmax, _zoneRequete.userid1) != -1){
                    writerepcode(zone_reponse, -2);
                    break;
                }

                for(i=0; i<*nbthreadmax; i++){
                    if (annuaire[i].id == 0){
                        if((annuaire[i].bal = malloc(sizeof(BaL)))==NULL){
                            writerepcode(zone_reponse, -4);
                        }
                        annuaire[i].id = _zoneRequete.userid1;
                        annuaire[i].idThread = _zoneRequete.id_thread;
                        #ifdef DEBUGGEST
                        printf("J'ai un abo réussi %d\n", _zoneRequete.userid1);
                        #endif // DEBUGGEST
                        nbthreadabonne++;
                        writerepcode(zone_reponse, 0);
                        break;
                    }
                }break;

            case 2: /*envoi de message*/
                int indexdest;
                int indexsource;
                argThreadEcriture * argecriture;
                #ifdef DEBUGGEST
                printf("Le gestionnaire va traiter la requête d'envoi\n");
                #endif

                indexsource = findid(annuaire, *nbthreadmax, _zoneRequete.userid2);

                if (indexsource == -1){
                    writerepcode(zone_reponse, -2);
                    break;
                }
                if (annuaire[indexsource].idThread != _zoneRequete.id_thread){
                    writerepcode(zone_reponse, -3);
                    break;
                }

                indexdest = findid(annuaire, *nbthreadmax, _zoneRequete.userid1);
                if (indexdest == -1){
                    writerepcode(zone_reponse, -4);
                    break;
                }


                argecriture = calloc(1, sizeof(argThreadEcriture));
                *argecriture.boitealettres = annuaire[indexdest].bal;
                strcpy(*argecriture.message, _zoneRequete.msg);

                if (pthread_create(NULL, NULL, Ecriture, argecriture) != 0){ 	/*Création du thread d'écriture*/
                    writerepcode(zone_reponse, -5);
                    break;
                }


                break;

                case 3:
                break;
            }

        }
        _zoneRequete.flag_req = 0;
        pthread_cond_signal(&(_zoneRequete.var_cond_req_full));
        pthread_mutex_unlock(&(_zoneRequete.mutexreq));

    }

}
