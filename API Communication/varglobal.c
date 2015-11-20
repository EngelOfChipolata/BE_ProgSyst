#include "varglobal.h"

_idgest.idgest = 0;
_idgest.mutexgest = PTHREAD_MUTEX_INITIALIZER;

_zoneRequete.flag_req = 0;
_zoneRequete.id_thread = 0;
_zoneRequete.msg = "";
_zoneRequete.mutexreq = PTHREAD_MUTEX_INITIALIZER;
_zoneRequete.numrequest = 0;
_zoneRequete.repzoneaddr = NULL;
_zoneRequete.userid1 = 0;
_zoneRequete.userid2 = 0;
_zoneRequete.var_cond_req_empty = PTHREAD_COND_INITIALIZER;
_zoneRequete.var_cond_req_full = PTHREAD_COND_INITIALIZER;
