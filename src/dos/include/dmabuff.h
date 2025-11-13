#ifndef DMABUFF_H
#define DMABUFF_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ DMABUFF.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... DMA
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    20/02/94  Borja     Codificacion inicial

======================== Contenido ========================
Fichero de cabecera y definiciones de DMABUFF.C.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"   /* definicion de tipos fundamentales */

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

pfVOID dmabuff_malloc( UINT32 nbytes, UINT16 kbpage, UINT32 align,
                UINT32 granularity, UINT32 * allocbytes );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
