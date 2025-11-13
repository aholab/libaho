#ifndef __CODEC_H
#define __CODEC_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ CODEC.H
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... definicion de tipos fundamentales
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.0    27/08/95  Borja     ADPCM ZyXEL.
1.0.0    23/07/95  Borja     Codificacion inicial.

======================== Contenido ========================
Cabecera de ADPCM.C

Modulos para la codificacion/decodificacion de audio PCM
con diferentes formatos de compresion.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#include "spltdef.h"

/**********************************************************/
/* Intel/DVI ADPCM codec */

typedef struct _ADPCM_IDVI {
	INT16 valprev;	/* Valor anterior de salida */
	INT index;		/* indice en tabla de StepSize */
} ADPCM_IDVI;

SPL_VOID API adpcm_idvi_reset( ADPCM_IDVI * a );
SPL_VOID API adpcm_idvi_code( ADPCM_IDVI * a, pINT16 v16, pUINT8 v8, SPL_INT nd );
SPL_VOID API adpcm_idvi_decode( ADPCM_IDVI * a, pUINT8 v8, pINT16 v16, SPL_INT nd );

#define adpcm_idvi_d2c(d)  ((INT)(((INT)d+1)/2))
#define adpcm_idvi_c2d(c)  ((INT)(((INT)c)*2))
#define ADPCM_IDVI_DGRAIN 2
#define ADPCM_IDVI_CGRAIN 1
#define adpcm_idvi_dgrain() ADPCM_IDVI_DGRAIN
#define adpcm_idvi_cgrain() ADPCM_IDVI_CGRAIN

/**********************************************************/
/* Zyxel ADPCM 2 codec */

typedef struct _ADPCM_ZY2 {
	INT16 Delta;
	INT16 ExtMax;
} ADPCM_ZY2;

SPL_VOID API adpcm_zy2_reset( ADPCM_ZY2 * a );
SPL_VOID API adpcm_zy2_code( ADPCM_ZY2 * a, pINT16 v16, pUINT8 v8, SPL_INT nd );
SPL_VOID API adpcm_zy2_decode( ADPCM_ZY2 * a, pUINT8 v8, pINT16 v16, SPL_INT nd );

#define adpcm_zy2_d2c(d)  ((INT)(((INT)d+3)/4))
#define adpcm_zy2_c2d(c)  ((INT)(((INT)c)*4))
#define ADPCM_ZY2_DGRAIN 4
#define ADPCM_ZY2_CGRAIN 1
#define adpcm_zy2_dgrain() ADPCM_ZY2_DGRAIN
#define adpcm_zy2_cgrain() ADPCM_ZY2_CGRAIN

/**********************************************************/
/* Zyxel ADPCM 3 codec */

typedef struct _ADPCM_ZY3 {
	INT16 Delta;
	INT16 ExtMax;
} ADPCM_ZY3;

SPL_VOID API adpcm_zy3_reset( ADPCM_ZY3 * a );
SPL_VOID API adpcm_zy3_code( ADPCM_ZY3 * a, pINT16 v16, pUINT8 v8, SPL_INT nd );
SPL_VOID API adpcm_zy3_decode( ADPCM_ZY3 * a, pUINT8 v8, pINT16 v16, SPL_INT nd );

#define adpcm_zy3_d2c(d)  ((INT)(((LONG)d*3+7)/8))
#define adpcm_zy3_c2d(c)  ((INT)(((LONG)c*8)/3))
#define ADPCM_ZY3_DGRAIN 8
#define ADPCM_ZY3_CGRAIN 3
#define adpcm_zy3_dgrain() ADPCM_ZY2_DGRAIN
#define adpcm_zy3_cgrain() ADPCM_ZY2_CGRAIN

/**********************************************************/

#endif
