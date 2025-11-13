#ifndef __SB16IO_H
#define __SB16IO_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SB16IO.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Sound Blaster, dma..., infinitas
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    02/04/95  Borja     Codificacion inicial.

======================== Contenido ========================
Cabecera del modulo SB16IO.C, para grabar y reproducir
simultaneamente con la sound blaster 16 (para hacer efectos
especiales en tiempo real).

Para que funcione la SB16 debe poder utilizar el canal DMA
de 16 bits.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"

/**********************************************************/
/* constantes utiles */

#define SB16IO_RECORD8 TRUE
#define SB16IO_RECORD16 FALSE
#define SB16IO_PLAY8 FALSE
#define SB16IO_PLAY16 TRUE

#define SB16IO_STEREO TRUE
#define SB16IO_STEREO8 TRUE
#define SB16IO_STEREO16 TRUE
#define SB16IO_MONO FALSE
#define SB16IO_MONO8 FALSE
#define SB16IO_MONO16 FALSE

#define SB16IO_NOUSRPROC 0
#define SB16IO_NOCALLBACK 0

#define SB16IO_WAITNEW TRUE
#define SB16IO_NOWAIT FALSE

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

UINT16 sb16io_open( BOOL record8, UINT16 srate, BOOL stereo8, BOOL stereo16,
                UINT16 blklen, VOID (PTRF usrproc) ( VOID ) );

UINT16 sb16io_close( VOID );

UINT16 sb16io_start( VOID );

UINT16 sb16io_stop( VOID );

UINT16 sb16io_getoverrun( VOID );

UINT16 sb16io_pause( VOID );

UINT16 sb16io_continue( VOID );

pfINT16 sb16io_get16( BOOL waitnew );

pfINT8 sb16io_get8( BOOL waitnew );

UINT16 sb16io_setdelay( UINT16 delay );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
