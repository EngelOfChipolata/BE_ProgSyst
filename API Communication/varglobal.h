#ifndef VARGLOBAL_H_INCLUDED
#define VARGLOBAL_H_INCLUDED

#include "structures.h"

extern requestZone _zoneRequete;
/**
_zoneRequete : La zone de requête, interface entre le thread gestionnaire et les fonctions appelées par l'utilisateur
*/

extern idThreadGest _idgest;
/**
_idgest : Permet de savoir si le thread gestionnaire est lancé ou non
*/

#endif // VARGLOBAL_H_INCLUDED
