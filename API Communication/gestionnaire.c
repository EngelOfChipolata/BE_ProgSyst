#include "gestionnaire.h"
#include "varglobal.h"
#include "structures.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define DEBUGGEST
#define DEBUGECRITURE
#define DEBUGRECV



void* Ecriture(void * arg){  /*Thread lancé pour l'écriture d'un message*/
    #ifdef DEBUGECRITURE
    printf("[Ecriture] Thread ecriture lancé \n");
    #endif

    argThreadEcriture * my_args = arg; /*On récupère les arguments pour pouvoir libérer la structure dans le tas*/
    BaL * boite = my_args->boitealettres; /*Parmis eux on récupère la boite à lettre a renseigner*/
    char message[TAILLEMAX];  /*Ainsi que le message à écrire*/
    strcpy(message, my_args->message);

    free(my_args);  /*On libère la zone utilisée pour passer les arguments*/
    my_args = NULL;

    #ifdef DEBUGECRITURE
    printf("[Ecriture] Arguments libérés\n");
    #endif

    pthread_mutex_lock(&(boite->mutex_bal));  /*On vérouille le mutex de la boite à lettre à écrire*/
    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture a le mutex d'une boite à lettre\n");
    #endif

    while (boite->nb_msg >= TAILLEBAL){ /*Si la boite à lettre est pleine on attend*/
        pthread_cond_wait(&(boite->var_cond_bal_full), &(boite->mutex_bal));
    }

    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture va écrire le message : %s\n", message);
    #endif // DEBUGECRITURE

    strcpy(boite->msgs[boite->iecriture], message); /*On écrit le message*/

    (boite->iecriture)++; /*On incrémente l'indice d'écriture*/

    (boite->nb_msg)++; /*On incrémente le nombre de message dans la boite à lettre*/

    pthread_cond_signal(&(boite->var_cond_bal_empty)); /*La boite étant maintenant forcément non-vide on réveille les tâches qui attendaient cette condition*/
    pthread_mutex_unlock(&(boite->mutex_bal)); /*On libère le mutex de la boite à lettre*/

    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture a envoyé le signal, libéré le mutex et va s'éteindre\n");
    #endif

    pthread_exit(NULL); /*On fini le thread*/
}

int findid(Annuaire* annuaire,int tailleannuaire, int id){ /*Fonction qui parcourt l'annuaire et qui renvoie l'index de l'id demandé ou -1 si non trouvé*/
    int i;
    for (i =0; i<tailleannuaire; i++){
        if (annuaire[i].id == id){
            return i;
        }
    }
    return -1;
}

void writerepcode(repZone* zone_reponse, int errno){ /*Fonction qui écrit errno dans la zone réponse passé en argument (en gérant le mutex et la var. cond.)*/
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
    int nbthreadabonne = 0; /*Le nombre de thread abonné*/
    repZone * zone_reponse; /*Pour récupérer les zone_réponses de chaque requête*/

    pthread_t idthreadlance; /*Pour récupérer l'id des threads lancés*/

    int indexdest = 0;
    int indexsource = 0;
    argThreadEcriture * argecriture; /*Pour passer des arguments aux threads d'écriture*/

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

                if(findid(annuaire,*nbthreadmax, _zoneRequete.userid1) != -1){ /*Si on trouve l'id dans l'annuaire alors cet id est pris donc on écrit une erreur*/
                    writerepcode(zone_reponse, -2);
                    break;
                }

                for(i=0; i<*nbthreadmax; i++){  /*On parcourt l'annuaire pour trouver une place libre (qui existe car on a testé avant si l'annuaire était plein)*/
                    if (annuaire[i].id == 0){
                        if((annuaire[i].bal = calloc(1, sizeof(BaL)))==NULL){ /*Si on n'arrive pas a créer la boite à lettre on écrit une erreur*/
                            writerepcode(zone_reponse, -4);
                        }
                        annuaire[i].id = _zoneRequete.userid1; /*On écrit les informations dans l'annuaire*/
                        annuaire[i].idThread = _zoneRequete.id_thread;
                        #ifdef DEBUGGEST
                        printf("J'ai un abo réussi %d\n", _zoneRequete.userid1);
                        #endif // DEBUGGEST
                        nbthreadabonne++; /*On incrémente le nombre d'abonnés*/
                        writerepcode(zone_reponse, 0); /*Tout s'est bien passé on écrit la réponse 0*/
                        break; /*Nul besoin de parcourir le reste de l'annuaire*/
                    }
                }break;

            case 2: /*envoi de message*/
                #ifdef DEBUGECRITURE
                printf("Le gestionnaire va traiter la requête d'envoi\n");
                #endif

                indexsource = findid(annuaire, *nbthreadmax, _zoneRequete.userid2); /*On cherche l'identifiant de la source dans l'annuaire*/

                if (indexsource == -1){ /*Si il n'y est pas on écrit une erreur*/
                    writerepcode(zone_reponse, -2);
                    break;
                }
                if (annuaire[indexsource].idThread != _zoneRequete.id_thread){ /*Si le thread source ne correspond pas au thread abonné avec cet id on écrit une erreur*/
                    writerepcode(zone_reponse, -3);
                    break;
                }

                if (_zoneRequete.userid1 == 0){ /*Si on veut envoyer en broadcast*/
                    for (i=0; i < *nbthreadmax; i++){ /*On parcourt tout l'annuaire*/
                        if (annuaire[i].id != 0 && annuaire[i].id != annuaire[indexsource].id){ /*Si c'est une entrée abonnée et que ce n'est pas celle de l'emmeteur*/
                            if ((argecriture = calloc(1, sizeof(argThreadEcriture)))==NULL){ /*On alloue une zone pour passer les arguments au thread*/
                                writerepcode(zone_reponse, -5); /*Si on y arrive pas on écrit une erreur*/
                            }
                            argecriture->boitealettres = annuaire[i].bal; /*On prépare les arguments a passer*/
                            strcpy(argecriture->message, _zoneRequete.msg);
                            #ifdef DEBUGECRITURE
                            printf("Le gestionnaire a écrit les arguments et va lancé le thread d'écriture\n");
                            #endif // DEBUGECRITURE
                            if (pthread_create(&idthreadlance, NULL, Ecriture, argecriture) != 0){/*Création du thread d'écriture*/
                                free(argecriture); /*Si il y a eu un problème lors de la création du thread on libère la zone des arguments et on écrit un erreur*/
                                argecriture = NULL;
                                writerepcode(zone_reponse, -6);
                            }
                        }
                    }break;
                }

                indexdest = findid(annuaire, *nbthreadmax, _zoneRequete.userid1); /*On cherche l'identifiant destinataire dans l'annuaire*/
                if (indexdest == -1){ /*Si il n'y est pas on renvoie une erreur*/
                    writerepcode(zone_reponse, -4);
                    break;
                }


                if ((argecriture = calloc(1, sizeof(argThreadEcriture)))==NULL){ /*On alloue une zone pour passer les arguments au thread*/
                    writerepcode(zone_reponse, -5); /*Si on y arrive pas on écrit une erreur*/
                    break;
                }
                argecriture->boitealettres = annuaire[indexdest].bal; /*On prépare les arguments a passer*/
                strcpy(argecriture->message, _zoneRequete.msg);
                #ifdef DEBUGECRITURE
                printf("Le gestionnaire a écrit les arguments et va lancé le thread d'écriture\n");
                #endif // DEBUGECRITURE
                if (pthread_create(&idthreadlance, NULL, Ecriture, argecriture) != 0){/*Création du thread d'écriture*/
                    free(argecriture); /*Si il y a eu un problème lors de la création du thread on libère la zone des arguments et on écrit un erreur*/
                    argecriture = NULL;
                    writerepcode(zone_reponse, -6);
                    break;
                }

                #ifdef DEBUGECRITURE
                printf("[Gestionnaire] Thread écriture lancé, répondu à : %d\n",zone_reponse);
                #endif

                argecriture = NULL; /*Il faut etre sur de ne pas y refaire référence*/
                writerepcode(zone_reponse, 0); /*tout s'est bien passé on écrit 0*/

                break;

            case 3:
                break;
        }

        _zoneRequete.flag_req = 0; /*On libère le mutex de la requête et on envoie un signal réveillant les thread en attente d'écriture d'une requête*/
        pthread_cond_signal(&(_zoneRequete.var_cond_req_full));
        pthread_mutex_unlock(&(_zoneRequete.mutexreq));
        #ifdef DEBUGGEST
        printf("[Gestionnaire]mutex requête libéré\n");
        #endif
    }

}
