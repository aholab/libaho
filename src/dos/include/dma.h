#ifndef __DMA_H
#define __DMA_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ DMA.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... DMA
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.0    28/02/94  Borja     Unificado DMA8 con DMA16
1.0.0    06/08/93  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera y definiciones de DMA.H.

Definicion de funciones para la programacion del DMA
de 8 y 16 bits del AT
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "lmem.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/* constantes utiles
....................
These are some useful alternative names for boolean flags that you can
use to clarify your source code. */

#define DMA_AUTO_INIT   TRUE
#define DMA_SINGLE_CICLE   FALSE
#define DMA_WRITE_TO_MEM   TRUE
#define DMA_READ_FROM_MEM   FALSE
#define DMA_INCREMENT  FALSE
#define DMA_DECREMENT  TRUE
#define DMA_ENABLE   TRUE
#define DMA_DISABLE   FALSE

/**********************************************************/
/* {devuelve} TRUE cuando el canal {channel} es de 16 bits,
o FALSE sin es de 8
....................
{returns} TRUE when {channel} is a 16 bit DMA channel, or
FALSE if it is an 8 bit channel */

#define DMA_IS_DMA16(channel)  ((channel) > 3)

/**********************************************************/

VOID dma_resetb( VOID );
VOID dma_set( UINT8 channel, BOOL decrement, BOOL autoinit, BOOL writetomem,
                UINT32 lmem, UINT16 count_l1, BOOL enable_dma );
VOID dma_set_mode( UINT8 channel, BOOL decrement, BOOL autoinit,
		BOOL writetomem, BOOL enable_dma );
VOID dma_set_buff( UINT8 channel, UINT32 lmem, UINT16 count_l1,
		BOOL enable_dma );
VOID dma_set_buff_addr( UINT8 channel, UINT32 lmem,      BOOL enable_dma );
VOID dma_set_buff_count( UINT8 channel, UINT16 count_l1, BOOL enable_dma );
VOID dma_disable( UINT8 channel );
VOID dma_enable( UINT8 channel );
VOID dma_sreq( UINT8 channel );
UINT16 dma_get_count_l1( UINT8 channel );
UINT32 dma_get_addr( UINT8 channel );
UINT8 dma_get_status( VOID );
BOOL dma_get_dmareq( UINT8 channel );
BOOL dma_get_status_dmareq( UINT8 status, UINT8 channel );
BOOL dma_get_tc( UINT8 channel );
BOOL dma_get_status_tc( UINT8 status, UINT8 channel );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
