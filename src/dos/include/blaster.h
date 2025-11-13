#ifndef __BLASTER_H
#define __BLASTER_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ BLASTER.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Sound Blaster
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.0    24/08/94  Borja     Constantes dec/hex. Param {hex} en blaster_env().
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera de BLASTER.C, para lectura de
configuracion de tarjetas SoundBlaster a partir
de la variable de entorno BLASTER.
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
/* campos posibles para enviar en {field} a blaster_env()
....................
These are the fields available in the BLASTER environment.
You can send them in {field} to blaster_env() */

#define BLASTER_BASEPORT  'A'     /* puerto base de la SB */
#define BLASTER_IRQ  'I'          /* canal irq utilizado */
#define BLASTER_DMA8  'D'         /* canal DMA de 8 bits */
#define BLASTER_DMA16  'H'        /* canal DMA de 16 bits */
#define BLASTER_MIDIBASE  'P'     /* puerto base de MIDI */
#define BLASTER_CARDTYPE  'T'     /* tipo de tarjeta */


/* valores por defecto para los campos anteriores (para la
tarjeta Sound Blaster 16 ASP
...................
Default values for the available fields in the BLASTER environment */

#define BLASTER_DEFAULT_BASEPORT  0x220
#define BLASTER_DEFAULT_IRQ  0x05
#define BLASTER_DEFAULT_DMA8  0x01
#define BLASTER_DEFAULT_DMA16  0x05
#define BLASTER_DEFAULT_MIDIBASE  0x330
#define BLASTER_DEFAULT_CARDTYPE  0x06


/* base (decimal/hexa) en la que se encuentran los campos anteriores.
Todos los campos son hexa, excepto uno: los muy bestias ponen IRQ en
decimal, para fastidiar un poco, supongo.
....................
Numerical base (decimal/hexadecimal) used for each of the field
available in the BLASTER environment variable. */

#define BLASTER_DECHEX_BASEPORT  TRUE
#define BLASTER_DECHEX_IRQ  FALSE
#define BLASTER_DECHEX_DMA8  TRUE
#define BLASTER_DECHEX_DMA16  TRUE
#define BLASTER_DECHEX_MIDIBASE  TRUE
#define BLASTER_DECHEX_CARDTYPE  TRUE

/**********************************************************/

INT blaster_env( CHAR field, INT default_value, BOOL hex );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
