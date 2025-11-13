#ifndef __XSND_H
#define __XSND_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XSND.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... SoundBlaster 16, DMA, IRQs...
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    10/11/96  Borja     missing prototipes.
1.0.0    02/03/95  Borja     Codificacion inicial.

======================== Contenido ========================
Cabecera de modulos para la conversion AD/DA de 8/16 bits
con Sound Blaster 16.
Ver XSNDSB16.C para mas informacion.
Se definen dos familias de funciones:

   xsnd16_???() (xsnd16_open(), xsnd16_close()...)
   xsnd8_???() (xsnd8_open(), xsnd8_close()...)

las primeras para trabajar con muestras de 16 bits, las
segundas para 8 bits.

Se define otra familia:

	xsnd_???() (xsnd_open(), xsnd_close()...)

que son alias para una de las familias anteriores (por
defecto xsnd16_???() ).
Creo que deberia unir xsnd16_??? y xsnd8_??? y pasar el numero
de bits como parametro... usar xsnd_??? en prevision a esto.
....................
Header for 8/16 bit AD/DA using SB16.
Refer to XSNDSB16.C for more information.
There are two families of functions:

   xsnd16_???() (xsnd16_open(), xsnd16_close()...)
   xsnd8_???() (xsnd8_open(), xsnd8_close()...)

the first one for 16 bits samples, the second one for 8
bit samples.

There is also another family:

	xsnd_???() (xsnd_open(), xsnd_close()...)

these are alias to one of the previous families (default
alias to xsnd16_???() ).
I think I should unify xsnd16_??? and xsnd8_???, and send the
sample length as a parameter... use xsnd_??? as a prevision
for this change.
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

#define XSND_RECORD TRUE
#define XSND_PLAY FALSE
#define XSND_MONO FALSE
#define XSND_STEREO TRUE

/**********************************************************/

#define xsnd_open  xsnd16_open
#define xsnd_close  xsnd16_close
#define xsnd_getblk  xsnd16_getblk
#define xsnd_addblk  xsnd16_addblk
#define xsnd_addlastblk  xsnd16_addlastblk
#define xsnd_stopped  xsnd16_stopped
#define xsnd_blocked  xsnd16_blocked
#define xsnd_reset  xsnd16_reset
#define xsnd_start  xsnd16_start
#define xsnd_stop  xsnd16_stop
#define xsnd_getoverrun  xsnd16_getoverrun
#define xsnd_getblknum  xsnd16_getblknum
#define xsnd_getnblk  xsnd16_getnblk
#define xsnd_initialize  xsnd16_initialize
#define xsnd_srate  xsnd16_srate
#define xsnd_getsrate  xsnd16_getsrate
#define xsnd_getqblk xsnd16_getqblk

/**********************************************************/

UINT16 xsnd16_open( BOOL record, UINT16 srate, BOOL stereo,
                UINT32 blklen, UINT16 numblk, VOID (PTRF usrproc) ( BOOL last) );
UINT16 xsnd16_close( VOID );
pfINT16 xsnd16_getblk( VOID );
UINT16 xsnd16_addblk( VOID );
UINT16 xsnd16_addlastblk( UINT32 len );
BOOL xsnd16_stopped( VOID );
BOOL xsnd16_blocked( VOID );

UINT16 xsnd16_reset( VOID );
UINT16 xsnd16_start( VOID );
UINT16 xsnd16_stop( VOID );
UINT16 xsnd16_getoverrun( VOID );

pfINT16 xsnd16_getblknum( UINT16 nblk );
UINT16 xsnd16_getnblk( VOID );

UINT16 xsnd16_srate( UINT16 srate );
DOUBLE xsnd16_getsrate( VOID );

UINT16 xsnd16_getqblk( VOID );

UINT16 xsnd16_initialize( VOID );

/**********************************************************/

UINT16 xsnd8_open( BOOL record, UINT16 srate, BOOL stereo,
				UINT32 blklen, UINT16 numblk, VOID (PTRF usrproc) ( BOOL last) );
UINT16 xsnd8_close( VOID );
pfUINT8 xsnd8_getblk( VOID );
UINT16 xsnd8_addblk( VOID );
UINT16 xsnd8_addlastblk( UINT32 len );
BOOL xsnd8_stopped( VOID );
BOOL xsnd8_blocked( VOID );

UINT16 xsnd8_reset( VOID );
UINT16 xsnd8_start( VOID );
UINT16 xsnd8_stop( VOID );
UINT16 xsnd8_getoverrun( VOID );

pfUINT8 xsnd8_getblknum( UINT16 nblk );
UINT16 xsnd8_getnblk( VOID );

UINT16 xsnd8_srate( UINT16 srate );
DOUBLE xsnd8_getsrate( VOID );

UINT16 xsnd8_getqblk( VOID );

UINT16 xsnd8_initialize( VOID );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
