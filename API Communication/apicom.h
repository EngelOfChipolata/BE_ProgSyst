#ifndef APICOM_H_INCLUDED
#define APICOM_H_INCLUDED
int initMsg(int nombrethmax);
int aboMsg(int idabo);
int sendMsg(char * message, int dest_id, int source_id);
int recvMsg(int id, int flag);
int desaboMsg(int id);
int finMsg(int force);

#endif // APICOM_H_INCLUDED
