#ifndef __WRKBUFF_H__
#define __WRKBUFF_H__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DET) ETSII/IT-Bilbao

Nombre fuente................ wrkbuff.h
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... en desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG, NOWRKBUFF

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.0    07/06/95  Borja     Codificacion inicial.

======================== Contenido ========================
Cabecera de WRKBUFF.C.
Para usar un buffer de trabajo global.
Ver comentarios sobre utilizacion en WRKBUFF.C.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"
#include <assert.h>

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/* esta es la longitud del buffer global. */

#define WRKBUFFLEN 10240

/**********************************************************/

/* definir NOWRKBUFF para NO utilizar un buffer global */

#define NOWRKBUFFxx

/**********************************************************/

#ifdef NOWRKBUFF   /* NOWRKBUFF esta definido */

/* usamos buffer automatico en pila */
#define WRKBUFF(n) CHAR _wrkbuff[(n)]
#define WRKBUFF_FREE() {}

#else  /* NOWRKBUFF NO esta definido */

/* asi que usamos buffer global {_wrkbuff}. WRKBUFF(n) comprueba
que {_wrkbuff} tiene elementos suficientes. */
#ifdef NDEBUG
#define WRKBUFF(n) {}
#define WRKBUFF_FREE() {}
#else
#define WRKBUFF(n) { assert((n)<=WRKBUFFLEN); assert(!_wrkbuff_used); _wrkbuff_used=TRUE; }
#define WRKBUFF_FREE() { assert(_wrkbuff_used); _wrkbuff_used=FALSE; }
#endif

#ifndef __WRKBUFF_C__

#ifdef __OS_MSDOS__
EXTERN CHAR _far *_wrkbuff;
#else
EXTERN CHAR *_wrkbuff;
#endif

#ifndef NDEBUG
EXTERN BOOL _wrkbuff_used;
#endif
#endif  /* __WRKBUFF_C */

#endif  /* NOWRKBUFF */

/**********************************************************/

#ifdef __cplusplus
}
#endif

/**********************************************************/

#endif
