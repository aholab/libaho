#ifndef __XMOUSE_H
#define __XMOUSE_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo TSR - DET ETSII/IT - Bilbao

Nombre fuente................ xmouse.h
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria <borja@bips.bi.ehu.es>

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    28/06/98  Borja     repasillo gordo
0.0.1    14/06/98  Borja     repasillo general
0.0.0    16/08/96  Borja     Codificacion inicial.

======================== Contenido ========================
User interface. Manejo de estados del raton.
Los codigos de evento son compatibles con los de teclado (xkbd()).
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "mouse.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

VOID xmouse_initialize( VOID );
UINT16 xmouse_get( INT16* x, INT16* y );
UINT16 xmouse_getstat( INT16* x, INT16* y );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/
/*<DOC>*/
/* Bits para comprobar estado de valores leidos con xmouse_get().
Los bits forman una mascara compatible con los definidos en xkbd.h */

#define XMOUSE_MOUSE ((UINT16)0x0800)
#define xmouse_ismouse(n)  ((n)&XMOUSE_MOUSE)

#define XMOUSE_LKEY ((UINT16)0x0200)
#define XMOUSE_RKEY ((UINT16)0x0400)
#define XMOUSE_BKEY (XMOUSE_LKEY|XMOUSE_RKEY)
#define XMOUSE_STATBITS XMOUSE_BKEY

#define xmouse_testlkey(n)  ((n) & XMOUSE_LKEY)
#define xmouse_testrkey(n)  ((n) & XMOUSE_RKEY)
#define xmouse_testbkey(n)  (((n) & XMOUSE_BKEY)==XMOUSE_BKEY)
#define xmouse_setlkey(n)   ((n) | XMOUSE_LKEY)
#define xmouse_setrkey(n)   ((n) | XMOUSE_RKEY)
#define xmouse_setbkey(n)   ((n) | XMOUSE_BKEY)
#define xmouse_resetlkey(n) ((n) & ~XMOUSE_LKEY)
#define xmouse_resetrkey(n) ((n) & ~XMOUSE_RKEY)
#define xmouse_resetbkey(n) ((n) & ~XMOUSE_BKEY)

#define xmouse_resetstat(k) ((k) & ~XMOUSE_STATBITS)
#define xmouse_peekstat(k)  ((k) & XMOUSE_STATBITS)

/*</DOC>*/

/**********************************************************/

#define __MK XMOUSE_MOUSE

/*<DOC>*/
/* Valores que {devuelve} xmouse_get() */
enum MouseEvType {
	M_None = 0x000,
	M_Move = __MK | 0x100,
	M_LDown = __MK | 0x101,
	M_RDown = __MK | 0x102,
	M_BDown = __MK | 0x103,
	M_LUp = __MK | 0x104,
	M_RUp = __MK | 0x105,
	M_BUp = __MK | 0x106,
	M_LDownRep = __MK | 0x107,
	M_RDownRep = __MK | 0x108,
	M_BDownRep = __MK | 0x109,
};
/*</DOC>*/

#undef __MK

/**********************************************************/

#endif
