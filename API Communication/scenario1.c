#define _REENTRANT

#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "ctype.h"
#include "apicom.h"


int _flagsusp = 0;
int _flagstop = 0;


void* ecriture (void * arg)
{
    int i=0;
    char my_message[20];
    if (aboMsg(999) != 0){
        printf("Erreur lors de l'abonnement du thread d'écriture\n");
        pthread_exit(NULL);
    };
	while(1){
        if (_flagstop){
            desaboMsg(999);
            printf("Arrêt de la tâche d'écriture\n");
            pthread_exit(NULL);
        }
        sprintf(my_message, "Message n°%d", i);
		sendMsg(my_message, 666, 999);
		i++;
		sleep(2);				/*On attend un peu*/
	}
}

void* affichage (void *arg)
{
    char my_message[60];

    if (aboMsg(666) != 0){
        printf("Erreur lors de l'abonnement du thread affichage\n");
        pthread_exit(NULL);
    }

	while(1){
        while (_flagsusp == 1){
            sleep(2);
        }
        if (_flagstop && recvMsg(1, 666, NULL) <= 0){
            desaboMsg(666);
            printf("Arrêt de la tâche de lecture\n");
            pthread_exit(NULL);
        }
        if (recvMsg(1, 666, NULL) != 0){ /*On est obligé de vérifier s'il y a des messages car on ne veut pas le caractère bloquant ici (en cas de _flagstop levé)*/
        recvMsg(0, 666, my_message);
        printf("%s\n",my_message);
        }
		sleep(1);				/*On attend un peu*/
	}
	pthread_exit(NULL);
}

void* Supervision (void *arg)
{
	char command;
	while(1){
		getchar(); /*On attend l'appuie sur la touche entrée*/
		_flagsusp = 1; /*On lève le flag de suspension*/
		scanf("%c", &command);	/*on attend l'appuie sur un caractère*/
		command = toupper(command);	/*On capitalise le caractère*/
		if (command=='Q'){		/*Si le caractère est Q*/
			_flagsusp = 0;		/*On libère le flag de suspension*/
			_flagstop = 1;		/*On lève le flag d'arrêt*/
			printf("Fin de la tâche de supervision\n");	/*On s'arrête*/
			pthread_exit(NULL);
		}
		fflush(stdin);			/*On vide le buffer*/
		_flagsusp = 0;			/*On descend le flag de suspension*/
	}
}

int main (void)
{
	pthread_t idThreadEcriture, idThreadAffichage, idThreadSupervision; /*Déclaration des ID de threads*/

	initMsg(2);

	if (pthread_create(&idThreadAffichage, NULL, affichage, NULL) != 0){ 	/*Création du thread d'affichage*/
		printf("Erreur Création thread d'affichage\n");
		exit(1);
	}

	if (pthread_create(&idThreadEcriture, NULL, ecriture, NULL) != 0){	/*Création du thread d'écriture*/
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

    finMsg(0);

	printf("Fin main\n");

	return 0;
}
