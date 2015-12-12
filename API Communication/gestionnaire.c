#include "gestionnaire.h"
#include "varglobal.h"
#include "structures.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#define DEBUGGEST
//#define DEBUGECRITURE
//#define DEBUGRECV
//#define DEBUGDESABO


void writerepcode(repZone* zone_reponse, int errno){ /*Fonction qui écrit errno dans la zone réponse passé en argument (en gérant le mutex et la var. cond.)*/
    pthread_mutex_lock(&(zone_reponse->mutexrep));
    zone_reponse->code_err = errno;
    zone_reponse->flag_rep = 1;
    pthread_cond_signal(&(zone_reponse->var_cond_rep));
    pthread_mutex_unlock(&(zone_reponse->mutexrep));

}



void* Ecriture(void * arg){  /*Thread lancé pour l'écriture d'un message*/
    #ifdef DEBUGECRITURE
    printf("[Ecriture] Thread ecriture lancé \n");
    #endif

    argThreadEcriture * my_args = arg; /*On récupère les arguments pour pouvoir libérer la structure dans le tas*/
    BaL * boite = my_args->boitealettres; /*Parmis eux on récupère la boite à lettre a renseigner*/
    char message[TAILLEMAX];  /*Ainsi que le message à écrire*/
    strcpy(message, my_args->message);
    char * exist = my_args->exist; /*Mais aussi le flag d'existence !*/

    free(my_args);  /*On libère la zone utilisée pour passer les arguments*/
    my_args = NULL;

    #ifdef DEBUGECRITURE
    printf("[Ecriture] Arguments libérés\n");
    #endif


    if (*exist){ /*Si la boite à lettre existe*/
    pthread_mutex_lock(&(boite->mutex_bal));  /*On vérouille le mutex de la boite à lettre à écrire*/
    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture a le mutex d'une boite à lettre\n");
    #endif

    while (*exist && boite->nb_msg >= TAILLEBAL){ /*Si la boite à lettre est pleine et qu'elle existe on attend*/
        pthread_cond_wait(&(boite->var_cond_bal_full), &(boite->mutex_bal));
    }
    if (!*exist){ /*Si le réveil à été provoqué car la boite à lettre n'existe plus*/
        #ifdef DEBUGECRITURE
        printf("[Ecriture] La bal n'existe plus je me suicide\n");
        #endif // DEBUGECRITURE
        pthread_mutex_unlock(&(boite->mutex_bal));
        pthread_exit(NULL); /*On se suicide*/
    }

    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture va écrire le message : %s\n", message);
    #endif // DEBUGECRITURE

    strcpy(boite->msgs[boite->iecriture], message); /*On écrit le message*/

    boite->iecriture = (boite->iecriture + 1)%TAILLEBAL ; /*On incrémente l'indice d'écriture*/

    (boite->nb_msg)++; /*On incrémente le nombre de message dans la boite à lettre*/

    pthread_cond_signal(&(boite->var_cond_bal_empty)); /*La boite étant maintenant forcément non-vide on réveille les tâches qui attendaient cette condition*/
    pthread_mutex_unlock(&(boite->mutex_bal)); /*On libère le mutex de la boite à lettre*/

    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture a envoyé le signal, libéré le mutex\n");
    #endif
    }
    #ifdef DEBUGECRITURE
    printf("[Ecriture] Ecriture va s'éteindre\n");
    #endif // DEBUGECRITURE

    pthread_exit(NULL); /*On fini le thread*/
}

void* Lecture_msg(void * arg){  /*Thread lancé pour la lecture d'un message*/
    #ifdef DEBUGRECV
    printf("[Lecture_msg] Thread lecture lancé \n");
    #endif

    argThreadLecture * my_args = arg; /*On récupère les arguments pour pouvoir libérer la structure dans le tas*/
    BaL * boite = my_args->boitealettres; /*Parmis eux on récupère la boite à lettre a renseigner*/
    repZone * zonerep = my_args->repzoneaddr; /*Egalement la zone réponse où écrire*/
    int flag = my_args->flag;
    char * exist = my_args->exist; /*Mais aussi le flag d'existence !*/

    char message[TAILLEMAX];  /*Message à lire que l'on stockera dans cette varible locale*/
    int nb_msg; /*nombre message dans la Bal*/

    free(my_args);  /*On libère la zone utilisée pour passer les arguments*/
    my_args = NULL;

    #ifdef DEBUGRECV
    printf("[Lecture_msg] Arguments libérés, Bal : %d\n", boite);
    #endif

    if(!*exist){
    #ifdef DEBUGRECV
    printf("[Lecture_msg] La boite à lettres n'existe plus\n");
    #endif

    writerepcode(zonerep, -2);
    pthread_exit(NULL);
    }
    pthread_mutex_lock(&(boite->mutex_bal));  /*On vérouille le mutex de la boite à lettre à écrire*/
    #ifdef DEBUGRECV
    printf("[Lecture_msg] Lecture_msg a le mutex d'une boite à lettre\n");
    #endif

    while (*exist && boite->nb_msg == 0 && flag==0){/*Si on demande à lire un message (bloquant) qu'il n'y en a pas on attend*/
        pthread_cond_wait(&(boite->var_cond_bal_empty), &(boite->mutex_bal));
    }


    if (!*exist){
        #ifdef DEBUGRECV
        printf("[Lecture_msg] La bal n'existe plus je me suicide\n");
        #endif // DEBUGRECV

        pthread_mutex_unlock(&(boite->mutex_bal));
        writerepcode(zonerep, -2);
        pthread_exit(NULL); /*Si la boite à lettre n'existe plus on se suicide*/
    }

    if (flag == 0) {
        strcpy(message, boite->msgs[boite->ilecture]);/*On lit le message*/

        #ifdef DEBUGRECV
        printf("[Lecture_msg] Lecture_msg a lu le message : %s\n", message);
        #endif // DEBUGRECV

        boite->ilecture = (boite->ilecture + 1)%TAILLEBAL; /*On incrémente l'indice de lecture*/
        (boite->nb_msg)--; /*On décrémente le nombre de messages*/
    }

    else if (flag == 1) {
        nb_msg = boite->nb_msg;
        #ifdef DEBUGRECV
        printf("[Lecture_msg] Lecture_msg a lu le nombre de messages : %d\n", nb_msg);
        #endif // DEBUGRECV
    }


    pthread_cond_signal(&(boite->var_cond_bal_full)); /*On réveille les Threads d'écriture en attente*/
    pthread_mutex_unlock(&(boite->mutex_bal)); /*On libère le mutex de la boite à lettre*/

    #ifdef DEBUGRECV
    printf("[Lecture_msg] Lecture_msg a libéré la BaL\n");
    #endif // DEBUGRECV

    pthread_mutex_lock(&(zonerep->mutexrep)); /* On vérouille le mutex de la zone réponse */

    #ifdef DEBUGRECV
    printf("[Lecture_msg] Lecture_msg a vérouillé la zone réponse\n");
    #endif // DEBUGRECV

    if (flag == 0) {
        strcpy(zonerep->msg,message);
    }
    else if (flag == 1) {
        zonerep->nb_msg = nb_msg;
    }
    zonerep->code_err = 0;
    zonerep->flag_rep = 1;
    pthread_cond_signal(&(zonerep->var_cond_rep));
    pthread_mutex_unlock(&(zonerep->mutexrep));

    #ifdef DEBUGRECV
    printf("[Lecture_msg] Lecture_msg a libéré la zone réponse");
    #endif // DEBUGRECV

    pthread_exit(NULL);
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

void desaboid(Annuaire* annuaire, int index){
    pthread_mutex_lock(&(annuaire[index].bal->mutex_bal)); /*On s'assure que personne n'utilise la boite à lettre pendant le désabonnement*/
    *(annuaire[index].exist) = 0; /*On passe la flag d'existance à 0*/
    pthread_cond_broadcast(&(annuaire[index].bal->var_cond_bal_empty)); /*On réveille tous les threads lecteurs en attente de cette boite à lettres pour qu'ils testent le flag d'existance et se suicident*/
    pthread_cond_broadcast(&(annuaire[index].bal->var_cond_bal_full)); /*On réveille tous les threads écrivains en attente de cette boite à lettres pour qu'ils testent le flag d'existance et se suicident*/
    pthread_mutex_unlock(&(annuaire[index].bal->mutex_bal)); /*On libère le mutex de la boite à lettre avant de libérer la mémoire*/

    free(annuaire[index].bal);
    annuaire[index].bal = NULL;

    annuaire[index].idThread = 0; /*On efface l'entrée dans l'annuaire*/
    annuaire[index].id = 0;

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
    argThreadLecture * arglecture_msg; /*Pour passer des arguments aux threads de lecture de message*/


    pthread_attr_t attributes; /*Pour passer les attributs aux threads d'écriture et de lecture*/

    pthread_attr_init(&attributes); /*Initialisation des attributs*/
    pthread_attr_setschedpolicy(&attributes, SCHED_FIFO); /*On met la politique d'ordonnancement des futurs threads en FIFO*/

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
        printf("[Gestionnaire] Le gestionnaire est réveillé et va traiter la requête\nLa requête est : %d\n", _zoneRequete.numrequest);
        #endif // DEBUGGEST
        zone_reponse = _zoneRequete.repzoneaddr; /*On lit et on stocke la zone dans laquelle on doit répondre a cette requête*/
        switch(_zoneRequete.numrequest){ /*On traite différemment suivant les requêtes*/
            case 1: /*abonnement*/
                #ifdef DEBUGGEST
                printf("[Gestionnaire] Le gestionnaire va traiter la requête abo\n");
                #endif // DEBUGGEST
                if (nbthreadabonne >=*nbthreadmax){ /*Si le nombre maximal est atteint on écrit une erreur dans la zone réponse*/
                    #ifdef DEBUGGEST
                    printf("[Gestionnaire]Plus de place dans l'annuaire\n");
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
                        if((annuaire[i].bal = calloc(1, sizeof(BaL)))==NULL){ /*Si on n'arrive pas a créer la boite à lettre on écrit une erreur et on sort de la boucle*/
                            writerepcode(zone_reponse, -4);
                            break;
                        }
                        if ((annuaire[i].exist = calloc(1, sizeof(char)))==NULL){ /*Si on arrive pas à créer le flag d'existence*/
                            writerepcode(zone_reponse, -4); /*On écrit une erreur, on libère la boite à lettre et on sort de la boucle*/
                            free(annuaire[i].bal);
                            annuaire[i].bal = NULL;
                            break;
                        }
                        annuaire[i].bal->iecriture = 0; /*On initialise la boite à lettres*/
                        annuaire[i].bal->ilecture = 0;
                        annuaire[i].bal->mutex_bal = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
                        annuaire[i].bal->var_cond_bal_empty = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
                        annuaire[i].bal->var_cond_bal_full = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

                        *(annuaire[i].exist) = 1;
                        annuaire[i].id = _zoneRequete.userid1; /*On écrit les informations dans l'annuaire*/
                        annuaire[i].idThread = _zoneRequete.id_thread;
                        #ifdef DEBUGGEST
                        printf("J'ai un abo réussi %d\n", _zoneRequete.userid1);
                        #endif // DEBUGGEST
                        nbthreadabonne++; /*On incrémente le nombre d'abonnés*/
                        writerepcode(zone_reponse, 0); /*Tout s'est bien passé on écrit la réponse 0*/
                        break; /*Nul besoin de parcourir le reste de l'annuaire*/
                    }
                }break; /*Fin de l'abonnement*/

            case 2: /*envoi de message*/
                #ifdef DEBUGECRITURE
                printf("Le gestionnaire va traiter la requête d'envoi\n");
                #endif

                indexsource = findid(annuaire, *nbthreadmax, _zoneRequete.userid2); /*On cherche l'identifiant de la source dans l'annuaire*/

                if (indexsource == -1){ /*Si il n'y est pas on écrit une erreur*/
                    writerepcode(zone_reponse, -2);
                    break; /*Fin du switch*/
                }
                if (annuaire[indexsource].idThread != _zoneRequete.id_thread){ /*Si le thread source ne correspond pas au thread abonné avec cet id on écrit une erreur*/
                    writerepcode(zone_reponse, -3);
                    break; /*Fin du switch*/
                }

                if (_zoneRequete.userid1 == 0){ /*Si on veut envoyer en broadcast*/
                    for (i=0; i < *nbthreadmax; i++){ /*On parcourt tout l'annuaire*/
                        if (annuaire[i].id != 0 && annuaire[i].id != annuaire[indexsource].id){ /*Si c'est une entrée abonnée et que ce n'est pas celle de l'emmeteur*/
                            if ((argecriture = calloc(1, sizeof(argThreadEcriture)))==NULL){ /*On alloue une zone pour passer les arguments au thread*/
                                writerepcode(zone_reponse, -5); /*Si on y arrive pas on écrit une erreur*/
                                break; /*Et on sort de la boucle*/
                            }
                            argecriture->boitealettres = annuaire[i].bal; /*On prépare les arguments a passer*/
                            strcpy(argecriture->message, _zoneRequete.msg);
                            argecriture->exist = annuaire[i].exist;
                            #ifdef DEBUGECRITURE
                            printf("Le gestionnaire a écrit les arguments et va lancé le thread d'écriture\n");
                            #endif // DEBUGECRITURE
                            if (pthread_create(&idthreadlance, &attributes, Ecriture, argecriture) != 0){/*Création du thread d'écriture*/
                                free(argecriture); /*Si il y a eu un problème lors de la création du thread on libère la zone des arguments et on écrit un erreur*/
                                argecriture = NULL;
                                writerepcode(zone_reponse, -5);
                                break; /*On sort de la boucle*/
                            }
                            argecriture = NULL; /*Il faut etre sur de ne pas y refaire référence*/
                        }
                    }
                    writerepcode(zone_reponse, 0); /*tout s'est bien passé on écrit 0*/
                    break; /*Fin du switch*/
                }

                indexdest = findid(annuaire, *nbthreadmax, _zoneRequete.userid1); /*On cherche l'identifiant destinataire dans l'annuaire*/
                if (indexdest == -1){ /*Si il n'y est pas on renvoie une erreur*/
                    writerepcode(zone_reponse, -4);
                    break; /*Fin du switch*/
                }


                if ((argecriture = calloc(1, sizeof(argThreadEcriture)))==NULL){ /*On alloue une zone pour passer les arguments au thread*/
                    writerepcode(zone_reponse, -5); /*Si on y arrive pas on écrit une erreur*/
                    break; /*Fin du switch*/
                }
                argecriture->boitealettres = annuaire[indexdest].bal; /*On prépare les arguments a passer*/
                strcpy(argecriture->message, _zoneRequete.msg);
                argecriture->exist = annuaire[indexdest].exist;
                #ifdef DEBUGECRITURE
                printf("Le gestionnaire a écrit les arguments et va lancé le thread d'écriture\n");
                #endif // DEBUGECRITURE
                if (pthread_create(&idthreadlance, &attributes, Ecriture, argecriture) != 0){/*Création du thread d'écriture*/
                    free(argecriture); /*Si il y a eu un problème lors de la création du thread on libère la zone des arguments et on écrit un erreur*/
                    argecriture = NULL;
                    writerepcode(zone_reponse, -5);
                    break; /*Fin du switch*/
                }

                #ifdef DEBUGECRITURE
                printf("[Gestionnaire] Thread écriture lancé, répondu à : %d\n",zone_reponse);
                #endif

                argecriture = NULL; /*Il faut etre sur de ne pas y refaire référence*/
                writerepcode(zone_reponse, 0); /*tout s'est bien passé on écrit 0*/

                break; /*Fin de l'envoi*/

            case 3: /*Reception de message*/
                #ifdef DEBUGRECV
                printf("Le gestionnaire va traiter la requête de lecture de message\n");
                #endif

                indexsource = findid(annuaire, *nbthreadmax, _zoneRequete.userid1); /*On cherche l'identifiant dans l'annuaire*/

                if (indexsource == -1){ /*Si il n'y est pas on écrit une erreur*/
                    writerepcode(zone_reponse, -2);
                    break; /*Fin du switch*/
                }
                if (annuaire[indexsource].idThread != _zoneRequete.id_thread){ /*Si le thread source ne correspond pas au thread abonné avec cet id on écrit une erreur*/
                    writerepcode(zone_reponse, -3);
                    break; /*Fin du switch*/
                }

                if ((arglecture_msg = calloc(1, sizeof(argThreadLecture)))==NULL){ /*On alloue une zone pour passer les arguments au thread*/
                    writerepcode(zone_reponse, -5); /*Si on y arrive pas on écrit une erreur*/
                    break; /*Fin du switch*/
                }
                arglecture_msg->boitealettres = annuaire[indexsource].bal; /*On prépare les arguments a passer*/
                arglecture_msg->repzoneaddr = zone_reponse;
                arglecture_msg->flag = 0;
                arglecture_msg->exist = annuaire[indexsource].exist;

                #ifdef DEBUGRECV
                printf("Le gestionnaire a écrit les arguments et va lancer le thread de lecture\n");
                #endif // DEBUGECRITURE

                if (pthread_create(&idthreadlance, &attributes, Lecture_msg, arglecture_msg) != 0){/*Création du thread d'écriture*/
                    free(arglecture_msg); /*Si il y a eu un problème lors de la création du thread on libère la zone des arguments et on écrit un erreur*/
                    arglecture_msg = NULL;
                    writerepcode(zone_reponse, -5);
                    break; /*Fin du switch*/
                }

                #ifdef DEBUGRECV
                printf("[Gestionnaire] Thread écriture lancé");
                #endif

                arglecture_msg = NULL; /*Il faut etre sur de ne pas y refaire référence*/
                break; /*Fin de la reception*/

            case 4: /*Demande du nombre de message*/

                #ifdef DEBUGRECV
                printf("Le gestionnaire va traiter la requête de lecture de nombre de message\n");
                #endif

                indexsource = findid(annuaire, *nbthreadmax, _zoneRequete.userid1); /*On cherche l'identifiant de la source dans l'annuaire*/

                if (indexsource == -1){ /*Si il n'y est pas on écrit une erreur*/
                    writerepcode(zone_reponse, -2);
                    break; /*Fin du switch*/
                }
                if (annuaire[indexsource].idThread != _zoneRequete.id_thread){ /*Si le thread source ne correspond pas au thread abonné avec cet id on écrit une erreur*/
                    writerepcode(zone_reponse, -3);
                    break; /*Fin du switch*/
                }

                if ((arglecture_msg = calloc(1, sizeof(argThreadLecture)))==NULL){ /*On alloue une zone pour passer les arguments au thread*/
                    writerepcode(zone_reponse, -5); /*Si on y arrive pas on écrit une erreur*/
                    break; /*Fin du switch*/
                }
                arglecture_msg->boitealettres = annuaire[indexsource].bal; /*On prépare les arguments a passer*/
                arglecture_msg->repzoneaddr = zone_reponse;
                arglecture_msg->flag = 1;
                arglecture_msg->exist = annuaire[indexsource].exist;

                #ifdef DEBUGRECV
                printf("Le gestionnaire a écrit les arguments et va lancer le thread de lecture\n");
                #endif // DEBUGECRITURE

                if (pthread_create(&idthreadlance, &attributes, Lecture_msg, arglecture_msg) != 0){/*Création du thread d'écriture*/
                    free(arglecture_msg); /*Si il y a eu un problème lors de la création du thread on libère la zone des arguments et on écrit un erreur*/
                    arglecture_msg = NULL;
                    writerepcode(zone_reponse, -5);
                    break; /*Fin du switch*/
                }

                #ifdef DEBUGRECV
                printf("[Gestionnaire] Thread écriture lancé");
                #endif

                arglecture_msg = NULL; /*Il faut etre sur de ne pas y refaire référence*/
                break; /*Fin de nombre de message*/

            case 5: /*Désabonnment*/
                #ifdef DEBUGDESABO
                printf("Le gestionnaire va traiter une reqûete de désabonnement\n");
                #endif // DEBUGDESABO
                indexsource = findid(annuaire, *nbthreadmax, _zoneRequete.userid1); /*On cherche l'identifiant à désabonner dans l'annuaire*/

                if (indexsource == -1){ /*S'il n'y est pas on écrit une erreur*/
                    writerepcode(zone_reponse, -2);
                    break;
                }

                if (annuaire[indexsource].idThread != _zoneRequete.id_thread){ /*Si le thread demandant le désabonnement n'est pas celui qui s'est abonné on écrit une erreur*/
                    writerepcode(zone_reponse, -3);
                    break;
                }

                desaboid(annuaire, indexsource); /*On désabonne l'identifiant*/

                nbthreadabonne--; /*On décrémente le nombre de threads abonnés*/
                #ifdef DEBUGDESABO
                printf("[Gestionnaire] Désabonnement réussi \n");
                #endif // DEBUGDESABO

                writerepcode(zone_reponse, 0); /*Tout s'est bien passé on écrit le code retour 0 dans la zone de réponse*/
                break; /*Fin désabo*/

            case 6: /*Terminaison douce*/
            #ifdef DEBUGTERM
            printf("[Gestionnaire] Le gestionnaire va traiter une requête de terminaison douce\n");
            #endif // DEBUGTERM
            if (nbthreadabonne != 0){ /*S'il reste des threads abonnés, on écrit une erreur*/
                writerepcode(zone_reponse, -2);
                break;
            }

            writerepcode(zone_reponse, 0); /*On écrit le code retour 0*/
            pthread_exit(NULL); /*On termine le gestionnaire*/
            break; /*Fin terminaison douce*/

            case 7:/*Terminaison brute*/
            #ifdef DEBUGTERM
            printf("[Gestionnaire] Le gestionnaire va traiter une requete de terminaison forcée \n");
            #endif // DEBUGTERM

            for (i = 0; i<*nbthreadmax; i++){ /*On parcourt l'annuaire en quete des abonnés*/
                if(annuaire[i].id != 0){ /*On désabonne chaque abonné restant*/
                    desaboid(annuaire, i);
                    nbthreadabonne--;
                }
            }

            writerepcode(zone_reponse, 0);
            pthread_exit(NULL);
            break;/*fin termaison brute*/
        }/*Fin du switch*/

        _zoneRequete.flag_req = 0; /*On libère le mutex de la requête et on envoie un signal réveillant les thread en attente d'écriture d'une requête*/
        pthread_cond_signal(&(_zoneRequete.var_cond_req_full));
        pthread_mutex_unlock(&(_zoneRequete.mutexreq));
        #ifdef DEBUGGEST
        printf("[Gestionnaire]mutex requête libéré\n");
        for (i=0; i<*nbthreadmax; i++ ){
            printf("[Annuaire] id : %d\tidthread : %d\tBaL : %d\n",annuaire[i].id, annuaire[i].idThread, annuaire[i].bal);
        }
        #endif
    }

}

