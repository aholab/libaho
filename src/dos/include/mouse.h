#ifndef __MOUSE_H__
#define __MOUSE_H__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ MOUSE.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.0    16/08/96  Borja     Codificacion inicial.

======================== Contenido ========================
Manejo del raton
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"

#define MOUSESOFT

/**********************************************************/

#define MOUSE_NOKEY    0
#define MOUSE_LEFTKEY  1
#define MOUSE_RIGHTKEY 2
#define MOUSE_BOTHKEY  3

/**********************************************************/

typedef struct {
  UINT16 mask[2][16];
  INT16 horz_hot_spot, vert_hot_spot;
} mouse_cursor;

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

BOOL mouse_open( VOID );
VOID mouse_close( VOID );

VOID mouse_show( VOID );
VOID mouse_hide( VOID );

VOID mouse_set_icon( mouse_cursor * mc );
VOID mouse_set_xrange( INT16 x0, INT16 x1 );
VOID mouse_set_yrange( INT16 y0, INT16 y1 );
VOID mouse_set_scale( INT16 xratio, INT16 yratio );
VOID mouse_set_xy( INT16 x, INT16 y );

UINT16 mouse_get( INT16* x, INT16* y );

/* #ifdef MOUSESOFT  */
VOID mouse_update(VOID);
VOID mouse_set_showfunc( VOID (*mfunc)(BOOL show, INT16 x, INT16 y) );
/* #endif */

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
