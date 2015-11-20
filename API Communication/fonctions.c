#define _REENTRANT

#include "apicom.h"
#include "varglobal.h"
#include "gestionnaire.h"
#include "pthread.h"
#include "stdio.h"




int initMsg(int nombrethmax){

    pthread_mutex_lock(&(_idgest.mutexgest)); /*On prend le mutex de idgest*/

    if (_idgest.idgest != 0){ /*Si le service est déjà lancé on retourne -1*/
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return -1;
    }


    else {
        if (pthread_create(&(_idgest.idgest), NULL, Gestionnaire, &nombrethmax) != 0){ /*On lance le thread gestionnaire et on écrit son id sinon on retourne -2*/
            pthread_mutex_unlock(&(_idgest.mutexgest));
            return -2;
        }
        pthread_mutex_unlock(&(_idgest.mutexgest));
    }
    return 0;
}


int aboMsg(int idabo){

}

int sendMsg(char * message, int dest_id, int source_id){

}

int recvMsg(int id, int flag){

}

int desaboMsg(int id){

}

int finMsg(int force){

}
