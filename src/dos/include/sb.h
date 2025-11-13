#ifndef __SB_H
#define __SB_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SB.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Sound Blaster
Codigo condicional........... DSP_GET_DOUBLE_RATE

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.0	 03/03/95  Borja     funciones IRQ_ack_???
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Prototipos para primitivas basicas para tarjetas
SoundBlaster definidas en SB.C.

Si DSP_GET_DOUBLE_RATE esta definido, se declara la
funcion sb_DSP_tc2dsr() que utiliza numeros en coma
flotante.
....................
Basic primitives for SB cards. Code is in SB.C.

If DSP_GET_DOUBLE_RATE is defined, the function
sb_DSP_tc2dsr() is declared. This function uses floating
point numbers.
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
/* 0xFFFF, codigo que {devuelven} las funciones en caso de error
....................
0xFFFF, error code {retured} in all function */
#define DSP_ERROR 0xFFFF

/* quitar este define para eliminar la funcion sb_DSP_tc2dsr(),
que utiliza numeros de coma flotante.
....................
Undefine this to strip sb_DSP_tc2dsr(), that uses floating point
numbers */
#define DSP_GET_DOUBLE_RATE

/**********************************************************/
/* mascara de bits para estado de interrupciones del DSP.
....................
DSP interrup status mask */
#define DSP_MASK_IRQ_DMA8MIDI  0x01
#define DSP_MASK_IRQ_DMA16  0x02
#define DSP_MASK_IRQ_MPU  0x04

/**********************************************************/
/* constantes utiles
....................
Usefull constants */

#define SB_DSP_OUTPUT        FALSE
#define SB_DSP_INPUT         TRUE
#define SB_DSP_SINGLE_CICLE  FALSE
#define SB_DSP_AUTO_INIT     TRUE
#define SB_DSP_FIFO_OFF      FALSE
#define SB_DSP_FIFO_ON       TRUE
#define SB_DSP_MONO          FALSE
#define SB_DSP_STEREO        TRUE
#define SB_DSP_UNSIGNED      FALSE
#define SB_DSP_SIGNED        TRUE

/**********************************************************/

UINT16 sb_DSP_reset( UINT16 _sb_baseport );
UINT16 sb_DSP_read( UINT16 _sb_baseport );
UINT16 sb_DSP_write( UINT16 _sb_baseport, UINT8 data );
UINT8 sb_IRQ_stat( UINT16 _sb_baseport );
VOID sb_IRQ_ack( UINT16 _sb_baseport );
VOID sb_IRQ_ack_dma8midi( UINT16 _sb_baseport );
VOID sb_IRQ_ack_dma16( UINT16 _sb_baseport );
VOID sb_IRQ_ack_mpu( UINT16 _sb_baseport );
UINT16 sb_DSP_out8( UINT16 _sb_baseport, UINT8 sample_val );
UINT16 sb_DSP_dma_out8_single( UINT16 _sb_baseport, UINT16 samples_less_1 );
UINT16 sb_DSP_dma_out8_auto( UINT16 _sb_baseport );
UINT16 sb_DSP_in8( UINT16 _sb_baseport );
UINT16 sb_DSP_dma_in8_single( UINT16 _sb_baseport, UINT16 samples_less_1 );
UINT16 sb_DSP_dma_in8_auto( UINT16 _sb_baseport );
UINT8 sb_DSP_sr2tc( UINT16 sampling_rate, UINT8 channels );
UINT16 sb_DSP_tc2sr( UINT8 time_constant, UINT8 channels );
#ifdef DSP_GET_DOUBLE_RATE
DOUBLE sb_DSP_tc2dsr( UINT8 time_constant, UINT8 channels );
#endif
UINT16 sb_DSP_tconst( UINT16 _sb_baseport, UINT8 time_constant );
UINT16 sb_DSP_out_rate( UINT16 _sb_baseport, UINT16 sampling_rate );
UINT16 sb_DSP_in_rate( UINT16 _sb_baseport, UINT16 sampling_rate );
UINT16 sb_DSP_blk_size( UINT16 _sb_baseport, UINT16 block_transfer_size_less_1 );
UINT16 sb_DSP_out_silence( UINT16 _sb_baseport, UINT16 samples_less_1 );
UINT16 sb_DSP_dma_out8hs_auto( UINT16 _sb_baseport );
UINT16 sb_DSP_dma_out8hs_single( UINT16 _sb_baseport );
UINT16 sb_DSP_dma_in8hs_auto( UINT16 _sb_baseport );
UINT16 sb_DSP_dma_in8hs_single( UINT16 _sb_baseport );
UINT16 sb_DSP_mono( UINT16 _sb_baseport );
UINT16 sb_DSP_stereo( UINT16 _sb_baseport );
UINT16 sb_DSP_16( UINT16 _sb_baseport, BOOL input, BOOL auto_init,
                BOOL fifo, BOOL stereo, BOOL signed_val, UINT16 samples_less_1 );
UINT16 sb_DSP_8( UINT16 _sb_baseport, BOOL input, BOOL auto_init,
                BOOL fifo, BOOL stereo, BOOL signed_val, UINT16 samples_less_1 );
UINT16 sb_DSP_8_pause( UINT16 _sb_baseport );
UINT16 sb_DSP_speaker_on( UINT16 _sb_baseport );
UINT16 sb_DSP_speaker_off( UINT16 _sb_baseport );
UINT16 sb_DSP_8_continue( UINT16 _sb_baseport );
UINT16 sb_DSP_16_pause( UINT16 _sb_baseport );
UINT16 sb_DSP_16_continue( UINT16 _sb_baseport );
UINT16 sb_DSP_speaker( UINT16 _sb_baseport );
UINT16 sb_DSP_16_exit( UINT16 _sb_baseport );
UINT16 sb_DSP_8_exit( UINT16 _sb_baseport );
UINT16 sb_DSP_version( UINT16 _sb_baseport );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif

