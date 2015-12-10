#include "varglobal.h"
#include "structures.h"

idThreadGest _idgest = {.idgest = 0, .mutexgest = PTHREAD_MUTEX_INITIALIZER};

requestZone _zoneRequete = {.numrequest = 0, .userid1 = 0, .userid2 = 0, .repzoneaddr = NULL, .id_thread = 0, .msg = "", .flag_req = 0, .mutexreq = PTHREAD_MUTEX_INITIALIZER, .var_cond_req_full = PTHREAD_COND_INITIALIZER, .var_cond_req_empty = PTHREAD_COND_INITIALIZER};
