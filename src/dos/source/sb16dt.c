/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SB16DT.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... DT2801A
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.3    24/08/94  Borja     adaptacion a nuevo blaster_env() dec/hex.
1.0.2    16/05/94  Borja     adaptacion a nueva interfaz LMEM.
1.0.1    15/05/94  Borja     adaptacion a nueva interfaz DMA.
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Mapea la libreria de funciones de la tarjeta DT2801A de
DataTranslation sobre la Sound Blaster 16.
No se han implementado todas las funciones!!!!!
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>

#include "tdef.h"
#include "lmem.h"
#include "dma.h"
#include "intrs.h"
#include "sb.h"
#include "blaster.h"

/**********************************************************/

PRIVATE BOOL  _sb16_initialized = FALSE;

PRIVATE UINT16 _sb16_baseport;
PRIVATE UINT16 _sb16_irq;
PRIVATE UINT16 _sb16_dma16;

PRIVATE UINT16 _rate = 8000;
PRIVATE UINT32 _lmem;  /* buffer circular en formato lineal */
PRIVATE UINT16 _nelem;  /* longitud del buffer circular (muestras 16 bits) */

// PRIVATE VOID INTERRUPT (* _sb16_oldint) ( ) = NULL;
PRIVATE IntrServiceFunc _sb16_oldint = NULL;

/**********************************************************/

void interrupt sb16_newint( void )
{
	sb_IRQ_ack(_sb16_baseport);   /* ack a lo bruto para DSP */
	PIC_SEND_EOI(_sb16_irq);  /* ack para PIC */
}

/**********************************************************/

short initialize( void )
{
	if (!_sb16_initialized) {
		/* lee configuracion de entorno BLASTER */
		_sb16_baseport = blaster_env(BLASTER_BASEPORT,
				BLASTER_DEFAULT_BASEPORT, BLASTER_DECHEX_BASEPORT);
		_sb16_irq = blaster_env(BLASTER_IRQ,
				BLASTER_DEFAULT_IRQ, BLASTER_DECHEX_IRQ);
		_sb16_dma16 = blaster_env(BLASTER_DMA16,
				BLASTER_DEFAULT_DMA16,BLASTER_DECHEX_DMA16);
	}

	/* resetea la tarjeta, -1 en caso de error ??????????? */
	if (sb_DSP_reset(_sb16_baseport))
		return -1;

	if (!_sb16_initialized) {
		_sb16_initialized = TRUE;

		/* programa rutina servicio interrupcion */
		_sb16_oldint = INT_GET_VEC(IRQ_INTN(_sb16_irq));
		INT_SET_VEC(IRQ_INTN(_sb16_irq),sb16_newint);

		/* conecta IRQ */
		PIC_ENABLE(_sb16_irq);
		PIC_SEND_EOI(_sb16_irq);
	}

	return 0;
}

/**********************************************************/

short select_board( short )
{
	if (!_sb16_initialized)
		return -1;

	return 0;
}

/**********************************************************/

short find_dma_length( unsigned short * buff, short * maxlen )
{
	if (!_sb16_initialized)
		return -1;

	if (DMA_IS_DMA16(_sb16_dma16))  /* DMA de 16 bits ? */
		*maxlen = (short)(fixlen((pfVOID)buff,0xFFFFL,128,2,2) >> 1);
	else
		*maxlen = (short)(fixlen((pfVOID)buff,0xFFFFL,64,1,2) >> 1);
	return 0;
}

/**********************************************************/

short terminate( void )
{
	if (!_sb16_initialized)
		return -1;

	sb_DSP_reset(_sb16_baseport);

	/* desconecta DMA */
	dma_disable(_sb16_dma16);

	PIC_DISABLE(_sb16_irq);
	INT_SET_VEC(IRQ_INTN(_sb16_irq),_sb16_oldint);

	_sb16_initialized = FALSE;

	return 0;
}

/**********************************************************/

short setup_adc( short , short , short , short )
{
	if (!_sb16_initialized)
		return -1;

	return 0;
}

/**********************************************************/

short begin_adc_dma( short len, unsigned short * buff )
{
	short tmp;

	if (!_sb16_initialized)
		return -1;

	/* comprueba longitud maxima posible */
	find_dma_length(buff, &tmp);
	_nelem = (len > tmp) ? tmp : len;   /* limita si es necesario */
	_lmem = PTR2LMEM(buff);
	if (!_nelem)  /* sal si no hay buffer */
		return -1;

	/* programa DMA */
	dma_set(_sb16_dma16, DMA_INCREMENT, DMA_SINGLE_CICLE, DMA_WRITE_TO_MEM,
			_lmem, DMA_IS_DMA16(_sb16_dma16) ? _nelem - 1 :
            ((UINT16)_nelem << 1) - 1, DMA_ENABLE);

	/* programa DSP */
	return sb_DSP_16(_sb16_baseport, SB_DSP_INPUT, SB_DSP_SINGLE_CICLE,
			SB_DSP_FIFO_ON, SB_DSP_MONO, SB_DSP_SIGNED,
            (UINT16)(_nelem - 1));

}

/**********************************************************/

short continuous_adc_dma( short len, unsigned short * buff )
{
	short tmp;

	if (!_sb16_initialized)
		return -1;

	/* comprueba longitud maxima posible */
	find_dma_length(buff, &tmp);
	_nelem = (len > tmp) ? tmp : len;   /* limita si es necesario */
	_lmem = PTR2LMEM(buff);
	if (!_nelem)  /* sal si no hay buffer */
		return -1;

	/* programa DMA */
	dma_set(_sb16_dma16, DMA_INCREMENT, DMA_AUTO_INIT, DMA_WRITE_TO_MEM,
			_lmem, DMA_IS_DMA16(_sb16_dma16) ? _nelem - 1 :
            ((UINT16)_nelem << 1) - 1, DMA_ENABLE);

	/* programa DSP */
	return sb_DSP_16(_sb16_baseport, SB_DSP_INPUT, SB_DSP_AUTO_INIT,
			SB_DSP_FIFO_ON, SB_DSP_MONO, SB_DSP_SIGNED,
            (UINT16)((_nelem >> 1) - 1));
}

/**********************************************************/

short test_adc_dma( short * left )
{
	if (!_sb16_initialized)
		return -1;

	/* lee DMA */
	*left = dma_get_count_l1(_sb16_dma16) + 1;

	if (! DMA_IS_DMA16(_sb16_dma16))
		*left >>= 1;

	return 0;
}

/**********************************************************/

short wait_adc_dma( unsigned short *  ptr )
{
    UINT16 left_l1, count_l1, ocount_l1;

	if (!_sb16_initialized)
		return -1;

    left_l1 = _nelem - (UINT16)((PTR2LMEM(ptr) - _lmem) >> 1);
	ocount_l1 = _nelem;
	if (! DMA_IS_DMA16(_sb16_dma16)) {
		left_l1 <<= 1;
		ocount_l1 <<= 1;
		}
	left_l1 -= 1;

	/* mientras estes por delante y no comienze ciclo nuevo */
	while (((count_l1 = dma_get_count_l1(_sb16_dma16)) >= left_l1) &&
			(ocount_l1 >= count_l1))
		ocount_l1 = count_l1;

	return 0;
}

/**********************************************************/

short stop_adc_dma( void )
{
	if (!_sb16_initialized)
		return -1;

	sb_DSP_16_pause(_sb16_baseport);
	sb_DSP_reset(_sb16_baseport);
	sb_DSP_out_rate(_sb16_baseport, _rate);
	sb_DSP_in_rate(_sb16_baseport, _rate);

	return 0;
}

/**********************************************************/

short setup_dac( short , short )
{
	if (!_sb16_initialized)
		return -1;

	return 0;
}
/**********************************************************/

short begin_dac_dma( short len, unsigned short * buff )
{
	short tmp;

	if (!_sb16_initialized)
		return -1;

	/* comprueba longitud maxima posible */
	find_dma_length(buff, &tmp);
	_nelem = (len > tmp) ? tmp : len;   /* limita si es necesario */
	_lmem = PTR2LMEM(buff);
	if (!_nelem)  /* sal si no hay buffer */
		return -1;

	/* programa DMA */
	dma_set(_sb16_dma16, DMA_INCREMENT, DMA_SINGLE_CICLE, DMA_READ_FROM_MEM,
			_lmem, DMA_IS_DMA16(_sb16_dma16) ? _nelem - 1 :
            ((UINT16)_nelem << 1) - 1, DMA_ENABLE);

	/* programa DSP */
	return sb_DSP_16(_sb16_baseport, SB_DSP_OUTPUT, SB_DSP_SINGLE_CICLE,
			SB_DSP_FIFO_ON, SB_DSP_MONO, SB_DSP_SIGNED,
            (UINT16)(_nelem - 1));
}

/**********************************************************/

short continuous_dac_dma( short len, unsigned short * buff )
{
	short tmp;

	if (!_sb16_initialized)
		return -1;

	/* comprueba longitud maxima posible */
	find_dma_length(buff, &tmp);
	_nelem = (len > tmp) ? tmp : len;   /* limita si es necesario */
	_lmem = PTR2LMEM(buff);
	if (!_nelem)  /* sal si no hay buffer */
		return -1;

	/* programa DMA */
	dma_set(_sb16_dma16, DMA_INCREMENT, DMA_AUTO_INIT, DMA_READ_FROM_MEM,
			_lmem, DMA_IS_DMA16(_sb16_dma16) ? _nelem - 1 :
            ((UINT16)_nelem << 1) - 1, DMA_ENABLE);

	/* programa DSP */
	return sb_DSP_16(_sb16_baseport, SB_DSP_OUTPUT, SB_DSP_AUTO_INIT,
			SB_DSP_FIFO_ON, SB_DSP_MONO, SB_DSP_SIGNED,
            (UINT16)((_nelem >> 1) - 1));
}

/**********************************************************/

short test_dac_dma( short * left )
{
	if (!_sb16_initialized)
		return -1;

	/* lee DMA */
	*left = dma_get_count_l1(_sb16_dma16) + 1;

	if (! DMA_IS_DMA16(_sb16_dma16))
		*left >>= 1;

	return 0;

}

/**********************************************************/

short wait_dac_dma( unsigned short * ptr )
{
    UINT16 left_l1, count_l1, ocount_l1;

	if (!_sb16_initialized)
		return -1;

    left_l1 = _nelem - (UINT16)((PTR2LMEM(ptr) - _lmem) >> 1);
	ocount_l1 = _nelem;
	if (! DMA_IS_DMA16(_sb16_dma16)) {
		left_l1 <<= 1;
		ocount_l1 <<= 1;
		}
	left_l1 -= 1;

	/* mientras estes por delante y no comienze ciclo nuevo */
	while (((count_l1 = dma_get_count_l1(_sb16_dma16)) >= left_l1) &&
			(ocount_l1 >= count_l1))
		ocount_l1 = count_l1;

	return 0;
}

/**********************************************************/

short stop_dac_dma( void )
{
	if (!_sb16_initialized)
		return -1;

	sb_DSP_16_pause(_sb16_baseport);
	sb_DSP_reset(_sb16_baseport);
	sb_DSP_out_rate(_sb16_baseport, _rate);
	sb_DSP_in_rate(_sb16_baseport, _rate);

	return 0;
}

/**********************************************************/

short set_clock_frequency( float * frate )
{
	if (!_sb16_initialized)
		return -1;

    _rate = (UINT16)(*frate + 0.5);

	sb_DSP_out_rate(_sb16_baseport, _rate);
	sb_DSP_in_rate(_sb16_baseport, _rate);

	return 0;
}

/**********************************************************/

short set_error_control_word( unsigned short )
{
	return 0;
}

/**********************************************************/

