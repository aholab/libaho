/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XSNDSB16.C
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
Conversion AD/DA de 16 bits con Sound Blaster 16.

Metodo habitual de funcionamiento:

REPRODUCCION:

1. initialize()
2. open()
3. start()
4. Repetir getblk(), rellenar bloque, addblk() mientras
se tengan bloques.
5. Ultimo mini-bloque: getblk(), rellenar bloque, addlastblk()
6. Esperar a que termine la reproduccion: blocked() == TRUE
7. close()

Opcionalmente se pueden pre-encolar buffers antes del start().

GRABACION:

1. initialize()
2. open()
3. preencolar todos los bloques con addblk()
4. start()
5. Repetir	getblk(), salvar bloque, addblk() mientras
se tengan ganas.
6. close()

Se puede hacer un stop() antes del close, pero no hace falta.
Normalmente no es necesario usar addlastblk() en grabacion:
uno deja de grabar cuando quiere y coge las muestras que
quiera del ultimo bloque que le interese.
....................
AD/DA 16 bit conversion using Sound Blaster 16

Usual programming sequence:

PLAYING SOUNDS:

1. initialize()
2. open()
3. start()
4. repeat getblk(), fill the block, addblk() while you have
audio blocks to play.
5. Last mini-block: getblk(), fill block, addlastblk()
6. Wait until playback is finisned: blocked() == TRUE
7. close()

You can also pre-queue audio buffers before you start() the playback.

RECORDING SOUNDS:

1. initialize()
2. open()
3. queue all the available blocks using addblk()
4. start()
5. repeat getblk(), save block, addblk(), while you want.
6. close()

You can send an stop() before close(), but it's not necessary.
It's also not necessary to use addlastblk() in recording tasks,
as you can stop recording whenever you want, keep as many samples
as desired, and throw away the rest of them.
=========================================================== */
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
por bloque!!!
....................
Max. number of blocks to process per second. Each block triggers
two interrups!! */

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
PRIVATE UINT16 _rectoread;  /* buffer pendientes de leer (en grabacion) */

PRIVATE INT16 _status;

enum STATUS { AWAIT=1, WAIT1, WAIT1A, WAIT2, WAIT3,
		WAIT4, WAIT5, WAIT6, WAIT6A, BLOCK };   /* neg -> pausa */

PRIVATE VOID (PTRF _usrproc) ( BOOL last );

/**********************************************************/

PRIVATE VOID INTERRUPT _newint( VOID )
{
	BOOL up = FALSE;

	if ((sb_IRQ_stat(_baseport) & DSP_MASK_IRQ_DMA16) != 0) {
		switch (_status) {
		case WAIT1:
			if (_qblk>1) {
				sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
				_status = WAIT2;
			}
			else if (!_qtail) {  /* no hay nada encolado */
				_status = WAIT1A;
			}
			else if (_qtail==1) { /* ultimo bloque > 0 */
				sb_DSP_16(_baseport, _record, SB_DSP_SINGLE_CICLE,
						SB_DSP_FIFO_ON, _stereo, SB_DSP_SIGNED, _taillen);
				sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
				_status = WAIT3;
			}
			else {  /* ultimo bloque = 0 bytes */
				sb_DSP_16_exit(_baseport);
				sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
				_status = WAIT4;
			}
			break;
		case WAIT2:
			sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
			_qblk--;
			if ((++_dmablk)==_nblk)
				_dmablk = 0;
			_status = WAIT1;
			up = TRUE;
			break;
		case WAIT3:
			sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
			_qblk--;
			_status = WAIT4;
			up = TRUE;
			break;
		case WAIT4:
			sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
			_status = BLOCK;
			up = TRUE;
			break;
		case WAIT5:
			sb_DSP_16_exit(_baseport);
			sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
			_status = WAIT4;
			up = TRUE;
			break;
		case WAIT6:
			sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
			_status = WAIT6A;
			up = TRUE;
			break;
		case WAIT6A:
			sb_DSP_16_exit(_baseport);
			sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
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
/* Reset de estado. Detiene la conversion (si esta en marcha)
y reinicializa todo de nuevo.  {devuelve} !=0 en caso de error.
....................
Reset status. This function stops the conversion process (if
active) and initializes the system. {returs}!=0 if error. */

UINT16 xsnd16_reset( VOID )
{
	sb_DSP_16_pause(_baseport);

	_status = -AWAIT;
	_dmablk = 0;
	_qblk = 0; //(_record?_nblk:0);
	_qtail = 0;
	_overrun = 0;
	_rectoread = 0;

	/* programa DMA */
	dma_set(_dmach, DMA_INCREMENT, DMA_AUTO_INIT,
			_record?DMA_WRITE_TO_MEM:DMA_READ_FROM_MEM,
            _bufflmem, (UINT16)((DMA_IS_DMA16(_dmach)?
			_samplelen/2:_samplelen)*_nblk*_blklen - 1), DMA_ENABLE);

	return 0;
}

/**********************************************************/
/* Prepara el modulo de sonido.
{record} es TRUE para grabar o FALSE para reproducir (alias
XSND_RECORD y XSND_PLAY).
{srate} es la frecuencia de muestreo en Hz (5000 a 44100).
{stereo} es FALSE para mono, TRUE para estereo (alias XSND_MONO
y XSND_STEREO).
{blklen} es la longitud de la unidad de conversion (en muestras
de 16bits para mono, o 'muestras' de 32bits para estereo). El
sistema graba/reproduce a golpe de esta unidad. Si es muy corta,
se sobrecarga al sistema. Si es muy larga, es poco 'agil'. Bloques
que duren 1/10 de segundo estan bien (por ejemplo, 800 muestras
a 8kHz).  Tiene que ser un valor par de muestras!
{numblk} es el numero de bloques de longitud {blklen} que el sistema
puede manejar simultaneamente. Menos de 3 puede producir 'clics' en
el sonido. Cuantos mas se pongan, mas seguro, pero tampoco es
necesario un numero excesivo, que ocupa memoria. 4 va bien.
Tambien se puede usar {numblk}=0, que se gestiona de manera especial,
reservando tantos bloques como permita la memoria disponible.
{usrproc} es un puntero a un call-back de usuario que recibe un
parametro BOOL {last}. Este call-back es llamado cada vez que el
sistema llena/vacia un bloque de muestras. Cuando se llena/vacia
el ultimo bloque de muestras, el parametro {last} vale TRUE. El
resto del tiempo, vale FALSE. Si se envia {usrproc}=NULL, entonces
no se llama al call-back de usuario.
Esta funcion {devuelve}!=0 en caso de error (parametros no validos,
no hay memoria, etc).
....................
Configure the audio control module.
{record} is TRUE to record, or FALSE to playback (you can use
the defines XSND_RECORD y XSND_PLAY).
{srate} is the sampling rate in Hz (5000 a 44100).
{stereo} is FALSE for mono, TRUE for stereo (alias XSND_MONO
y XSND_STEREO).
{blklen} is the basic block length (as 16 bits samples for mono,
or 32 bits 'samples' for stereo). The system will play/record
blocks of this size. If this length is too short, there is quite
a lot system overhead. If this length is too long, you'll get long
processing delay but little system overhead. A 1/10 sec. block
length is a good choice (i.e. 800 samples at 8kHz samplerate).
This value MUST be an EVEN number of samples!!!!
{numblk} is the number of blocks ({blklen} 'samples' each) that the
system will buffer. One or two blocks may produce audible 'clicks'.
The more blocks you use, the safer and smoother the process, but it
will eat more memory. A value of 4 is right. You can also
use {numblk}=0, that will auto-select as many blocks as memory
available in your system (max. = 128kbytes).
{usrproc} is a pointer to a user call-back. This callback receives
a BOOL {last} argument. The callback is called whenever the system
fills/frees a basic block. When the last audio block is filled/freed,
the {last} parameter is TRUE, or FALSE otherwise.
You can send {usrproc}=NULL to disable the callback.

This function {returns}!=0 if error (invalid argument, no memory...). */

UINT16 xsnd16_open( BOOL record, UINT16 srate, BOOL stereo,
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
	_dmach = blaster_env(BLASTER_DMA16,
			BLASTER_DEFAULT_DMA16,BLASTER_DECHEX_DMA16);

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

	_samplelen = ( stereo ? 4 : 2 );
	maxlen = (DMA_IS_DMA16(_dmach)?1024L*128L:1024L*64L)/_samplelen;
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
		_buffmem = dmabuff_malloc(bufflen*_samplelen,
				DMA_IS_DMA16(_dmach)?128:64, DMA_IS_DMA16(_dmach)?2:1,
				blklen*_samplelen,&len);
		if (_buffmem==NULL) {  /* no se pudo reservar memoria */
			/* si no se fijo el n. bloques, reduce */
			if ((!numblk) && (_nblk>1))
				_nblk--;
			else
				return 1;  /* no hay memoria para DMA */
		}
	} while (_buffmem==NULL);

	_buffptr = (pfINT8)fixmem(_buffmem, len, DMA_IS_DMA16(_dmach)?128:64,
			DMA_IS_DMA16(_dmach)?2:1, blklen*_samplelen, &_bufflmem, &use);

	xsnd16_reset();
	/* programa rutina servicio interrupcion */
	_oldint = INT_GET_VEC(IRQ_INTN(_irq));
	INT_SET_VEC(IRQ_INTN(_irq),_newint);
	PIC_ENABLE(_irq);  	/* conecta IRQ */
	PIC_SEND_EOI(_irq);

	_initialized = TRUE;

	return 0;  /* no hay error */
}

/**********************************************************/
/* detiene la grabacion/reproduccion y libera los recursos
utilizados (memoria, interrupciones, sound blaster...).
{devuelve}!=0 en caso de error
....................
stops the record/playback process and frees all the allocated
resources (memory, interrupts, sb16...).
{returns}!=0 if error. */

UINT16 xsnd16_close( VOID )
{
	if (!_initialized)
		return 1;  /* ni siquiera estaba abierto */

	sb_DSP_16_pause(_baseport);

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
/* Pone en marcha la grabacion/reproduccion.
{devuelve}!=0 en caso de error.
....................
Starts the record/play process.
{returns}!=0 if error. */

UINT16 xsnd16_start( VOID )
{
	if (_status>AWAIT)
		return 1; /* ya esta en marcha */
	if (_status<0)
		_status = -_status;

	if (_status>AWAIT)      /* estaba en pausa */
		sb_DSP_16_continue(_baseport);
	else if (_qblk) {
		_status=WAIT1;
		sb_DSP_16(_baseport, _record, SB_DSP_AUTO_INIT,
				SB_DSP_FIFO_ON, _stereo, SB_DSP_SIGNED, _sblen);
	}
	else if (_qtail==1) {  /* ultimo blk > 0 */
		_status=WAIT4;
		sb_DSP_16(_baseport, _record, SB_DSP_SINGLE_CICLE,
				SB_DSP_FIFO_ON, _stereo, SB_DSP_SIGNED, _taillen);
	}
	else if (_qtail==2) {   /* ultimo blk = 0 */
		_status=BLOCK;
	}

	return 0;
}

/**********************************************************/
/* detiene la grabacion/reproduccion.
{devuelve}!=0 en caso de error.
....................
Stops the record/play process.
{returns}!=0 if error. */

UINT16 xsnd16_stop( VOID )
{
	sb_DSP_16_pause(_baseport);
	if (_status<0)
		return 1;  /* ya estaba en pausa */

	_status = -_status;
	return 0;
}

/**********************************************************/
/* Si el usrproc() dura mucho tiempo, puede suceder que se
produzca una nueva interrupcion antes de que termine (o sea,
se produce overrun). NO se llamara de nuevo a usrproc, para
evitar problemas de reentrancia, pero se incrementara un
contador interno para que quede reflejada la situacion. Esta
funcion {devuelve} el numero de overruns (numero de bloques
llenos/vacios que producen interrupcion pero no generan llamada al
usrproc) desde la ultima vez que se llamo a esta misma funcion.
La funcion reinicializa a cero el contador interno.
....................
If the usrproc() function is too long, it could happen that a
new interrupt arrives before this function ends (i.e. you get
an overrun). The usrproc() will no be called again for this
new interrupt to avoid reentrancy problems, but an internal
counter will be incremented to reflect this fact.
This function {returns} the number of overruns (number of
memory blocks filled/emptyed with no associated call to
usrproc()) since the last time we called this function
(the function resets the internal counter). */

UINT16 xsnd16_getoverrun( VOID )
{
    UINT16 or;

	INT_DISABLE();
	or = _overrun;
	_overrun = 0;
	INT_ENABLE();

	return or;
}

/**********************************************************/
/* {devuelve} un puntero al bloque numero {nblk}.
Normalmente no es necesario utilizar esta funcion.
....................
{returns} a pointer to memory block number {nblk}. Normally
it's not necessary to use this function at all */

pfINT16 xsnd16_getblknum( UINT16 nblk )
{
	return (pfINT16)(_buffptr + (_samplelen*_blklen*nblk));
}

/**********************************************************/
/* {devuelve} el numero de bloques que mantiene el sistema.
Normalmente no es necesario utilizar esta funcion.
....................
{returns} the number of memory blocks kept by the module.
Normally it's not necessary to use this function at all. */

UINT16 xsnd16_getnblk( VOID )
{
	return _nblk;
}

/**********************************************************/
/* esta funcion {devuelve} un puntero al siguiente bloque
que el usuario debe procesar, o NULL en caso de no ser
posible.
En reproduccion, el usuario obtiene un puntero al proximo
bloque que debe rellenar. Si no quedan bloques libre que
el usuario pueda rellenar, la funcion {devuelve} NULL.
En grabacion, el usuario obtiene un puntero al siguente
bloque que debe leer. Si aun no hay bloques
llenos, {devuelve} NULL.

El funcionamiento es algo diferente en grabacion que en
reproduccion: reproduciendo, es posible llamar muchas veces
a ???_getblk(), que siempre {devuelve} un puntero al *mismo*
buffer mientras no se reencole con ???_addblk(). Por el
contrario, en grabacion, solo se puede llamar una vez a
???_getblk() para obtener un buffer rellenado. Una vez obtenido
el puntero al buffer, si llamamos de nuevo a ???_getblk()
obtendremos un puntero al *siguiente* buffer lleno (NULL si
aun no esta disponible) y asi sucesivamente.  En reproduccion,
podemos llamar a ???_addblk() para reencolar un buffer sin siquiera
haber hecho primero un ???_getblk(). En cambio, en grabacion no
es posible reencolar un buffer con  ???_addblk (o ???_addlastblk)
a no ser que primero se haya llamado a ???_getblk(). De este modo,
no es posible reencolar un buffer hasta que el usuario no lo haya
leido.
....................
This function {returs} a pointer to the next memory block that the
user must process, or NULL if not possible.
On playback the user gets a pointer to the next block that he must
fill. If there is no more blocks free, it {returns} NULL.
On record the user gets a pointer to the next block that he must
read (save). If there is no more filled block yet, it {returns} NULL.

The process is a bit different during recording than during
playback: On playback, you can call lot's of time to ???_getblk(),
as it will always {return} a pointer to the *same* buffer as
long as it is not re-queued using ???_addblk(). But on recording,
you can just call once to ???_getblk() to get a filled buffer.
Once you get the pointer to the buffer, if we call again ???_getblk()
we'll get a pointer to the *next* filled buffer (NULL if not available
yet), and so on.
During playback we can call ???_addblk() to re-queue a buffer even
if we have not issued a ???_getblk() first.  But on recording it
is not possible to queue a buffer using ???_addblk (or ???_addlastblk)
if you don't call first to ???_getblk(). In this way, it is not possible
to queue a buffer until you read it first. */

pfINT16 xsnd16_getblk( VOID )
{
		UINT16 n;

	if (_record) {  /* grabando */
		if (!_qtail) {   /* normalmente */
			if (_rectoread==_qblk)  /* y aun no hay bloques llenos */
				return NULL;
			INT_DISABLE();
			n = _dmablk+_qblk-_rectoread;
			INT_ENABLE();
		}
		else { /* grabando con ultimo bloque encolado */
			if (_status!=BLOCK) {  /* aun no terminado */
				if (_rectoread-1==_qblk)
					return NULL;
				INT_DISABLE();
				n = _dmablk+_qblk-_rectoread+1;
				INT_ENABLE();
			}
			else {  /* _status==BLOCK */
				if (!_rectoread)  /* ya no hay mas */
					return NULL;
				n = _dmablk;
			}
		}
		_rectoread--;
		if (n>=_nblk)  /* realmente <0 */
			n += _nblk;
	}
	else { 	/* reproduciendo */
		if ((_qblk==_nblk)||(_qtail))  /* no hay bloques */
			return NULL;
		INT_DISABLE();
		n = _dmablk+_qblk;
		INT_ENABLE();
		if (n>=_nblk)
			n -= _nblk;
	}

	return (pfINT16)(_buffptr + (n*_blklen*_samplelen));
}

/**********************************************************/
/* esta funcion encola el siguente bloque, o {devuelve}!=0
si no es posible.
En reproduccion, el usuario obtiene el siguente bloque
a rellenar llamando a la funcion ???_getblk(), rellena el
bloque, y lo re-encola con esta funcion.
En grabacion, el usuario obtiene el siguente bloque
a leer llamando a la funcion ???_getblk(), salva el
contenido del bloque, y lo re-encola con esta funcion.

Ver comentarios en  ???_getblk() sobre diferencias entre
grabacion y reproduccion
....................
This function re-queues the next block, or {returns}!=0
if it's not possible.
During playback, the user gets the next block to fill calling
to ???_getblk(), fills the block, and re-queue it with this
function.
During record, the user gets the next block to read calling
to ???_getblk(), saves the block contents, and re-queu it wint
this function. */

UINT16 xsnd16_addblk( VOID )
{
	if (_record) {  /* record */
		if ((_nblk==_rectoread)||(_qtail))  /* si no quedan bloques */
			return 1;
	}
	else {  /* play */
		if ((_qblk==_nblk)||(_qtail))  /* si no quedan bloques */
			return 1;
	}

	INT_DISABLE();
	if (_record)
		_rectoread++;
	_qblk++;
	if (_status==AWAIT)
		xsnd16_start();
	else if (abs(_status)==WAIT1A) {
		_status = (_status<0)?-WAIT2:WAIT2;
		sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
	}
	INT_ENABLE();

	return 0;
}

/**********************************************************/
/* Esta funcion permite encolar un ultimo bloque de longitud
inferior a la habitual. Una vez encolado este bloque el
sistema ya no admite encolar ningun bloque mas, y detiene
la grabacion/reproduccion. El sistema quedara bloqueado
al terminar la grabacion/reproduccion, y para volver a grabar
o reproducir, hay que resetear con ????_reset().
En caso de error {devuelve}!=0.
En grabacion no tiene mucho sentido, pero bueno, se puede hacer.
En reproduccion, sirve para encolar el ultimo grupo de muestras,
que no dan para rellenar un buffer completo. En {len} se envia
la longitud utilizada.
....................
This function allows to queue a final block with shorter length
that the normal block length. Once this block is queued you will
not be able to queue any more blocks. When this block is processed,
the record/playback process is stopped and the system will block.
To record/play again, you must reset using ???_reset().
{returns}!= if error.
On recording, this function is not very usefull.
On playback, this function is necessary to queue those final samples
you have that are not enough to fill a complete block. In {len} you
send the number of 'samples' you want to queue. */

UINT16 xsnd16_addlastblk( UINT32 len )
{
		UINT16 i, n;
	pfVOID p;
	INT16 i16;
	UINT32 u32;

	if (_record) {  /* record */
		if ((_nblk==_rectoread)||(_qtail))  /* si no quedan bloques */
			return 1;
	}
	else {  /* play */
		if ((_qblk==_nblk)||(_qtail))  /* si no quedan bloques */
			return 1;
	}

	INT_DISABLE();
	if (_record)
		_rectoread++;
	_qtail = (len>0)?1:2;
    _taillen = (UINT16)((_stereo?len*2:len)-1);
	if (_status==AWAIT)
		xsnd16_start();
	else if (abs(_status)==WAIT1A) {
		sb_IRQ_ack_dma16(_baseport);  /* SB ACK */
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
					u32 = 0;
				else if (len)
                    u32 = ((pfINT32)p)[(UINT16)(len-1)];
				else
					u32 = ((pfINT32)(_buffptr+(((_dmablk+_qblk-1)%_nblk)*
                            _blklen*_samplelen)))[(UINT16)(_blklen-1)];
                for (i=(UINT16)len; i<=n; i++)
					((pfINT32)p)[i]=u32;
			}
			else {  /* mono */
				if (_record)
					i16 = 0;
				else if (len)
                    i16 = ((pfINT16)p)[(UINT16)(len-1)];
				else
					i16 = ((pfINT16)(_buffptr+(((_dmablk+_qblk-1)%_nblk)*
                            _blklen*_samplelen)))[(UINT16)(_blklen-1)];
                for (i=(UINT16)len; i<=n; i++)
					((pfINT16)p)[i]=i16;
			}
		}
		else {  /* 2 semi bloques */
			_status = (_status<0)?-WAIT6:WAIT6;
			if ((!_record)&&(len<_blklen)) {
                n = (UINT16)(_blklen-1);
				p = _buffptr+(((_dmablk+_qblk) % _nblk)*_blklen*_samplelen);
				if (_stereo) {
                    u32 = ((pfINT32)p)[(UINT16)(len-1)];
										for (i=(UINT16)len; i<=n; i++)
						((pfINT32)p)[i]=u32;
				}
				else {  /* mono */
                    i16 = ((pfINT16)p)[(UINT16)(len-1)];
                    for (i=(UINT16)len; i<=n; i++)
						((pfINT16)p)[i]=i16;
				}
			}
		}
	}
	INT_ENABLE();

	return 0;
}

/**********************************************************/
/* {devuelve} TRUE si es sistema esta detenido (o en pausa).
....................
{returns} TRUE if the system is stopped (or paused) */

BOOL xsnd16_stopped( VOID )
{
	return (_status<0);
}

/**********************************************************/
/* {devuelve} TRUE si el sistema ha terminado de grabar/reproducir
el ultimo bloque y ha entrado en bloqueo.
....................
{returns} TRUE if the system has already finished recorded/played
the last block and is in blocked state. */

BOOL xsnd16_blocked( VOID )
{
	return (_status==BLOCK);
}

/**********************************************************/
/* Configura la frecuencia de muestro {srate}.
{devuelve}!=0 en caso de error.
....................
Configures the sampling rate to {srate}.
{returns}!=0 if error. */

UINT16 xsnd16_srate( UINT16 srate )
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
/* {devuelve} la frecuencia de muestreo en uso.
....................
{returns} the used sampling rate. */

DOUBLE xsnd16_getsrate( VOID )
{
	return sb_DSP_tc2dsr(sb_DSP_sr2tc(_srate,(_stereo?2:1)), (_stereo?2:1));
}

/**********************************************************/
/* inicializacion global del sistema audio. Ejecutar
antes que ninguna otra funcion. {devuelve}!=0 en caso
de error
....................
Global system initialization. This must be called before
any other xsnd function. It {returns}!=0 if error  */

UINT16 xsnd16_initialize( VOID )
{
	return sb_DSP_reset( blaster_env(BLASTER_BASEPORT,
			BLASTER_DEFAULT_BASEPORT, BLASTER_DECHEX_BASEPORT));
}

/**********************************************************/
/* {devuelve} el numero de bloques actualmente encolados
para reproduccion o grabacion. El bloque en curso tambien
se considera.
....................
{returns} the current number of queued blocks for playback/record.
The block being played/recorded right now is also considered. */

UINT16 xsnd16_getqblk( VOID )
{
	if (_status==BLOCK) return 0;
	return _qblk+(_qtail?1:0);
}

/**********************************************************/
