#ifndef APICOM_H_INCLUDED
#define APICOM_H_INCLUDED
extern int initMsg(int nombrethmax);
extern int aboMsg(int idabo);
extern int sendMsg(char * message, int dest_id, int source_id);
extern int recvMsg(int id, int flag);
extern int desaboMsg(int id);
extern int finMsg(int force);

#endif // APICOM_H_INCLUDED
