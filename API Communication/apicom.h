#ifndef APICOM_H_INCLUDED
#define APICOM_H_INCLUDED

extern int initMsg(int nombrethmax);
/**
int initMsg(int nombremax) : Permet la création du service (thread, mémoire, mutex, ...).
Doit être appelée une et une seule fois avant toute autre fonction de l'API.
Argument : nombremax : Nombre maximal de threads pouvant être abonné au service.
Retour :
◦ 0 : service lancé
◦ -1 : service déjà lancé
◦ -2 : erreur création thread gestionnaire
*/

extern int aboMsg(int idabo);
/**
int aboMsg(int idabo) : Permet à un thread de s'abonner au service (d'envoyer et de recevoir
des messages) et d'être connu comme le numéro id. Doit être appelée par chaque thread
avant d'utiliser une autre fonction de l'API. L'id doit être différent de 0 (broadcast).
Argument : id : Identifiant du thread sur le service.
Retour :
◦ 0 : identifiant abonné avec l'id API passé en argument
◦ -1 : service non lancé
◦ -2 : identifiant id existant
◦ -3 : nombre maximal de thread abonné atteint
◦ -4 : erreurs techniques (impossible d'allouer de la mémoire, ...)
*/

extern int sendMsg(char * message, int dest_id, int source_id);
/**
int sendMsg(char * message, int dest_id, int source_id) : Permet à un thread de déposer le
message message dans la boîte à lettres du destinataire d'adresse dest_id.
Arguments : message : Le message à faire passer (chaîne de caractères de 60 caractères
max)
dest_id : L'identifiant API de la boite à lettres cible. (0 = broadcast)
source_id : L'identifiant API de l’émetteur du message (utile en cas de plusieurs
abonnements à l'API par un même thread)
Retour :
◦ 0 : message envoyé
◦ -1 : service non lancé
◦ -2 : tâche émettrice non abonnée (id API source inexistant)
◦ -3 : thread émetteur non associé avec cet id API (id API n'a pas été associé à ce thread)
◦ -4 : tâche destinataire non abonnée (id API destinataire inexistant)
◦ -5 : erreurs techniques
*/

extern int recvMsg(int flag, int id, char * message);
/**
int recvMsg(int flag, int id, char ** message) : Permet à un thread d'identifiant API id de
consulter sa boite à lettres. Soit de lire le plus ancien message non lu (fonction bloquante si
la boite à lettre est vide), soit de consulter le nombre de messages non lus.
Arguments :
◦ flag : 0 : lire le plus ancien message (bloquant)
         1 : consulter le nombre de messages non lus
◦ id : id API de la boite à lettres à consulter
◦ message : pointeur où l’utilisateur récupérera le message en cas de lecture
Retour :
◦ 0-* : nombre de messages non lu (dans le cas de la consultation du nombre de messages)
◦ 0 : message lu et présent dans message (dans le cas d'une lecture de message)
◦ -1 : service non lancé
◦ -2 : id non abonné
◦ -3 : thread demandeur non associé à cet id API (ce thread ne s'est pas abonné avec cet id)
◦ -4 : flag (argument) invalide
◦ -5 : Autres erreurs techniques
*/

extern int desaboMsg(int id);
/**
int desaboMsg(int id) : Permet à un thread de se désabonner de l'id API id. Un thread
désabonné ne pourra plus ni envoyé ni recevoir de message, sa boite à lettres est détruite
même s'il reste des messages non lus. L'identifiant id libéré peut être a nouveau utilisé pour
s'abonner.
Argument :
◦ id : identifiant API à désabonner
Retour :
◦ 0 : désabonné
◦ -1 : service non lancé
◦ -2 : id non abonné
◦ -3 : id ne correspondant pas au thread (ce thread n'a pas été abonné avec cet id)
◦ -4 : erreurs techniques
*/

extern int finMsg(int force);
/**
int finMsg(int force) : Termine le service de communication. Si force = 0 : le service
vérifiera qu'il n'y plus de threads abonnés avant de se terminer, si force = 1 : le service
s’arrêtera sans vérifications.
Argument :
◦ force : 0 : le service se termine s'il n'y a plus de tâches abonnées
          1 : le service se termine
Retour :
◦ 0 : le service s'est terminé
◦ -1 : service non lancé
◦ -2 : il reste des threads abonnés, le service ne s'est pas arrêté
◦ -3 : erreurs techniques
*/

#endif // APICOM_H_INCLUDED
