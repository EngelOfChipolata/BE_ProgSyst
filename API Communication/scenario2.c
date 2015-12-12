#define _REENTRANT

#include "apicom.h"
#include "stdio.h"
#include "pthread.h"
#include "unistd.h"
#include "stdlib.h"


void* ecriture(void* idarg){
    int *id = idarg;
    int i = 0;
    char message[20];
    aboMsg(*id);
    while(1){
        sprintf(message, "Message %d from %d", i, *id);
        sendMsg(message, 4, *id);
        i++;
        usleep(100000);


    }
}


void* lecture(void* arg){
    char message[20];
    aboMsg(4);
    while(1){
        recvMsg(0, 4, message);
        printf("%s\n", message);
        sleep(1);
    }
}

int main(void){

    pthread_t idEcriture1, idEcriture2, idEcriture3, idLecture;
    int idec1 = 1, idec2 =2, idec3 = 3;

    if (initMsg(4) != 0){
        printf("Erreur initialisation API\n");
        exit(1);
    }


    if (pthread_create(&idEcriture1, NULL, ecriture, &idec1) != 0){
        printf("Erreur création thread écriture 1\n");
        exit(1);
    }

    if (pthread_create(&idEcriture2, NULL, ecriture, &idec2) != 0){
        printf("Erreur création thread écriture 2\n");
        exit(1);
    }

    if (pthread_create(&idEcriture3, NULL, ecriture, &idec3) != 0){
        printf("Erreur création thread écriture 3\n");
        exit(1);
    }

    if (pthread_create(&idLecture, NULL, lecture, NULL) != 0){
        printf("Erreur création thread lecture\n");
        exit(1);
    }

    pthread_join(idEcriture1, NULL);					/*On attend que les threads se terminent*/
	pthread_join(idEcriture2, NULL);
	pthread_join(idEcriture3, NULL);
	pthread_join(idLecture, NULL);

    finMsg(0);

	printf("Fin main\n");

	return 0;

}
