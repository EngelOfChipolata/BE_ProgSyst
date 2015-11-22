#include "varglobal.h"
#include "structures.h"

idThreadGest _idgest = {0, PTHREAD_MUTEX_INITIALIZER};
requestZone _zoneRequete = {0, 0, 0, NULL, 0, "", 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};
