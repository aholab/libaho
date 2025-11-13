/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XSNDSB8.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... SoundBlaster 16, DMA, IRQs...
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    23/05/96  Borja     bug: case WAIT3 sin break final
1.0.0    02/03/95  Borja     Codificacion inicial.

======================== Contenido ========================
Conversion AD/DA de 8 bits con Sound Blaster 16.
Ver XSNDSB16.C para mas informacion.
....................
8 bit AD/DA conversion using SB16.
Refer to XSNDSB16.C for detailled information, as the
function set xsnd8_???() is very similar to xsnd16_???().
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>

#include "tdef.h"
#include "xalloc.h"
#include "lmem.h"
#include "dma.h"
#include "dmabuff.h"
#include "intrs.h"
#include "sb.h"
#include "blaster.h"

#include "xsnd.h"

/**********************************************************/
/* numero maximo de bloques por segundo. Hay 2 interrupciones
por bloque!!! */

#define MAX_BLK_PER_SEC 1000;

/**********************************************************/

PRIVATE BOOL  _initialized = FALSE;

PRIVATE UINT16 _baseport;
PRIVATE UINT16 _irq;
PRIVATE UINT16 _dmach;

PRIVATE BOOL _record;
PRIVATE UINT16 _srate;
PRIVATE BOOL _stereo;

PRIVATE IntrServiceFunc _oldint;

PRIVATE UINT16 _nblk;      /* numero total de bloques */
PRIVATE UINT16 _dmablk;    /* bloque en que esta el DMA */
PRIVATE UINT16 _qblk;      /* bloques encolados */
PRIVATE UINT32 _blklen;    /* longitud de bloque */
PRIVATE UINT16 _qtail;     /* hay ultimo bloque encolado */
/*                            (0=no, 1=si, 2=si pero de long. 0) */
PRIVATE UINT16 _taillen;   /* longitud de ultimo bloque */
PRIVATE phINT8 _buffptr;   /* puntero a buffer DMA */
PRIVATE UINT32 _bufflmem;  /* puntero a buffer DMA (puntero plano) */
PRIVATE pfVOID _buffmem;   /* puntero a buffer completo reservado para DMA */
PRIVATE UINT16 _samplelen; /* bytes que ocupa una 'muestra' */
PRIVATE UINT16 _sblen;     /* longitud a dar a la SB16 */
PRIVATE BOOL _inusrproc;   /* TRUE cuando se esta en usrproc() */
PRIVATE UINT16 _overrun;   /* cuenta numero de usrproc() no llamados */

PRIVATE INT16 _status;
enum STATUS { AWAIT=1, WAIT1, WAIT1A, WAIT2, WAIT3,
		WAIT4, WAIT5, WAIT6, WAIT6A, BLOCK };   /* neg -> pausa */

PRIVATE VOID (PTRF _usrproc) ( BOOL last );

/**********************************************************/

PRIVATE VOID INTERRUPT _newint( VOID )
{
	BOOL up = FALSE;

	if ((sb_IRQ_stat(_baseport) & DSP_MASK_IRQ_DMA8MIDI) != 0) {
		switch (_status) {
		case WAIT1:
			if (_qblk>1) {
				sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
				_status = WAIT2;
			}
			else if (!_qtail) {  /* no hay nada encolado */
				_status = WAIT1A;
			}
			else if (_qtail==1) { /* ultimo bloque > 0 */
				sb_DSP_8(_baseport, _record, SB_DSP_SINGLE_CICLE,
						SB_DSP_FIFO_ON, _stereo, SB_DSP_UNSIGNED, _taillen);
				sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
				_status = WAIT3;
			}
			else {  /* ultimo bloque = 0 bytes */
				sb_DSP_8_exit(_baseport);
				sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
				_status = WAIT4;
			}
			break;
		case WAIT2:
			sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
			_qblk--;
			if ((++_dmablk)==_nblk)
				_dmablk = 0;
			_status = WAIT1;
            up = TRUE;
			break;
		case WAIT3:
			sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
			_qblk--;
			_status = WAIT4;
			up = TRUE;
			break;
		case WAIT4:
			sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
			_status = BLOCK;
			up = TRUE;
			break;
		case WAIT5:
			sb_DSP_8_exit(_baseport);
			sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
			_status = WAIT4;
			up = TRUE;
			break;
		case WAIT6:
			sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
			_status = WAIT6A;
			up = TRUE;
			break;
		case WAIT6A:
			sb_DSP_8_exit(_baseport);
			sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
			_status = WAIT4;
			break;
		}
		PIC_SEND_EOI(_irq);  /* ack para PIC */
		if (_usrproc && up) {
			if (_inusrproc)
				_overrun++;
			else {
				_inusrproc = TRUE;
				_usrproc(_status==BLOCK);
				_inusrproc = FALSE;
			}
		}
	}
	else
		_oldint();
}

/**********************************************************/

UINT16 xsnd8_reset( VOID )
{
	sb_DSP_8_pause(_baseport);

	_status = -AWAIT;
	_dmablk = 0;
	_qblk = (_record?_nblk:0);
	_qtail = 0;
	_overrun = 0;

	/* programa DMA */
	dma_set(_dmach, DMA_INCREMENT, DMA_AUTO_INIT,
			_record?DMA_WRITE_TO_MEM:DMA_READ_FROM_MEM,
            _bufflmem, (UINT16)(_samplelen*_nblk*_blklen - 1), DMA_ENABLE);

	return 0;
}

/**********************************************************/

UINT16 xsnd8_open( BOOL record, UINT16 srate, BOOL stereo,
        UINT32 blklen, UINT16 numblk, VOID (PTRF usrproc) ( BOOL last) )
{
	UINT32 maxlen, minlen, bufflen;
	UINT32 len, use;

	if (_initialized)   /* si ya esta inicializada, indica error */
		return 1;   /* ya inicializado */

	/* lee configuracion de entorno BLASTER */
	_baseport = blaster_env(BLASTER_BASEPORT,
			BLASTER_DEFAULT_BASEPORT, BLASTER_DECHEX_BASEPORT);
	_irq = blaster_env(BLASTER_IRQ,
			BLASTER_DEFAULT_IRQ, BLASTER_DECHEX_IRQ);
	_dmach = blaster_env(BLASTER_DMA8,
			BLASTER_DEFAULT_DMA8,BLASTER_DECHEX_DMA8);

	if (record) {
		if (sb_DSP_in_rate(_baseport,srate))
			return 1;  /* error escribiendo srate */
	}
	else
		if (sb_DSP_out_rate(_baseport,srate))
			return 1;  /* error escribiendo srate */

	_record = record;
	_stereo = stereo;
	_srate = srate;
	_blklen = blklen;
	_usrproc = usrproc;
	_inusrproc = FALSE;

	_samplelen = ( stereo ? 2 : 1 );
	maxlen = (1024L*64L)/_samplelen;
	minlen = _srate/MAX_BLK_PER_SEC;

	if (_blklen>maxlen)
		return 1;    /* bloque muy grande */
	if (_blklen<minlen)
		return 1;    /* bloque muy pequeno */
	if (_blklen<2)
		return 1;    /* bloque MUY pequeno */
	if (_blklen % 2)
		return 1;    /* bloque no multiplo de 2 */

    _sblen = (UINT16)((stereo?_blklen:_blklen/2)-1);
    _nblk = (UINT16)(numblk ? numblk : maxlen/_blklen);

	do {
		bufflen = _blklen*_nblk;
		if (bufflen > maxlen)
			return 1;    /* demasiados bloques */
		_buffmem = dmabuff_malloc(bufflen*_samplelen, 64, 1,
				blklen*_samplelen,&len);
		if (_buffmem==NULL) {  /* no se pudo reservar memoria */
			/* si no se fijo el n. bloques, reduce */
			if ((!numblk) && (_nblk>1))
				_nblk--;
			else
				return 1;  /* no hay memoria para DMA */
		}
	} while (_buffmem==NULL);

	_buffptr = (pfINT8)fixmem(_buffmem, len, 64, 1,
			blklen*_samplelen, &_bufflmem, &use);

    xsnd8_reset();
	/* programa rutina servicio interrupcion */
	_oldint = INT_GET_VEC(IRQ_INTN(_irq));
	INT_SET_VEC(IRQ_INTN(_irq),_newint);
	PIC_ENABLE(_irq);  	/* conecta IRQ */
	PIC_SEND_EOI(_irq);

	_initialized = TRUE;

	return 0;  /* no hay error */
}

/**********************************************************/

UINT16 xsnd8_close( VOID )
{
	if (!_initialized)
		return 1;  /* ni siquiera estaba abierto */

	sb_DSP_8_pause(_baseport);

	/* desconecta DMA */
	dma_disable(_dmach);

/*  $$ nada de desconectar, que puede haber otros */
/*
	PIC_DISABLE(_irq);
*/
	INT_SET_VEC(IRQ_INTN(_irq),_oldint);

	xfree(_buffmem);

	_initialized = FALSE;

	return 0;
}

/**********************************************************/

UINT16 xsnd8_start( VOID )
{
	if (_status>AWAIT)
		return 1; /* ya esta en marcha */
	if (_status<0)
		_status = -_status;

	if (_status>AWAIT)      /* estaba en pausa */
		sb_DSP_8_continue(_baseport);
	else if (_qblk) {
		_status=WAIT1;
		sb_DSP_8(_baseport, _record, SB_DSP_AUTO_INIT,
				SB_DSP_FIFO_ON, _stereo, SB_DSP_UNSIGNED, _sblen);
	}
	else if (_qtail==1) {  /* ultimo blk > 0 */
		_status=WAIT4;
		sb_DSP_8(_baseport, _record, SB_DSP_SINGLE_CICLE,
				SB_DSP_FIFO_ON, _stereo, SB_DSP_UNSIGNED, _taillen);
	}
	else if (_qtail==2) {   /* ultimo blk = 0 */
		_status=BLOCK;
	}

	return 0;
}

/**********************************************************/

UINT16 xsnd8_stop( VOID )
{
	sb_DSP_8_pause(_baseport);
	if (_status<0)
		return 1;  /* ya estaba en pausa */

	_status = -_status;
	return 0;
}

/**********************************************************/

UINT16 xsnd8_getoverrun( VOID )
{
    UINT16 or;

	INT_DISABLE();
	or = _overrun;
	_overrun = 0;
	INT_ENABLE();

	return or;
}

/**********************************************************/

pfUINT8 xsnd8_getblknum( UINT16 nblk )
{
	return (pfUINT8)(_buffptr + (_samplelen*_blklen*nblk));
}

/**********************************************************/

UINT16 xsnd8_getnblk( VOID )
{
	return _nblk;
}

/**********************************************************/

pfUINT8 xsnd8_getblk( VOID )
{
    UINT16 n;

	if ((_qblk==_nblk)||(_qtail))   /* si no quedan bloques */
		return NULL;

	INT_DISABLE();
	n = _dmablk+_qblk;
	INT_ENABLE();

	return (pfUINT8)(_buffptr + ((n % _nblk)*_blklen*_samplelen));
}

/**********************************************************/

UINT16 xsnd8_addblk( VOID )
{
	if ((_qblk==_nblk)||(_qtail))  /* si no quedan bloques */
		return 1;

	INT_DISABLE();
	_qblk++;
	if (_status==AWAIT)
        xsnd8_start();
	else if (abs(_status)==WAIT1A) {
		_status = (_status<0)?-WAIT2:WAIT2;
		sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
	}
	INT_ENABLE();

	return 0;
}

/**********************************************************/

UINT16 xsnd8_addlastblk( UINT32 len )
{
    UINT16 i, n;
	pfVOID p;
	UINT8 u8;
    UINT16 u16;

	if ((_qblk==_nblk)||(_qtail))  /* si no quedan bloques */
		return 1;

	INT_DISABLE();
	_qtail = (len>0)?1:2;
    _taillen = (UINT16)(len-1);
	if (_status==AWAIT)
        xsnd8_start();
	else if (abs(_status)==WAIT1A) {
		sb_IRQ_ack_dma8midi(_baseport);  /* SB ACK */
		if (len<=_blklen/2) {  /* 1 semi bloque */
			_status = (_status<0)?-WAIT5:WAIT5;
			if (_record) {
				len=_blklen/2;
                n = (UINT16)(_blklen-1);
			}
			else
                n = (UINT16)(_blklen/2-1);
			p = _buffptr + (((_dmablk+_qblk) % _nblk) * _blklen*_samplelen);
			if (_stereo) {
				if (_record)
					u16 = 0x8080;
				else if (len)
                    u16 = ((pfUINT16)p)[(UINT16)(len-1)];
				else
                    u16 = ((pfUINT16)(_buffptr+(((_dmablk+_qblk-1)%_nblk)*
                            _blklen*_samplelen)))[(UINT16)(_blklen-1)];
                for (i=(UINT16)len; i<=n; i++)
                    ((pfUINT16)p)[i]=u16;
			}
			else {  /* mono */
				if (_record)
					u8 = 0x80;
				else if (len)
                    u8 = ((pfUINT8)p)[(UINT16)(len-1)];
				else
					u8 = ((pfUINT8)(_buffptr+(((_dmablk+_qblk-1)%_nblk)*
                            _blklen*_samplelen)))[(UINT16)(_blklen-1)];
                for (i=(UINT16)len; i<=n; i++)
					((pfUINT8)p)[i]=u8;
			}
		}
		else {  /* 2 semi bloques */
			_status = (_status<0)?-WAIT6:WAIT6;
			if ((!_record)&&(len<_blklen)) {
                n = (UINT16)(_blklen-1);
				p = _buffptr+(((_dmablk+_qblk) % _nblk)*_blklen*_samplelen);
				if (_stereo) {
                    u16 = ((pfUINT16)p)[(UINT16)(len-1)];
                    for (i=(UINT16)len; i<=n; i++)
                        ((pfUINT16)p)[i]=u16;
				}
				else {  /* mono */
                    u8 = ((pfUINT8)p)[(UINT16)(len-1)];
                    for (i=(UINT16)len; i<=n; i++)
						((pfUINT8)p)[i]=u8;
				}
			}
		}
	}
	INT_ENABLE();

	return 0;
}

/**********************************************************/

BOOL xsnd8_stopped( VOID )
{
	return (_status<0);
}

/**********************************************************/

BOOL xsnd8_blocked( VOID )
{
	return (_status==BLOCK);
}

/**********************************************************/

UINT16 xsnd8_srate( UINT16 srate )
{
	if (!_initialized)
		return 1;  /* ni siquiera estaba abierto */

	_srate = srate;
	if (_record) {
		if (sb_DSP_in_rate(_baseport,srate))
			return 1;  /* error escribiendo srate */
	}
	else
		if (sb_DSP_out_rate(_baseport,srate))
			return 1;  /* error escribiendo srate */

	return 0;
}

/**********************************************************/

DOUBLE xsnd8_getsrate( VOID )
{
	return sb_DSP_tc2dsr(sb_DSP_sr2tc(_srate,(_stereo?2:1)), (_stereo?2:1));
}

/**********************************************************/

UINT16 xsnd8_initialize( VOID )
{
	return sb_DSP_reset( blaster_env(BLASTER_BASEPORT,
			BLASTER_DEFAULT_BASEPORT, BLASTER_DECHEX_BASEPORT));
}

/**********************************************************/

UINT16 xsnd8_getqblk( VOID )
{
	if (_status==BLOCK) return 0;
	return _qblk+(_qtail?1:0);
}

/**********************************************************/
