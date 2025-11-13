#ifndef __FVBLK_H
#define __FVBLK_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ FVBLK.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... NO
Codigo condicional........... SI (NDEBUG)

Codificacion................. Borja Etxebarria

Vers.   DD/MM/AA   Autor     Proposito de la edicion
-----   --------   -------   -----------------------
01.00   19/12/93   Borja     Codificacion inicial.

======================== Contenido ========================
Cabecera de FVBLK.C.
Lectura por bloques (entramado) de un fichero de voz.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"
#include "fvoz.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

typedef struct {
  fseni * _fs;
  INT _fl;
  INT _prefr;
  INT _posfr;
  INT _nread;
  UINT4 _nm;
  UINT4 _blkn;
  pDOUBLE _vblk;
  } fvblk;

/**********************************************************/

fvblk * fvblk_construct( pCHAR fname, INT fl,
                         INT prefr, INT posfr );
void fvblk_destruct( fvblk * fvb );
UINT4 fvblk_getnm( fvblk * fvb );
UINT4 fvblk_getfm( fvblk * fvb );
UINT4 fvblk_getnblks( fvblk * fvb );
BOOL fvblk_forward( fvblk * fvb );
INT fvblk_getnread( fvblk * fvb );
INT fvblk_getblknsamp( fvblk * fvb );
INT fvblk_getfl( fvblk * fvb );
pDOUBLE fvblk_getblk( fvblk * fvb );
UINT4 fvblk_getblknum( fvblk * fvb );
struct cgsenal * fvblk_getcgs( fvblk * fvb );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
