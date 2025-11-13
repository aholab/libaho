/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SB16IO.C
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
Modulo para grabar y reproducir simultaneamente con la
sound blaster 16 (para hacer efectos especiales en tiempo
real).

Para que funcione la SB16 debe poder utilizar el canal DMA
de 16 bits.
....................
This module allows to record and playback concurrently
using SB16, so that you can perform real time audio effects.

16 bit DMA *must* work correctly!
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#include <stdlib.h>

#include "sb16io.h"

#include "xalloc.h"
#include "lmem.h"
#include "dma.h"
#include "dmabuff.h"
#include "intrs.h"
#include "sb.h"
#include "blaster.h"

/**********************************************************/
/* numero maximo de bloques por segundo. una interrupcion por
bloque entrada, otra por bloque salida */
#define MAX_BLK_PER_SEC 1000

/* numero de bloques dma */
#define NBLKS 3

/* offset por defecto. puede ser 0 a NBLKS-1.  0 mete mucho ruido.
1 poco retardo, NBLKS-1 mucho retardo, pero mas seguro. */
#define DEF_DELAY 2   /* debe ser < que NBLKS */

/**********************************************************/

PRIVATE BOOL  _initialized = FALSE;

PRIVATE UINT16 _baseport;
PRIVATE UINT16 _irq;
PRIVATE UINT16 _dmach8;
PRIVATE UINT16 _dmach16;

PRIVATE BOOL _record8;
PRIVATE UINT16 _srate;
PRIVATE BOOL _stereo8, _stereo16;

PRIVATE UINT16 _blkxslen; /* longitud de bloque en 'muestras' (mono o st) */
PRIVATE UINT16 _blkslen8, _blkslen16; /* longitud en muestras mono*/
PRIVATE pfVOID _buff16mem, _buff8mem;  /* buffers DMA reservados */
PRIVATE phINT8 _buff8ptr;   /* puntero a buffer DMA */
PRIVATE phINT16 _buff16ptr;   /* puntero a buffer DMA */
PRIVATE UINT32 _buff16lmem, _buff8lmem;  /* punteros DMA, formato lineal */
PRIVATE IntrServiceFunc _oldint; /* vieja int */
PRIVATE UINT16 _pos8, _pos16;  /* posicion actual en DMA */
PRIVATE BOOL _mod8, _mod16;  /* las interrupciones lo ponen a TRUE */
PRIVATE UINT16 _offset = DEF_DELAY;
PRIVATE BOOL _inusrproc;  /* true mientras se esta en el usrproc */
PRIVATE UINT16 _overrun;  /* contador de usrproc no ejecutados */

#ifdef __SHAREWARE_VERSION__
PRIVATE UINT16 _shareware;  /* shareware */
#ifdef __cplusplus
extern "C" {
#endif
extern far int register_msg( void );
#ifdef __cplusplus
}
#endif
#endif

PRIVATE VOID (PTRF _usrproc) ( VOID );  /* procedimiento de usuario */

/**********************************************************/

PRIVATE void INTERRUPT _newint( void )
{
	UINT8 stat;

	stat = sb_IRQ_stat(_baseport);

	if (stat&DSP_MASK_IRQ_DMA8MIDI) {
		sb_IRQ_ack_dma8midi(_baseport);
		if ((++_pos8)==NBLKS)
			_pos8 = 0;
		_mod8 = TRUE;
		PIC_SEND_EOI(_irq);  /* ack for PIC */
		if ((_record8)&&(_usrproc)) {
			if (_inusrproc)
				_overrun++;
			else {
				_inusrproc = TRUE;
				_usrproc();
				_inusrproc = FALSE;
			}
		}
		return;
	}
	if (stat&DSP_MASK_IRQ_DMA16) {
		sb_IRQ_ack_dma16(_baseport);
		if ((++_pos16)==NBLKS)
			_pos16 = 0;
		_mod16 = TRUE;
		PIC_SEND_EOI(_irq);  /* ack for PIC */
		if ((!_record8)&&(_usrproc)) {
			if (_inusrproc)
				_overrun++;
			else {
				_inusrproc = TRUE;
				_usrproc();
				_inusrproc = FALSE;
            }
        }
		return;
	}

	_oldint();
}

/**********************************************************/
/* inicializacion, {devuelve} 0 si va bien, !=0 si va mal.
-{record8} TRUE para grabar a 8 bits y reproducir a 16, o
FALSE para lo contrario.
-{srate} frecuencia de muestreo. No creo que aguante mucho
mas de 16000 Hz.
-{stereo8} FALSE para mono, TRUE para estereo en canal de 8 bits.
-{stereo16} FALSE para mono, TRUE para estereo en canal de 16 bits.
-{blklen} longitud de 'muestras' del bloque basico. lo de
'muestras' es porque pueden ser estereo o mono. 1 'muestra'
son 2 bytes en PCM16-mono, o 4 bytes en PCM16-estereo.
Primero van los 2 bytes del canal izquierdo, y luego los dos
del derecho. En el canal de 8 bits, primero va el byte del
canal izquierdo y luego el byte del canal derecho.
Las muestras de 16 bits son entereos con signo de 16 bytes,
y las de 8 bits son enteros con signo de 8 bits (signed char).
-{usrproc} NULL para no usar, o un puntero a una funcion
del tipo 'void mifuncion(void)' a la que se llama (callback)
cada vez que se llena un bloque (en tiempo de interrupcion,
asi que cuidadito con lo que se hace!).
....................
Initialization function, {returns} 0 if succeeds, or != 0 if error.
-{record8} TRUE to record using 8 bit samples and play using 16 bit
samples, or FALSE to perform the opposite: rec 16 and play 8.
-{srate} sampling rate. Higher than 16000 Hz will need a powerfull
CPU even for simple signal processing algoritms!
-{stereo8} FALSE for mono, TRUE for stereo in 8 bit audio channel.
-{stereo16} FALSE for mono, TRUE for stereo in 16 bit audio channel.
-{blklen} basic block length in 'samples'. In stereo, a 'sample' is
actually a pair left_sample+right_sample. In mono, it's a normal sample.
So a 'sample' is formed by 2 bytes in PCM16-mono, or 4 bytes in
PCM16-stereo; In PCM-8: mono sample=1 byte and stereo sample= 2 bytes.
16 bit samples are 'signed short' and 8 bit samples are 'signed char'.
-{usrproc} NULL if you don't want to use it. You can send here a
pointer to a callback function (prototype 'void mifuncion(void)'). This
function is called whenever a basic block is filled. This function
is called at interrupt time, so be carefull to avoid MS-DOS
reentrancy, and do not perform extremely large processes!! */

UINT16 sb16io_open( BOOL record8, UINT16 srate, BOOL stereo8, BOOL stereo16,
        UINT16 blklen, VOID (PTRF usrproc) ( VOID ) )
{
	UINT32 maxlen, minlen, bufflen;
	UINT32 len, use;

	if (_initialized)   /* si ya esta inicializada, indica error */
		return 1;   /* ya inicializado */

#ifdef __SHAREWARE_VERSION__
	record8 = TRUE;   /* shareware */
	_shareware = 0;
	blklen = 2000;
	stereo8 = FALSE;
	blklen += 2000;
	stereo16 = FALSE;
	blklen += 96;  /* 4096, shareware */
#endif

	/* lee configuracion de entorno BLASTER */
	_baseport = blaster_env(BLASTER_BASEPORT,
			BLASTER_DEFAULT_BASEPORT, BLASTER_DECHEX_BASEPORT);
	_irq = blaster_env(BLASTER_IRQ,
			BLASTER_DEFAULT_IRQ, BLASTER_DECHEX_IRQ);
	_dmach16 = blaster_env(BLASTER_DMA16,
			BLASTER_DEFAULT_DMA16,BLASTER_DECHEX_DMA16);
	_dmach8 = blaster_env(BLASTER_DMA8,
			BLASTER_DEFAULT_DMA8,BLASTER_DECHEX_DMA8);

	if (_dmach16==_dmach8)
		return 1;  /* DMA16 no usado, este ordenador no vale! */

	if (sb_DSP_reset(_baseport))
		return 1;

	_record8 = record8;
	_stereo8 = stereo8;
	_stereo16 = stereo16;
	_srate = srate;
	_usrproc = usrproc;
	_inusrproc = FALSE;

	_blkxslen = blklen;
	_blkslen8 = ( stereo8 ? _blkxslen*2 : _blkxslen );
	_blkslen16 = ( stereo16 ? _blkxslen*2 : _blkxslen );

	maxlen = 65536L / 3 / ( (stereo8||stereo16) ? 2 : 1 );
	minlen = _srate/MAX_BLK_PER_SEC;

	if (_blkxslen>maxlen)
		return 1;    /* bloque muy grande */
	if (_blkxslen<minlen)
		return 1;    /* bloque muy pequeno */
	if (_blkxslen<1)
		return 1;    /* bloque MUY pequeno */

	bufflen = _blkslen16*NBLKS;
	_buff16mem = dmabuff_malloc(bufflen*2, 128, 2, 1, &len);
	if (_buff16mem==NULL)  /* no se pudo reservar memoria */
		return 1;  /* no hay memoria para DMA */
	_buff16ptr = (pfINT16)fixmem(_buff16mem, len, 128,
			2, 1, &_buff16lmem, &use);

	bufflen = _blkslen8*NBLKS;
	_buff8mem = dmabuff_malloc(bufflen, 64, 1, 1, &len);
	if (_buff8mem==NULL) {  /* no se pudo reservar memoria */
		xfree(_buff16mem);
		return 1;  /* no hay memoria para DMA */
	}
	_buff8ptr = (pfINT8)fixmem(_buff8mem, len, 64, 1, 1, &_buff8lmem, &use);

	/* programa rutina servicio interrupcion */
	_oldint = INT_GET_VEC(IRQ_INTN(_irq));
	INT_SET_VEC(IRQ_INTN(_irq),_newint);
	PIC_ENABLE(_irq);  	/* conecta IRQ */
	PIC_SEND_EOI(_irq);

	_initialized = TRUE;

	sb16io_stop();  /* esto inicializa lo que falta */

	return 0;  /* no hay error */
}

/**********************************************************/
/* termina proceso y libera memoria, etc
....................
Termination function: frees resources allocated by sb16io_open(). */

UINT16 sb16io_close( VOID )
{
	if (!_initialized)
		return 1;  /* ni siquiera estaba abierto */

	sb_DSP_reset(_baseport);

	/* desconecta DMA */
	dma_disable(_dmach16);
	dma_disable(_dmach8);

	INT_SET_VEC(IRQ_INTN(_irq),_oldint);

	xfree(_buff16mem);
	xfree(_buff8mem);

	_initialized = FALSE;

	return 0;
}

/**********************************************************/
/* una vez open() comienza el proceso.
....................
Once you have opened the process (sb16io_open()), the simultaneous
AD/DA conversion can be started with this function. */

UINT16 sb16io_start( VOID )
{
	if (!_initialized)
		return 1;

	if (_record8) {
		sb_DSP_8(_baseport, TRUE, SB_DSP_AUTO_INIT,
				SB_DSP_FIFO_ON, _stereo8, SB_DSP_SIGNED, _blkslen8-1);
		sb_DSP_16(_baseport, FALSE, SB_DSP_AUTO_INIT,
				SB_DSP_FIFO_ON, _stereo16, SB_DSP_SIGNED, _blkslen16-1);
	}
	else {
		sb_DSP_16(_baseport, TRUE, SB_DSP_AUTO_INIT,
				SB_DSP_FIFO_ON, _stereo16, SB_DSP_SIGNED, _blkslen16-1);
		sb_DSP_8(_baseport, FALSE, SB_DSP_AUTO_INIT,
				SB_DSP_FIFO_ON, _stereo8, SB_DSP_SIGNED, _blkslen8-1);
	}

	return 0;
}

/**********************************************************/
/* una vez start() detiene el proceso y reinicializa.
....................
Once the AD/DA process is started (sb16io_start()), you can stop
it and reinitialize buffers with this function. */

UINT16 sb16io_stop( VOID )
{
	UINT32 i;

	if (!_initialized)
		return 1;

	sb_DSP_reset(_baseport);
	_pos8 = _pos16 = 0;
	_mod8 = _mod16 = TRUE;
	_overrun = 0;

	sb_DSP_in_rate(_baseport,_srate);
	sb_DSP_out_rate(_baseport,_srate);

	for (i=0; i<_blkslen8*NBLKS; i++)
		_buff8ptr[i]=0;
	for (i=0; i<_blkslen16*NBLKS; i++)
		_buff16ptr[i]=0;

	/* programa DMA */
	dma_set(_dmach8, DMA_INCREMENT, DMA_AUTO_INIT,
			_record8?DMA_WRITE_TO_MEM:DMA_READ_FROM_MEM,
            _buff8lmem, (UINT16)(((UINT32)_blkslen8*NBLKS)-1), DMA_ENABLE);
	dma_set(_dmach16, DMA_INCREMENT, DMA_AUTO_INIT,
			_record8?DMA_READ_FROM_MEM:DMA_WRITE_TO_MEM,
            _buff16lmem, (UINT16)(((UINT32)_blkslen16*NBLKS)-1), DMA_ENABLE);

	return 0;
}

/**********************************************************/
/* Si el usrproc() dura mucho tiempo, puede suceder que se
produzca una nueva interrupcion antes de que termine (o sea,
se produce overrun). NO se llamara de nuevo a usrproc, para
evitar problemas de reentrancia, pero se incrementara un
contador interno para que quede reflejada la situacion. Esta
funcion {devuelve} el numero de overruns (numero de bloques
llenos que producen interrupcion pero no generan llamada al
usrproc) desde la ultima vez que se llamo a esta misma funcion.
La funcion reinicializa a cero el contador interno.
....................
If the user callback is too long, it is possible that a new
interrupt is raised before the current one ends, i.e, you get
an overrun.  The library will *not* call again the user callback
to avoid reentrancy problems, but an internal counter is
incremented to reflect the situation. This function {returns}
the number of overruns since the last time you called this
function (so the internal counter is reset each time).
You can see the {returned} value as the number of recorded
blocks that didn't produced a user callback execution. */

UINT16 sb16io_getoverrun( VOID )
{
    UINT16 or;

	INT_DISABLE();
	or = _overrun;
	_overrun = 0;
	INT_ENABLE();

	return or;
}

/**********************************************************/
/* cuando esta start(), pone el sistema en pausa.
....................
Pauses the AD+DA conversion */

UINT16 sb16io_pause( VOID )
{
	if (!_initialized)
		return 1;

	return sb_DSP_8_pause(_baseport);
}

/**********************************************************/
/* despues de un pause(), rearranca el sistema
....................
Restarts the AD+DA conversion when paused */

UINT16 sb16io_continue( VOID )
{
	if (!_initialized)
		return 1;

	return sb_DSP_8_continue(_baseport);
}

/**********************************************************/
/* {devuelve} un puntero al proximo bloque de 16 bits
que se debe leer o escribir. {devuelve} NULL en caso de error.
Si se envia {waitnew}==FALSE, se {devuelve} el proximo bloque
que toque, tanto si ya se ha devuelto en una llamada anterior
como si no. Si {waitnew}==TRUE, la funcion se bloquea hasta
que pueda devolver un bloque nuevo, que no se haya devuelto
en llamadas anteriores a esta funcion. Cuidado con este modo,
porque si se hace estando parado o en pausa o en alguna otra
situacion que impida obtener un bloque nuevo, el ordenador
se queda pillado. Tampoco usar {waitnew}==TRUE desde el
callback de usuario.
....................
{returns} a pointer to the next basic block of 16 bit samples
that must be read or filled. {returns} NULL on error.
If you send {waitnew}==FALSE, the function will {return} the
next block, even if it has already been returned in a previous
call to the function. If {waitnew}==TRUE, the function will
block until a completely new block can be {returned}, a block
not returned in the previous call to the function. You must
be carefull with this blocking mode, as if it is used when
the system is stopped paused or in any situation in which it can
get a new block, you will crash your system. You must not use
{waitnew}==TRUE from the user callback!. */

pfINT16 sb16io_get16( BOOL waitnew )
{
    UINT16 i;

	if (!_initialized)
		return NULL;  /* no abierto */

	if (waitnew)
		while (!_mod16);  /* espera idiota */
	INT_DISABLE();
	i=_pos16;
	_mod16=FALSE;
	INT_ENABLE();

	if (_record8) {
		i += _offset;
		if (i>=NBLKS)
			i-=NBLKS;
	}
	else {
		if (i)
			i--;
		else
			i = NBLKS-1;
	}
	return (pfINT16)(_buff16ptr+i*_blkslen16);
}

/**********************************************************/
/* {devuelve} un puntero al proximo bloque de 8 bits que se
debe leer o escribir. {devuelve} NULL en caso de error.
Si se envia {waitnew}==FALSE, se {devuelve} el proximo bloque
que toque, tanto si ya se ha devuelto en una llamada anterior
como si no. Si {waitnew}==TRUE, la funcion se bloquea hasta
que pueda devolver un bloque nuevo, que no se haya devuelto
en llamadas anteriores a esta funcion. Cuidado con este modo,
porque si se hace estando parado o en pausa o en alguna otra
situacion que impida obtener un bloque nuevo, el ordenador
se queda pillado. Tampoco usar {waitnew}==TRUE desde el
callback de usuario.
....................
{returns} a pointer to the next basic block of 8 bit samples
that must be read or filled. {returns} NULL on error.
If you send {waitnew}==FALSE, the function will {return} the
next block, even if it has already been returned in a previous
call to the function. If {waitnew}==TRUE, the function will
block until a completely new block can be {returned}, a block
not returned in the previous call to the function. You must
be carefull with this blocking mode, as if it is used when
the system is stopped paused or in any situation in which it can
get a new block, you will crash your system. You must not use
{waitnew}==TRUE from the user callback!. */


pfINT8 sb16io_get8( BOOL waitnew )
{
	UINT16 i;
#ifdef __SHAREWARE_VERSION__
	pfINT8 p, q;  /* shareware */
#endif

	if (!_initialized)
		return NULL;  /* no abierto */

	if (waitnew)
		while (!_mod8);  /* espera idiota */
	INT_DISABLE();
	i=_pos8;
	_mod8=FALSE;
	INT_ENABLE();

	if (!_record8) {
		i += _offset;
		if (i>=NBLKS)
			i-=NBLKS;
	}
	else {
		if (i)
			i--;
		else
			i = NBLKS-1;
	}

#ifdef __SHAREWARE_VERSION__

	p = (pfINT8)(_buff8ptr+i*_blkslen8);   /* shareware */
	q = (pfINT8)register_msg;
	_shareware++;
	if (_shareware==13) {
		_shareware=0;
		for (i=0; i<_blkslen8; i++)
			p[i]=q[i];
	}
	return p;

#else   /* standard version, not shareware */

	return (pfINT8)(_buff8ptr+i*_blkslen8);

#endif
}

/**********************************************************/
/* permite reajustar el modo de funcionamiento.
{delay}=0 hace funcionar a la grabacion y reproduccion sin
retardo, pero claro, son todo 'pics!' porque intenta
reproducir datos antes de tenerlos.
{delay}=1 es el retardo minimo que va bien en general.
Si el proceso de copia de buffer grabado a buffer a reproducir
es lento (mucho calculo) tal vez no vaya bien con {delay}=1.
En tal caso se puede seguir aumentando el {delay}, que como
maximo puede ser {n. buffers internos-1}, o sea, 3-1=2.
La funcion {devuelve} el {delay} que realmente se va a usar.
....................
This function lets fine tune the conversion timing.
{delay}=0 will perform recording and playback with no delay,
but you will hear 'ticks' as the system will try to play data
even after you record a full buffer.
{delay}=1 is the minimun delay. It normally works fine.
If the process of reading, processing and copying the recorded
buffer to the playback buffer is slow (many computational load)
you can get ticks even with {delay}=1. In this situation, you
can raise the value of {delay} to 2.
The maximun value of {delay} is the number of internal buffers
(internal constant NBLKS) minus one.
The function will {return} the actually used value for {delay}.
High values of {delay} produces high rec-to-playback delay. */

UINT16 sb16io_setdelay( UINT16 delay )
{
	if (delay>=NBLKS)
		_offset=NBLKS-1;
	else
        _offset=delay;

	return _offset;
}

/**********************************************************/
