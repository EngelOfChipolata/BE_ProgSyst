#define _REENTRANT

#include "apicom.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"


void* pingpong(void * arg){
    int * myarg = arg;
    int role = *myarg + 1;
    char message[20];
    char recv[20];
    int i = 0;

    aboMsg(role);
    if (role == 2){
        while (sendMsg("Test", 0, role) != 0);
    }

    while (1){
        recvMsg(0, role, recv);
        printf("%d a reçu : %s\n", role, recv);
        sprintf(message, "ping %d from %d", i, role);
        sendMsg(message, 0, role);
        i++;
        sleep(1);
    }
}

int main(void){
    pthread_t id1, id2;
    int amorceur = 1, receveur = 0;

    if (initMsg(2) != 0){
        printf("Erreur initialisation API\n");
        exit(1);
    }

    if (pthread_create(&id1, NULL, pingpong, &amorceur) != 0){
        printf("Erreur création thread 1\n");
        exit(1);
    }

    if (pthread_create(&id2, NULL, pingpong, &receveur) != 0){
        printf("Erreur création thread 2\n");
        exit(1);
    }

    pthread_join(id1, NULL);					/*On attend que les threads se terminent*/
	pthread_join(id2, NULL);
    return 0;

}
