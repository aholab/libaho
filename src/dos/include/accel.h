#ifndef __ACCEL_H
#define __ACCEL_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ ACCEL.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.0    23/11/95  Borja     Codificacion inicial.

======================== Contenido ========================
Acelerador (para cursores graficos...)
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

VOID accel_ini( UINT16 max, UINT16 time_ms );
UINT16 accel_get( UINT16 event );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
