#include "apicom.h"
#include "varglobal.h"
#include "gestionnaire.h"
#include <pthread.h>
#include <stdlib.h>
#include "unistd.h"
#include "stdio.h"
#include "structures.h"
#include "string.h"

#define DEBUGABO


int idgestlaunched(){ /*Fonction qui vérifie si le thread est lancé renvoie 0 si le thread gestionnaire n'est pas lancé*/
    pthread_mutex_lock(&(_idgest.mutexgest)); /*On prend le mutex*/

    if (_idgest.idgest == 0){
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return 0;
    }
    else{
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return 1;
    }

}




int initMsg(int nombrethmax){
    int *args;

    pthread_mutex_lock(&(_idgest.mutexgest)); /*On prend le mutex de idgest*/

    if (_idgest.idgest != 0){ /*Si le service est déjà lancé on retourne -1*/
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return -1;
    }


    else {
        args = malloc(sizeof(int));
        *args = nombrethmax;
        if (pthread_create(&(_idgest.idgest), NULL, Gestionnaire, args) != 0){ /*On lance le thread gestionnaire et on écrit son id sinon on retourne -2*/
            pthread_mutex_unlock(&(_idgest.mutexgest));
            return -2;
        }
        pthread_mutex_unlock(&(_idgest.mutexgest));
    }
    return 0;
}


int aboMsg(int idabo){

    if (!idgestlaunched()){ /*On teste si le thread gestionnaire est bien lancé*/
        return -1;
    }
    /*TODO : Régler ce problème de réponse*/

    #ifdef DEBUGABO
    printf("quelqu'un veut s'abonner\n");
    #endif

    pthread_mutex_lock(&(_zoneRequete.mutexreq)); /*On prend le mutex de la zone de requête*/
    #ifdef DEBUGABO
    printf("aboMsg a le mutex\n");
    #endif // DEBUGABO

    if(_zoneRequete.flag_req == 1){ /*Si une requête est déjà écrite*/
        #ifdef DEBUGABO
        printf("aboMsg attend\n");
        #endif
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }
     /*Sinon on écrit la requête d'abonnement !*/
    #ifdef DEBUGABO
    printf("aboMsg écrit la requete d'abo : %d\n", idabo);
    #endif // DEBUGABO
        _zoneRequete.numrequest = 1; /*La requete est un abonnement*/
        _zoneRequete.userid1 = idabo; /*On passe l'id voulu*/
        _zoneRequete.id_thread = pthread_self(); /*On renseigne l'id du thread actuel*/
        _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
        pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
        pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex*/
    #ifdef DEBUGABO
    printf("aboMsg lache le mutex\n");
    #endif

    /*TODO : Réponse..................................................*/

    return 0;

}

int sendMsg(char * message, int dest_id, int source_id){
    if (!idgestlaunched()){ /*On teste si le thread gesitionnaire est bien lancé*/
        return -1;
    }

    if(_zoneRequete.flag_req == 1){ /*Si une requête est déjà écrite*/
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }
    /*Sinon on écrit la requête d'envoi du message*/
    _zoneRequete.numrequest = 2; /*La requête est un envoi*/
    _zoneRequete.userid1 = dest_id; /*On renseigne le destinataire*/
    _zoneRequete.userid2 = source_id; /*On renseigne la source*/
    strcpy(_zoneRequete.msg, message); /*On renseigne le message*/
    _zoneRequete.id_thread = pthread_self();/*On renseigne l'id du thread emetteur*/
    _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
    pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
    pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex*/

    /*TODO : Réponse*/
    return 0;
}

int recvMsg(int flag, int id, char ** message){
    if (!idgestlaunched()){ /*On teste si le thread gesitionnaire est bien lancé*/
        return -1;
    }

    if(_zoneRequete.flag_req == 1){ /*Si une requête est déjà écrite*/
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }
    /*Sinon on écrit la requête de lecture de message*/
    if(flag==0){
        _zoneRequete.numrequest = 3; /*La requête est une lecture*/
        _zoneRequete.userid1 = id; /*On renseigne le destinataire*/
        _zoneRequete.id_thread = pthread_self();/*On renseigne l'id du thread emetteur*/
        _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
        pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
        pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex*/
    }

    /*TODO : Gérer les réponses*/
    return 0;
}

int desaboMsg(int id){
    return 0;
}

int finMsg(int force){
    return 0;
}
