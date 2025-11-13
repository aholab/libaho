#ifndef __LPC10_H
#define __LPC10_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LPC10.H
Nombre paquete............... LPC10E
Lenguaje fuente.............. C
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.0    10/03/97  Borja     codificacion inicial

======================== Contenido ========================
Cabecera para el codificador LPC10E a 2400bps.
Basado en unos fuentes de dominio publico.

Documentacion sobre todas las funciones, en lpc10_c1.c
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#include "tdef.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

#define LPC10_LFRAME 180
#define LPC10_ORDER 10

#define LPC10_DOUTMAX 320

/**********************************************************/

typedef struct {
	VOID *dummy;
} CLPC10_2400;

typedef struct {
	VOID *dummy;
} DLPC10_2400;

typedef struct {
	VOID *dummy;
} CLPC10_DGRAM;

typedef struct {
	VOID *dummy;
} DLPC10_DGRAM;

typedef struct {
	VOID *dummy;
} PTHL10;

typedef INT LPC10_2400_idata[54];
typedef BYTE LPC10_2400_data[7];
typedef BYTE LPC10_dgram_data[7];

/**********************************************************/

CLPC10_2400 *clpc10_2400_construct( VOID );
VOID clpc10_2400_destruct( CLPC10_2400 *l );
VOID clpc10_2400_reset( CLPC10_2400 *l );
LPC10_2400_idata *clpc10_2400_icode( CLPC10_2400 *l, FLOAT speech[LPC10_LFRAME] );
LPC10_2400_idata *clpc10_2400_icode_i16( CLPC10_2400 *l, INT16 speech[LPC10_LFRAME] );
VOID clpc10_2400_pack( LPC10_2400_idata *in, LPC10_2400_data *o);

/**********************************************************/

DLPC10_2400 *dlpc10_2400_construct( VOID );
VOID dlpc10_2400_destruct( DLPC10_2400 *d );
VOID dlpc10_2400_reset( DLPC10_2400 *d );
FLOAT *dlpc10_2400_outvec( DLPC10_2400 *d );
INT16 *dlpc10_2400_outvec_i16( DLPC10_2400 *d );
INT dlpc10_2400_idecode( DLPC10_2400 *d, LPC10_2400_idata *bits );
INT dlpc10_2400_idecode_i16( DLPC10_2400 *d, LPC10_2400_idata *bits );
VOID dlpc10_2400_unpack( LPC10_2400_data *in, LPC10_2400_idata *o);

/**********************************************************/

CLPC10_DGRAM *clpc10_dgram_construct( VOID );
VOID clpc10_dgram_destruct( CLPC10_DGRAM *l );
VOID clpc10_dgram_reset( CLPC10_DGRAM *l );
LPC10_dgram_data *clpc10_dgram_code( CLPC10_DGRAM *l, FLOAT speech[LPC10_LFRAME] );
LPC10_dgram_data *clpc10_dgram_code_i16( CLPC10_DGRAM *l, INT16 speech[LPC10_LFRAME] );

/**********************************************************/

DLPC10_DGRAM *dlpc10_dgram_construct( VOID );
VOID dlpc10_dgram_destruct( DLPC10_DGRAM *d );
VOID dlpc10_dgram_reset( DLPC10_DGRAM *d );
FLOAT *dlpc10_dgram_outvec( DLPC10_DGRAM *d );
INT16 *dlpc10_dgram_outvec_i16( DLPC10_DGRAM *d );
INT dlpc10_dgram_decode( DLPC10_DGRAM *d, LPC10_dgram_data *dgram );
INT dlpc10_dgram_decode_i16( DLPC10_DGRAM *d, LPC10_dgram_data *dgram );

/**********************************************************/

PTHL10 *pthl10_construct( VOID );
VOID pthl10_destruct( PTHL10 *p );
VOID pthl10_reset( PTHL10 *p );
INT pthl10_get_i16( PTHL10 *p, INT16 speech[LPC10_LFRAME] );

/**********************************************************/

VOID c_lpc10_ini( VOID );
VOID c_lpc10_code( FLOAT speech[] );
VOID c_lpc10_code_i16( INT16 speech[] );
VOID c_lpc10_peek( BOOL voice[2], INT *pitch,
		FLOAT *rms, FLOAT rc[LPC10_ORDER] );

VOID d_lpc10_ini( VOID );
INT d_lpc10_decode( BOOL voice[2], INT pitch, FLOAT rms,
	FLOAT rc[LPC10_ORDER], FLOAT speech[] );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
