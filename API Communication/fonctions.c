#include "apicom.h"
#include "varglobal.h"
#include "gestionnaire.h"
#include <pthread.h>
#include <stdlib.h>
#include "unistd.h"
#include "stdio.h"
#include "structures.h"
#include "string.h"

//#define DEBUGABO
//#define DEBUGSEND
//#define DEBUGRECV
//#define DEBUGDESABO

int idgestlaunched()  /*Fonction qui vérifie si le thread est lancé, renvoie 0 si le thread gestionnaire n'est pas lancé*/
{
    pthread_mutex_lock(&(_idgest.mutexgest));

    if (_idgest.idgest == 0)
    {
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return 0;
    }
    else
    {
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return 1;
    }

}




int initMsg(int nombrethmax)
{
    int *args; /*L'argument à passer au futur thread gestionnaire*/

    pthread_mutex_lock(&(_idgest.mutexgest)); /*On prend le mutex de idgest*/

    if (_idgest.idgest != 0)  /*Si le service est déjà lancé on retourne -1*/
    {
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return -1;
    }


    else /*Si le service n'est pas lancé*/
    {
        if ((args = calloc(1, sizeof(int))) == NULL){ /*On alloue pour les arguments (on renvoie une erreur si on y arrive pas)*/
            pthread_mutex_unlock(&(_idgest.mutexgest));
            return -2;}

        *args = nombrethmax; /*On renseigne le nombre de thread maximal demandé par l'utilisateur*/
        if (pthread_create(&(_idgest.idgest), NULL, Gestionnaire, args) != 0)  /*On lance le thread gestionnaire et on écrit son id sinon on retourne -2*/
        {
            pthread_mutex_unlock(&(_idgest.mutexgest));
            return -2;
        }
        pthread_mutex_unlock(&(_idgest.mutexgest)); /*On libère le mutex*/
    }
    return 0;
}


int aboMsg(int idabo)
{

    repZone *my_zone_reponse; /*La zone réponse de cette session*/
    int coderet;                /*Le code retour retourné par le thread gestionnaire (pour pouvoir le traiter si nécessaire)*/

    if (!idgestlaunched())  /*On teste si le thread gestionnaire est bien lancé*/
    {
        return -1;
    }

    if ((my_zone_reponse = calloc(1, sizeof(repZone)))==NULL) return -4; /*On alloue la zone de réponse*/

    pthread_mutex_lock(&(_zoneRequete.mutexreq)); /*On prend le mutex de la zone de requête*/
#ifdef DEBUGABO
    printf("aboMsg %d a le mutex\n", pthread_self());
#endif // DEBUGABO

    while(_zoneRequete.flag_req == 1)  /*Si une requête est déjà écrite*/
    {
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
    _zoneRequete.repzoneaddr = my_zone_reponse; /*On renseigne la zone réponse de cette session*/
    _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
    pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
    pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex*/
#ifdef DEBUGABO
    printf("aboMsg lache le mutex\n");
#endif

    pthread_mutex_lock(&(my_zone_reponse->mutexrep)); /*On prend le mutex de la zone de réponse*/
#ifdef DEBUGABO
    printf("aboMsg a le mutex de sa réponse : %d\n", my_zone_reponse);
#endif // DEBUGABO

    while(my_zone_reponse->flag_rep == 0) /*Si il n'y a pas de réponse on libère le mutex et on attend*/
    {
        pthread_cond_wait(&(my_zone_reponse->var_cond_rep), &(my_zone_reponse->mutexrep));
    }
    coderet = my_zone_reponse->code_err; /*Quand il y a une réponse on enregistre le code renseigné par le thread gestionnaire*/
#ifdef DEBUGABO
    printf("aboMsg a une réponse : %d\n", coderet);
#endif // DEBUGABO
    pthread_mutex_unlock(&(my_zone_reponse->mutexrep)); /*On libère le mutex de notre zone réponse*/
    free(my_zone_reponse); /*On libère la zone réponse*/
    my_zone_reponse = NULL; /*Pour être sur de ne pas y retourner*/
#ifdef DEBUGABO
    printf("aboMsg freeing \n");
#endif // DEBUGABO


    return coderet; /*On retourne le code retour à l'utilisateur*/

}


int sendMsg(char * message, int dest_id, int source_id)
{
    repZone *my_zone_reponse; /*La zone de réponse de cette session*/
    int coderet; /*Le code retour que l'on renverra*/

    if (!idgestlaunched())  /*On teste si le thread gesitionnaire est bien lancé*/
    {
        return -1;
    }

    if ((my_zone_reponse = calloc(1, sizeof(repZone))) == NULL) return -5; /*On créé la zone réponse de cette session*/

    pthread_mutex_lock(&(_zoneRequete.mutexreq)); /*On prend le mutex de la zone requete*/

#ifdef DEBUGSEND
    printf("[sendMsg] a le mutex de requête\n");
#endif

    while(_zoneRequete.flag_req == 1)  /*Si une requête est déjà écrite*/
    {
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }
    /*Sinon on écrit la requête d'envoi du message*/
    _zoneRequete.numrequest = 2; /*La requête est un envoi*/
    _zoneRequete.userid1 = dest_id; /*On renseigne le destinataire*/
    _zoneRequete.userid2 = source_id; /*On renseigne la source*/
    strcpy(_zoneRequete.msg, message); /*On renseigne le message*/
    _zoneRequete.id_thread = pthread_self();/*On renseigne l'id du thread emetteur*/
    _zoneRequete.repzoneaddr = my_zone_reponse; /*On renseigne notre zone de réponse*/
    _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
    pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
    pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex*/



    pthread_mutex_lock(&(my_zone_reponse->mutexrep)); /*On prend le mutex de notre zone réponse*/
#ifdef DEBUGSEND
    printf("[sendMsg] a le mutex de sa réponse : %d\n", my_zone_reponse);
#endif

    while(my_zone_reponse->flag_rep == 0) /*Si il n'y a pas de réponse on libère le mutex et on attend*/
    {
        pthread_cond_wait(&(my_zone_reponse->var_cond_rep), &(my_zone_reponse->mutexrep));
    }
    coderet = my_zone_reponse->code_err; /*On récupère le code renseigné par le thread gestionnaire*/
#ifdef DEBUGSEND
    printf("[sendMsg] a une réponse : %d\n", coderet);
#endif
    pthread_mutex_unlock(&(my_zone_reponse->mutexrep)); /*On libère le mutex de la zone réponse*/

    free(my_zone_reponse); /*On libère la zone réponse*/
    my_zone_reponse = NULL;
    return coderet; /*On retourne le code retour*/
}

int recvMsg(int flag, int id, char * message)
{
    repZone *my_zone_reponse;
    int coderet;

    if (flag != 0 && flag != 1){ /*Si le flag est différent de 0 ou 1 on renvoie une erreur*/
        return -4;
    }


    if (!idgestlaunched())  /*On teste si le thread gesitionnaire est bien lancé*/
    {
        return -1;
    }

    if ((my_zone_reponse = calloc(1, sizeof(repZone))) == NULL) return -5; /*On créé la zone réponse de cette session*/

    pthread_mutex_lock(&(_zoneRequete.mutexreq)); /*On prend le mutex de la requête*/

    while(_zoneRequete.flag_req == 1)  /*Si une requête est déjà écrite*/
    {
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }
    /*Sinon on écrit la requête de lecture de message*/
    _zoneRequete.numrequest = 3 + flag; /* Comme flag = 0 ou 1, la requête est égale à 3 (lecture du message) ou 4 (consultation du nombre)*/
    _zoneRequete.userid1 = id; /*On renseigne le destinataire*/
    _zoneRequete.id_thread = pthread_self();/*On renseigne l'id du thread emetteur*/
    _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
    _zoneRequete.repzoneaddr = my_zone_reponse; /*On lui passe la zone réponse*/
    pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
    pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex de requête*/

    pthread_mutex_lock(&(my_zone_reponse->mutexrep)); /*On prend le mutex de réponse*/
#ifdef DEBUGRECV
    printf("[rcvMsg] a le mutex de sa réponse : %d\n", my_zone_reponse);
#endif

    while(my_zone_reponse->flag_rep == 0) /*Si il n'y a pas de réponse on libère le mutex et on attend*/
    {
        pthread_cond_wait(&(my_zone_reponse->var_cond_rep), &(my_zone_reponse->mutexrep));
    }
    coderet = my_zone_reponse->code_err; /*On récupère le code renseigné par le thread gestionnaire*/
#ifdef DEBUGRECV
    printf("[rcvMsg] a une réponse : %d\n", coderet);
#endif

    if (coderet==0){ /*Si tout s'est bien passé*/
        if (flag == 0){ /*Et que l'utilisateur demande le message*/

        strcpy(message, my_zone_reponse->msg); /*On copie le message dans la variable passée par l'utilisateur*/
        }
        if (flag == 1){ /*Si l'utilisateur a plutot demandé le nombre de message*/
            coderet = my_zone_reponse->nb_msg; /*On se prépare à retourner le nombre de messages*/
        }
    }
    return coderet; /*On retourne la valeur à l'utilisateur*/
}

int desaboMsg(int id){
    repZone *my_zone_reponse; /*La zone de réponse de cette session*/
    int coderet;        /*Le code que l'on retournera*/

    if (!idgestlaunched())  /*On teste si le thread gestionnaire est bien lancé*/
    {
        return -1;
    }

    if ((my_zone_reponse = calloc(1, sizeof(repZone)))==NULL) return -4; /*On créé la zone réponse de cette session*/

    pthread_mutex_lock(&(_zoneRequete.mutexreq)); /*On prend le mutex de la zone requete*/

#ifdef DEBUGDESABO
    printf("[desaboMsg] a le mutex de requête\n");
#endif

    while(_zoneRequete.flag_req == 1)  /*Si une requête est déjà écrite*/
    {
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }
    /*Sinon on écrit la requête de désabonnement*/
    _zoneRequete.numrequest = 5; /*La requête est un désabonnement*/
    _zoneRequete.userid1 = id; /*On renseigne l'identifiant*/
    _zoneRequete.id_thread = pthread_self();/*On renseigne l'id du thread emetteur*/
    #ifdef DEBUGDESABO
        printf("[desabMsg] le thread est : %d\t%d\n", _zoneRequete.id_thread, pthread_self());
    #endif // DEBUGDESABO
    _zoneRequete.repzoneaddr = my_zone_reponse; /*On renseigne la zone réponse de cette session*/
    _zoneRequete.flag_req = 1; /*Il y a maintenant une requête à lire*/
    pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
    pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex*/



    pthread_mutex_lock(&(my_zone_reponse->mutexrep)); /*On prend le mutex de réponse*/
#ifdef DEBUGDESABO
    printf("[desaboMsg] a le mutex de sa réponse : %d\n", my_zone_reponse);
#endif

    while(my_zone_reponse->flag_rep == 0) /*Si il n'y a pas de réponse on libère le mutex et on attend*/
    {
        pthread_cond_wait(&(my_zone_reponse->var_cond_rep), &(my_zone_reponse->mutexrep));
    }
    coderet = my_zone_reponse->code_err; /*On récupère le code retour*/
#ifdef DEBUGDESABO
    printf("[desaboMsg] a une réponse : %d\n", coderet);
#endif
    pthread_mutex_unlock(&(my_zone_reponse->mutexrep)); /*On libère le mutex*/

    free(my_zone_reponse); /*On libère la mémoire de la zone réponse*/
    my_zone_reponse = NULL; /*Pour être sur de pas y retourner*/
    return coderet; /*On retourne directement le code retour*/
    }









int finMsg(int force)
    {
    repZone *my_zone_reponse; /*Zone réponse de cette session*/
    int coderet;    /*Que l'on va retourner*/

    if (force != 0 && force != 1) return -3; /*Si le flag n'est pas bon on renvoie une erreur*/

    pthread_mutex_lock(&(_idgest.mutexgest)); /*On prend le mutex de l'id du gestionnaire (pour éviter qu'un autre thread lance une requête pendant la terminaison)*/


    if (_idgest.idgest == 0){ /*Si le gestionnaire n'est pas lancé on libère le mutex et on renvoie une erreur*/
        pthread_mutex_unlock(&(_idgest.mutexgest));
        return -1;
    }

    if ((my_zone_reponse = calloc(1, sizeof(repZone)))==NULL) return -3; /*On créé la zone réponse de cette session*/ /*TODO : gérer les erreurs*/


    pthread_mutex_lock(&(_zoneRequete.mutexreq));

    while(_zoneRequete.flag_req == 1)  /*Si une requête est déjà écrite*/
    {
        pthread_cond_wait(&(_zoneRequete.var_cond_req_full), &(_zoneRequete.mutexreq)); /*On attend en libérant le mutex*/
    }

    _zoneRequete.numrequest = 6 + force; /*Le flag force est soit 0 soit 1 donc la requete est soit 6 (terminaison douce) ou 7 (forcée)*/
    _zoneRequete.repzoneaddr = my_zone_reponse; /*On renseigne notre zone réponse*/
    _zoneRequete.flag_req = 1; /*Il y a maintenant une requete à lire*/
    pthread_cond_signal(&(_zoneRequete.var_cond_req_empty)); /*On réveille le thread gestionnaire s'il est en attente*/
    pthread_mutex_unlock(&(_zoneRequete.mutexreq)); /*On libère le mutex de requête*/


    pthread_mutex_lock(&(my_zone_reponse->mutexrep)); /*On prend le mutex de réponse*/
#ifdef DEBUGDESABO
    printf("[finMsg] a le mutex de sa réponse : %d\n", my_zone_reponse);
#endif

    while(my_zone_reponse->flag_rep == 0) /*Si il n'y a pas de réponse on libère le mutex et on attend*/
    {
        pthread_cond_wait(&(my_zone_reponse->var_cond_rep), &(my_zone_reponse->mutexrep));
    }
    coderet = my_zone_reponse->code_err; /*On récupère le code renseigné par le thread gestionnaire*/

    pthread_mutex_unlock(&(my_zone_reponse->mutexrep)); /*On libère le mutex de notre zone réponse*/
    free(my_zone_reponse); /*On libère la zone réponse*/
    my_zone_reponse = NULL; /*Pour être sur de ne pas y retourner*/

    if (coderet==0){ /*Si le thread gestionnaire s'est bel et bien arreté*/
        _idgest.idgest = 0; /*On renseigne la non existance du thread gestionnaire*/
    }

    pthread_mutex_unlock(&(_idgest.mutexgest)); /*On libère le mutex de l'id du gestionnaire*/
    return coderet; /*On retourne le code retour*/
    }
