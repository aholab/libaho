#ifndef LMEM_H
#define LMEM_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LMEM.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Memoria segmentada
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
3.0.0    22/02/94  Borja     unificacion 64kb y 128kb ---> {kbpage}, etc
2.1.0    20/02/94  Borja     funciones fixlen???()
2.0.0    20/02/94  Borja     incorporacion de {align} y {granularity}
1.0.1    12/02/94  Borja     correccion de bug en macro LMEM2PTR()
1.0.0    06/08/93  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera y definiciones de LMEM.C.

Macros para la conversion entre un puntero estandar (pfVOID)
y un puntero de memoria lineal de 32 bits (UINT32) dependen
del SO y del compilador.

Declaracion de funciones fix??() para la adecuacion de bloques
de memoria en operaciones de transferencia DMA. Utilizan los
tipos y macros definidos anteriormente, y en principio no
necesitan modificarse para compiladores o SO diferentes.
....................
Header for LMEM.C

Macros to convert a far pointer (pfVOID) to a linear 32 bit
pinter (UINT32).
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
/* PTR2LMEM es una macro que debe convertir un puntero p de
tipo pfVOID (es decir, el tipo mas amplio y generico que
contempla el compilador) a un puntero de direccionamiento
lineal (memoria plana) de 32 bits (tipo UINT32).
LMEM2PTR debe hacer lo contrario, convertir un puntero l de
direccionamiento lineal (tipo UINT32) en un puntero de tipo
generico pfVOID de los que utiliza el compilador
....................
PTR2LMEM converts a far pointer (pfVOID) {p} to a flat 32 bit
pointer (UINT32).
LMEM2PTR is the inverse conversion: flat pointer to far pointer. */

#include <dos.h>    /* para FP_SEG(), FP_OFF() y MK_FP() */

#define PTR2LMEM(p)  ( (((UINT32)FP_SEG((pfVOID)(p)))<<4) +  \
        (UINT32)(FP_OFF((pfVOID)(p))) )

#define LMEM2PTR(l)  \
        (MK_FP((UINT16)((((UINT32)(l))>>4)&0xFFFF),(UINT16)(((UINT32)(l))&0x000F)))

/**********************************************************/

pfVOID fixmem( pfVOID ptr, UINT32 nbytes, UINT16 kbpage, UINT32 align,
                UINT32 granularity, UINT32 * lmem, UINT32 * usebytes );

UINT32 fixlen( pfVOID ptr, UINT32 nbytes, UINT16 kbpage, UINT32 align,
                UINT32 granularity );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
