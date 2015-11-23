#define _REENTRANT

#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "ctype.h"
#include "apicom.h"

#define DEBUG



void* Ecriture (void * arg)
{
    int i=0;
    char my_message[20];
    aboMsg(999);
    aboMsg(888);
    aboMsg(777);
    aboMsg(555);

	while(1){

        sprintf(my_message, "Message n°%d", i);
		//sendMsg(my_message, 666, 999);
		i++;
		sleep(2);					/*On attend un peu*/
	}
}

void* Affichage (void *arg)
{
    int debug;
    char* my_message[20];

    #ifdef DEBUG
    printf("affichage va s'abonner\n ");
    #endif

    debug = aboMsg(666);
    #ifdef DEBUG
    printf("affichage s'est abonné, code retour : %d\n", debug);
    #endif
	while(1){
        //recvMsg(0, 666, my_message);
		//printf("%s\n",*my_message);	/*On affiche le message présent dans le buffer là ou l'indice de lecture en est*/

		sleep(1);				/*On attend un peu*/
	}
}

void* Supervision (void *arg)
{
#ifdef Supervision
	char command;
	while(1){
		getchar(); /*On attend l'appuie sur la touche entrée*/
		_flagsusp = 1; /*On lève le flag de suspension*/
		scanf("%c", &command);	/*on attend l'appuie sur un caractère*/
		command = toupper(command);	/*On capitalise le caractère*/
		if (command=='Q'){		/*Si le caractère est Q*/
			_flagsusp = 0;		/*On libère le flag de suspension*/
			_flagstop = 1;		/*On lève le flag d'arrêt*/
			pthread_cond_signal(&_condEmptyBuffer);		/*On réveille les 2 tâches pour être sûr qu'elle testent le flagstop*/
			pthread_cond_signal(&_condFullBuffer);
			printf("Fin de la tâche de supervision\n");	/*On s'arrête*/
			pthread_exit(0);
		}
		fflush(stdin);			/*On vide le buffer*/
		_flagsusp = 0;			/*On descend le flag de suspension*/
		pthread_cond_signal(&_condEmptyBuffer);	/*On réveille la tâche de lecture*/
	}
#endif // Supervision
}

int main (void)
{
	pthread_t idThreadEcriture, idThreadAffichage, idThreadSupervision; /*Déclaration des ID de threads*/

	initMsg(3);

    #ifdef DEBUG
    printf("hey salut on va créé les threads\n");
    #endif

	if (pthread_create(&idThreadAffichage, NULL, Affichage, NULL) != 0){ 	/*Création du thread d'affichage*/
		printf("Erreur Création thread d'affichage\n");
		exit(1);
	}

	if (pthread_create(&idThreadEcriture, NULL, Ecriture, NULL) != 0){	/*Création du thread d'écriture*/
		printf("Erreur Création thread d'écriture\n");
		exit(1);
	}


	if (pthread_create(&idThreadSupervision, NULL, Supervision, NULL) != 0){ /*Création de la tâche de supervision*/
		printf("Erreur création thread de supervision\n");
		exit(1);
	}

	pthread_join(idThreadEcriture, NULL);					/*On attend que les threads se terminent*/
	pthread_join(idThreadAffichage, NULL);
	pthread_join(idThreadSupervision, NULL);

	printf("Fin main\n");

	return 0;
}
