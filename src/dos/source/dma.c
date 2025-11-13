/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ DMA.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... DMA
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.3    25/08/94  Borja     bug en lectura count_l1 y addr
2.0.2    15/03/94  Borja     optimizar lectura count_l1
2.0.1    15/03/94  Borja     algunos comentarios nuevos
2.0.0    28/02/94  Borja     Unificado DMA8 con DMA16
1.0.1    19/02/94  Borja     acceso a puertos mediante PORTS.H
1.0.0    06/08/93  Borja     Codificacion inicial.

======================== Contenido ========================
Definicion de macros y funciones para manejo basico DMA.
Definicion de funciones para la programacion del DMA
de 8 y 16 bits del AT. No se han implementado todas las
funciones ni modos de funcionamiento, solo los necesarios
para trabajar con perifericos tipo sblaster o scanner. Por
ejemplo, se programa siempre modo de transferencia SINGLE,
ya que los modos BLOCK o DEMAND no siempre funcionan.
Ademas no se implementan opciones para reprogramar el
registro COMMAND ni para efectuar el MASTER_RESET, ya que
en principio no hay que tocarlos (programados por BIOS al
arrancar).
El codigo no es optimo en absoluto :)
....................
Macro and function definitions for 8 and 16 bit DMA management.
Only some of the modes and functionality of the DMA has been
programmed, enough to work with sound cards or scanners. For
example, only SINGLE transfer mode has been considered, as
BLOCK and DEMAND modes do not always work. More over there are
also no function to program the COMMAND register nor to perform
a MASTER_RESET, as normally it't not necessary to play with this
registers (they are programmed by your computer BIOS on startup).
This is not optimal code at all!
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "dma.h"

#include "ports.h"  /* leer/escribir puertos */
#include "intrs.h"  /* desconectar interrupciones */

/**********************************************************/
/* Declara e inicializa las siguientes variables

UINT8 dma16_0x01, dma16_0xC0;

en funcion de {channel} canal DMA (0-7) en cuestion.
Dado un canal DMA en {channel} que debe estar en el rango 0-7,
esta macro configura las dos variables  {dma16_0x01} y
{dma16_0xC0} en funcion del tipo de canal DMA. Los canales
DMA de 8 bits (0 a 3) ponen a 0 estas 2 variables, mientras
que los canales DMA de 16 bits (4 a 7) pone 0x01 en la primera,
y 0xC0 en la segunda variable.
Ademas, se modifica {channel} para que sea siempre un valor entre 0 y 3,
quedando identificados los canales de 16 bits por las otras 2
variables.
Los dos valores dma16_??? se utilizan en muchos macros definidos despues. */
#define DMA_DMA16_INFO()   \
    UINT8  dma16_01h, dma16_C0h;  \
    dma16_01h = ((UINT8)(channel)) >> 2;  \
	dma16_C0h = (dma16_01h) ? 0xC0 : 0;  \
	channel &= 0x3

/* puertos para registros DMA-8bits. Se indica (I/O).
Los registros de DMA-16bits se obtienen a partir de estos,
haciendo (0xC0 | ({dma8_reg} << 1)) (usar DMA_REG()).
Cuando un registro depende del canal DMA, la variable UINT8 {channel}
debe estar en scope, y valer siempre de 0 a 3. */
#define DMAx_REG_STATUS   0x08  /* (I) registro de estado */
#define DMAx_REG_COMMAND  0x08 /* (O) registro de orden */
#define DMAx_REG_REQUEST  0x09 /* (O) registro de peticion */
#define DMAx_REG_SMASK    0x0A  /* (O) registro de mascara simple */
#define DMAx_REG_MMASK    0x0F  /* (O) registro de mascara multiple */
#define DMAx_REG_CLEARB   0x0C /* (O) registro clear byte pointer flag */
#define DMAx_REG_MODE     0x0B   /* (O) registro de modo */
#define DMAx_REG_ADDR     (channel << 1)  /* (I/O) reg. direccion */
#define DMAx_REG_COUNT    ( DMAx_REG_ADDR + 1 )  /* (I/O) reg. cuenta */

/* {devuelve} la direccion de un registro DMA 8/16 bits en base a los
registros DMA-8 bits. En UINT8 {reg} se envia el registro DMAx_REG_???. */
#define DMA_REG(reg)   (dma16_C0h | ((reg) << dma16_01h))

/* estos son los registros DMA definitivos, para 8 o 16 bits, que
se calculan en funcion de {channel}, {dma16_01h} y {dma16_C0h} */
#define DMA_REG_STATUS   DMA_REG(DMAx_REG_STATUS)
#define DMA_REG_COMMAND  DMA_REG(DMAx_REG_COMMAND)
#define DMA_REG_REQUEST  DMA_REG(DMAx_REG_REQUEST)
#define DMA_REG_SMASK    DMA_REG(DMAx_REG_SMASK)
#define DMA_REG_MMASK    DMA_REG(DMAx_REG_MMASK)
#define DMA_REG_CLEARB   DMA_REG(DMAx_REG_CLEARB)
#define DMA_REG_MODE     DMA_REG(DMAx_REG_MODE)
#define DMA_REG_ADDR     DMA_REG(DMAx_REG_ADDR)
#define DMA_REG_COUNT    DMA_REG(DMAx_REG_COUNT)

/* registros de pagina DMA-8 bits. ver DMA8_REG_PAGE() y DMA_REG_PAGE() */
#define DMA8_REG_PAGE0  0x87  /* (I/O) pagina DMA canal 0 */
#define DMA8_REG_PAGE1  0x83  /* (I/O) pagina DMA canal 1 */
#define DMA8_REG_PAGE2  0x81  /* (I/O) pagina DMA canal 2 */
#define DMA8_REG_PAGE3  0x82  /* (I/O) pagina DMA canal 3 */
/*{devuelve} cual es le registro de pagina para el canal {channel} de
DMA de 8 bits. {channel} debe estar en el rango 0 a 3 */
#define DMA8_REG_PAGE \
	((channel==0)?DMA8_REG_PAGE0:(channel==1)?DMA8_REG_PAGE1: \
	(channel==2)?DMA8_REG_PAGE2:DMA8_REG_PAGE3)
/* registros de pagina DMA-16 bits. ver DMA16_REG_PAGE() y DMA_REG_PAGE() */
#define DMA16_REG_PAGE0  0x88  /* (I/O) pagina DMA canal 4 */
#define DMA16_REG_PAGE1  0x8B  /* (I/O) pagina DMA canal 5 */
#define DMA16_REG_PAGE2  0x89  /* (I/O) pagina DMA canal 6 */
#define DMA16_REG_PAGE3  0x8A  /* (I/O) pagina DMA canal 7 */
/*{devuelve} cual es le registro de pagina para el canal {channel} de
DMA de 16 bits. {channel} debe estar en el rango 0 a 3 */
#define DMA16_REG_PAGE \
	((channel==0)?DMA16_REG_PAGE0:(channel==1)?DMA16_REG_PAGE1: \
	(channel==2)?DMA16_REG_PAGE2:DMA16_REG_PAGE3)

/* {devuelve} el registro de pagina DMA8/16 bits, para el canal
DMA indicado (siempre de 0 a 3, incluso para los de 16 bits. en
dma16_01h ya esta indicado si es de 16 bits) */
#define DMA_REG_PAGE  (dma16_01h?DMA16_REG_PAGE:DMA8_REG_PAGE)

/* macros para programacion del DMA */

/* genera byte para enmascarar de forma simple el canal {channel} (0 a 3) */
#define DMA_MASK_ON  ((channel) | 0x04)

/* genera byte para desenmascarar de forma simple el canal {channel} */
#define DMA_MASK_OFF  (channel)

/* genera mascara multiple, para los cuatro canales a la vez.
Enviar TRUE en s(i) para enmascarar la linea {i}, o FALSE para no
enmascararla */
#define DMA_MULTI_MASK(s0,s1,s2,s3)  \
	((s0)?0x01:0x00)|((s1)?0x02:0x00)|((s2)?0x04:0x00)|((s3)?0x08:0x00)

/* metodos de transferencia posibles */
#define DMA_XFER_DEMAND  0x00
#define DMA_XFER_SINGLE  0x40
#define DMA_XFER_BLOCK   0x80
#define DMA_XFER_CASCADE 0xC0

/* genera valor para establecer el modo DMA para el canal {channel} (0-3).
{xfermode}=DMA_XFER_???? (???? = SINGLE,DEMAND,BLOCK,CASCADE)
{decrement}==TRUE si queremos que transfiera hacia atras
{autoinit}==TRUE activa la autoinicializacion al terminal el ciclo DMA
{write}==TRUE activa Puerto--->Memoria.
{write}==FALSE activa Memoria-->Puerto */
#define DMA_MODE(xfermode,increment,autoinit,write) \
	( (channel) | (xfermode) | ((decrement) ? 0x20 : 0) |  \
	((autoinit) ? 0x10 : 0) | ((write) ? 0x04 : 0x08) )

/* genera valor de pagina para la direccion lineal {lmem}. */
#define DMA_ADDR_PAGE(lmem)  ((UINT8)((lmem)>>16) & (~dma16_01h))
/* genera valor LSB para la direccion lineal {lmem} */
#define DMA_ADDR_LSB(lmem)  (UINT8)(((UINT16)(lmem)) >> dma16_01h)
/* genera valor MSB para la direccion lineal {lmem} */
#define DMA_ADDR_MSB(lmem)  (UINT8)((UINT32)(lmem) >> (8 + dma16_01h))

/* genera valor LSB para el contador {count_l1} */
#define DMA_COUNT_LSB(count_l1)  (UINT8)(count_l1)
/* genera valor MSB para el contador {count_1}  */
#define DMA_COUNT_MSB(count_l1)  (UINT8)(((UINT16)(count_l1)) >> 8)

/*{devuelve} valor para hacer solicitud soft (SREQ) de dma por el
canal {channel} */
#define DMA_SREQ  (0x04 | (channel))

/* {devuelve} un valor distinto de cero si se ha producido un
TerminalCount(fin) en el canal {channel}, siendo {b} el valor leido del
registro de estado */
#define DMA_GET_STATUS_TC(b)  ( (b) & (0x01 << (channel)) )

/* {devuelve} un valor distinto de cero si se ha producido una
solicitud de DMA en el canal {channel}, siendo {b} el valor leido del
registro de estado */
#define DMA_GET_STATUS_REQ(b)  ( (b) & (0x10 << (channel)) )

/*a partir de los valores {lsb} y {msb} leidos, forma la cuenta actual
del DMA (elementos que quedan por transferir menos 1) */
#define DMA_COUNT_L1(msb,lsb)  ((((UINT16)(msb))<<8) | ((UINT16)(lsb)))

/*reconstruye la direccion a partir de la pagina {page}, el byte
alto de direccion {msb} y el bajo {lsb}. Se utiliza {dma16_01h} */
#define DMA_ADDRESS(page,msb,lsb) \
    ((((UINT32)page)<<16)|((UINT32)((((UINT16)(msb))<<8)|(lsb))<<(dma16_01h)))

/**********************************************************/
/* resetea el byte pointer flag del controlador DMA, para que cuando
se introduzca un valor de 2 bytes, este sincronizado (primero LSB
y luego MSB). No es necesario hacerlo, pero si se hace por ejemplo antes
de leer la cuenta DMA o la direccion actual no viene naada mal ....
....................
Resets the byte pointer flag of the DMA controler; after this, you
can introduce/get two-byte values through the 8 bit port (LSB first, MSB
later). Normally it's not necessary to do this, as it's suppoused
that the controller is correctly syncronithed, but it's convenient
to do this before reading DMA count or the actual address. */

VOID dma_resetb( VOID )
{
    UINT8 channel = 0;  /* es necesario que este definido */
	DMA_DMA16_INFO(); 	/* genera informacion 8/16 bits */


    OUTPORT8(DMA_REG_CLEARB,0x00);   /* byte reset */
}


/**********************************************************/
/* Configura el DMA: Permite programar todos los valores necesarios
para poner en funcionamiento un canal DMA. El resto de funciones
dma_set_???() simplemente son versiones reducidas de esta funcion
que solo reprograman ciertas partes del controlador.

{channel} canal DMA a programar. Puede ser 0, 1, 2 o 3 (canales
  de 8 bits) o 4, 5, 6, 7 (canales de 16 bits).
{decrement} distinto de cero para hacer el DMA hacia atras. Si es
cero, el DMA se realiza hacia alante, que es lo mas normal.
{autoinit} distinto de cero para modo de autoinicializacion
  (usar constantes DMA_AUTO_INIT y DMA_SINGLE_CICLE)
{writetomem} distinto de cero para escribir a memoria desde
  un puerto.  cero para leer de memoria y escribir en un puerto.
  (usar constantes DMA_WRITE_TO_MEM y DMA_READ_FROM_MEM)
{lmem} direccion (en formato lineal de 32 bits) del buffer DMA. Tener
  en cuenta que en los canales de 16 bits, debe ser una direccion
  par!!!!!
{count_l1} numero de elementos (bytes en DMA8 o words en DMA16)
  menos 1, que tiene el buffer DMA
{enable_dma} distinto de cero para activar el dma. cero para no
  activarlo (aunque se programa)
  (usar constantes DMA_ENABLE y DMA_DISABLE)

{lmem} se debe obtener a partir de un buffer de memoria reservado
por el usuario, teniendo en cuenta que no debe cruzar ningun
banco de 64kb para los canales de 8 bits, o un banco de 128kb
para los canales de 16 bits. Utilizar las funciones fixmem() o
fixlen() definidas en LMEM.C para fijar estas condiciones, asi como
la longitud ({count_l1}+1) del buffer (tener en cuenta, que para
canales de 16 bits, count indica words).
Si el dma no se activa ({enable_dma}==0) se debe hacer
dma_enable(ch) para activarlo.
NOTA: La macro DMA_IS_DMA16(ch) definida en DMA.H {devuelve}
TRUE cuando el canal {ch} es de 16 bits, o FALSE sin es de 8.
....................
Configures DMA channel: This function allows a quite complete
configuration of the DMA controller. The other dma_set_???()
functions are reduced versions of this one, valid to reprogram
just some part of the DMA controller.

{channel} is the DMA channel to programm. This can be 0, 1, 2 or 3
  for 8 bit channels, and 4, 5, 6, 7 for 16 bits channels.
{decrement} if !=0 runs the DMA buffer backwards. If zero, DMA
  runs forwards, that is the usual way. You can use the
  defines DMA_INCREMENT  and DMA_DECREMENT for clearer code.
{autoinit} if !=0 selects the auto-initialize mode. Otherwise,
  selects the single cicle mode. You can use DMA_AUTO_INIT and
  DMA_SINGLE_CICLE for clearer source code.
{writetomem} if !=0 selects port-to-memory tranfer. If 0, selects
  memory-to-port transfer. You can use DMA_WRITE_TO_MEM and
  DMA_READ_FROM_MEM to get clearer source code.
{lmem} this is the DMA buffer 32-bit linear address. For 16bit
  DMA channels this must be a even address!!!
{count_l1} number of elements minus one (count-1) of the DMA buffer.
  In 8 bit DMA channels, one element is one byte. In 16 bit DMA
  channels, one element is one word (two bytes).
{enable_dma} if !=0 enables DMA channel. if ==0, keeps disabled the
  DMA channel, but it is programmed (you can enable it later).
  You can use DMA_ENABLE y DMA_DISABLE to get clearer code.

{lmem} must be a linear pointer to a memory area reserved by the user
in the first megabyte of memory. The whole buffer ({count_l1}+1 elements)
_must_ be in the same memory page (ie. 64kb blocks in 8 bit channels
and 128kb blocks in 16 bit channels). You can use the functions
fixmem() or fixlen() defined in LMEM.C to fix this requeriment and
to determinte the length ({count_1}+1) of the buffer (remember that
for 16 bit DMA channelss, {count_l1} refers to words).
If the DMA channels is not activated ({enable_dma}==0) you can
use dma_enable(ch) later to enable it.
NOTE: macro DMA_IS_DMA16(ch) defined in DMA.H {returns} TRUE
when {ch} is a 16 bit DMA channel (ch=4,5,6,7), or FALSE when it
is a 8 bit channel (ch=0,1,2,3). */

VOID dma_set( UINT8 channel, BOOL decrement, BOOL autoinit, BOOL writetomem,
        UINT32 lmem, UINT16 count_l1, BOOL enable_dma )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	INT_DISABLE();  /* desconecta interrupciones */
    OUTPORT8(DMA_REG_SMASK,DMA_MASK_ON); /* enmascara linea DMA */
    OUTPORT8(DMA_REG_MODE,DMA_MODE(DMA_XFER_SINGLE,decrement,
			autoinit,writetomem));  /* modo DMA */
    OUTPORT8(DMA_REG_CLEARB,0x00);  /* byte reset */
	/* direccion buffer de transferencia */
    OUTPORT8(DMA_REG_ADDR, DMA_ADDR_LSB(lmem));
    OUTPORT8(DMA_REG_ADDR, DMA_ADDR_MSB(lmem));
    OUTPORT8(DMA_REG_PAGE, DMA_ADDR_PAGE(lmem));
	/* longitud buffer transferencia */
    OUTPORT8(DMA_REG_COUNT, DMA_COUNT_LSB(count_l1));
    OUTPORT8(DMA_REG_COUNT, DMA_COUNT_MSB(count_l1));
	if (enable_dma)  /* conectar DMA ? */
        OUTPORT8(DMA_REG_SMASK, DMA_MASK_OFF); /* desenmascara linea */
	INT_ENABLE(); /* conectar interrupciones */
}

/**********************************************************/
/* establece el modo de funcionamiento del DMA, en funcion
de {autoinit} y {writetomem}. Ver dma_set() para mas informacion.
....................
Selects the working mode of the DMA, based on {autoinit} and
{writetomem}. Refer to dma_set() for more information. */

VOID dma_set_mode( UINT8 channel, BOOL decrement, BOOL autoinit, BOOL writetomem,
		BOOL enable_dma )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	INT_DISABLE();  /* desconecta interrupciones */
    OUTPORT8(DMA_REG_SMASK,DMA_MASK_ON); /* enmascara linea DMA */
    OUTPORT8(DMA_REG_MODE,DMA_MODE(DMA_XFER_SINGLE,decrement,
			autoinit,writetomem));  /* modo DMA */
	if (enable_dma)  /* conectar DMA ? */
        OUTPORT8(DMA_REG_SMASK, DMA_MASK_OFF); /* desenmascara linea */
	INT_ENABLE(); /* conectar interrupciones */
}

/**********************************************************/
/* establece el buffer para el DMA en funcion de {lmem} y
{count_l1}. Ver dma_set() para mas informacion.
....................
Configures the DMA memory buffer based on {lmem} and {count_l1}.
Refer to dma_set() for more information */

VOID dma_set_buff( UINT8 channel, UINT32 lmem, UINT16 count_l1,
		BOOL enable_dma )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	INT_DISABLE();  /* desconecta interrupciones */
    OUTPORT8(DMA_REG_SMASK,DMA_MASK_ON); /* enmascara linea DMA */
    OUTPORT8(DMA_REG_CLEARB,0x00);  /* byte reset */
	/* direccion buffer de transferencia */
    OUTPORT8(DMA_REG_ADDR, DMA_ADDR_LSB(lmem));
    OUTPORT8(DMA_REG_ADDR, DMA_ADDR_MSB(lmem));
    OUTPORT8(DMA_REG_PAGE, DMA_ADDR_PAGE(lmem));
	/* longitud buffer transferencia */
    OUTPORT8(DMA_REG_COUNT, DMA_COUNT_LSB(count_l1));
    OUTPORT8(DMA_REG_COUNT, DMA_COUNT_MSB(count_l1));
	if (enable_dma)  /* conectar DMA ? */
        OUTPORT8(DMA_REG_SMASK, DMA_MASK_OFF); /* desenmascara linea */
	INT_ENABLE(); /* conectar interrupciones */
}

/**********************************************************/
/* establece la direccion del buffer DMA en funcion de {lmem}.
Ver dma_set() para mas informacion.
....................
Configures the DMA memory buffer address based on {lmem}.
Refer to dma_set() for more information */

VOID dma_set_buff_addr( UINT8 channel, UINT32 lmem,  BOOL enable_dma )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	INT_DISABLE();  /* desconecta interrupciones */
    OUTPORT8(DMA_REG_SMASK,DMA_MASK_ON); /* enmascara linea DMA */
    OUTPORT8(DMA_REG_CLEARB,0x00);  /* byte reset */
	/* direccion buffer de transferencia */
    OUTPORT8(DMA_REG_ADDR, DMA_ADDR_LSB(lmem));
    OUTPORT8(DMA_REG_ADDR, DMA_ADDR_MSB(lmem));
    OUTPORT8(DMA_REG_PAGE, DMA_ADDR_PAGE(lmem));
	if (enable_dma)  /* conectar DMA ? */
        OUTPORT8(DMA_REG_SMASK, DMA_MASK_OFF); /* desenmascara linea */
	INT_ENABLE(); /* conectar interrupciones */
}

/**********************************************************/
/* establece la cuenta del buffer DMA {count_l1}.
Ver dma_set() para mas informacion.
....................
Configures the DMA memory buffer length based on {count_l1}.
Refer to dma_set() for more information */

VOID dma_set_buff_count( UINT8 channel, UINT16 count_l1,
		BOOL enable_dma )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	INT_DISABLE();  /* desconecta interrupciones */
    OUTPORT8(DMA_REG_SMASK,DMA_MASK_ON); /* enmascara linea DMA */
    OUTPORT8(DMA_REG_CLEARB,0x00);  /* byte reset */
	/* longitud buffer transferencia */
    OUTPORT8(DMA_REG_COUNT, DMA_COUNT_LSB(count_l1));
    OUTPORT8(DMA_REG_COUNT, DMA_COUNT_MSB(count_l1));
	if (enable_dma)  /* conectar DMA ? */
        OUTPORT8(DMA_REG_SMASK, DMA_MASK_OFF); /* desenmascara linea */
	INT_ENABLE(); /* conectar interrupciones */
}

/**********************************************************/
/* Dado el canal DMA {channel} (0,1,2,3,4,5,6,7),
esta funcion enmascara la linea, por lo que detiene el DMA en
la linea indicada.
....................
This function masks the DMA line identified by {channel} (0,1,2,
3,4,5,6,7). */

VOID dma_disable( UINT8 channel )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
    OUTPORT8(DMA_REG_SMASK,DMA_MASK_ON); /* enmascara linea DMA */
}

/**********************************************************/
/* Dado el canal DMA en {channel} (0,1,2,3,4,5,6,7),
esta funcion desenmascara la linea DMA por el canal indicado
con lo que el DMA podra funcionar.
....................
This function unmasks the DMA line identified by {channel} (0,1,2,
3,4,5,6,7). Once the channel is unmasked, it will carry out
any requested DMA transfer (of course the channel must be previously
configured with dma_set()) */

VOID dma_enable( UINT8 channel )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
    OUTPORT8(DMA_REG_SMASK, DMA_MASK_OFF); /* desenmascara linea */
}

/**********************************************************/
/* esta funcion efectua por software una peticion de ciclo DMA
en la linea indicada.
....................
This function performs a software DMA cicle request through the
DMA line {channel}. */

VOID dma_sreq( UINT8 channel )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
    OUTPORT8(DMA_REG_REQUEST,DMA_SREQ);
}

/**********************************************************/
/* {devuelve} la cuenta DMA (elementos a transferir - 1)
para terminar el ciclo DMA (envia un TerminalCount TC) por
el canal {channel}. El DMA podra recomenzar de nuevo automaticamente
si se encuentra en modo de autoinicializacion.
El valor devuelto indica el numero de bytes (DMA8) o words (DMA16)
que aun quedan por tranferir, menos 1.
Por si las moscas, no esta de mas efectuar un dma_resetb() antes de
esta funcion...
....................
{returns} the DMA count (items to transfer - 1) left to complete the
DMA cicle (i.e., to send a TerminalCount TC) for DMA channel number
{channel}.  The DMA operation can restart automatically again when
the cicle completes if it has been previously programmed in auto-
initialize mode.
The value returned by this function is the number of bytes (DMA8) or
words (DMA16) left to transfer minus 1.
It's not a bad idea to call dma_resetb() before calling this
function. */

UINT16 dma_get_count_l1( UINT8 channel )
{
    UINT8 lsb1, msb1, lsb2, msb2;
    UINT16 port;

	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	port = DMA_REG_COUNT;

	INT_DISABLE();
    lsb1 = INPORT8(port);
    msb1 = INPORT8(port);
    lsb2 = INPORT8(port);
    msb2 = INPORT8(port);
	INT_ENABLE();

	/* comprobar si lsb ha llegado a 0 y empezado de nuevo en 0xFF,
	pues al ir decrementando la cuenta el acarreo puede modificar msb
	entre IN(lsb) e IN(msb) */

	/* Secuencia:   lsb1 * msb1 # lsb2 * msb2.
	Los puntos peligrosos son *, aunque puede haber cambio en #.
	Si msb2==msb1, significa que ni en # ni en *(2) ha habido cambio,
	por lo que la segunda lectura es segura */
	if (msb2==msb1)
		return DMA_COUNT_L1(msb2,lsb2);

	/* si msb2!=msb1, ha habido cambio en # o en *(2), por lo que la
	segunda lectura no ha sido segura. ¨pero es segura la primera lectura?
	puede que se produzca algun cambio en *(1) pero no son peligrosos a
	no ser que se produzca el acarreo. Como en # o *(2) se produce el acarreo,
	significa que por lo menos tiene que haber 0xFF lecturas entre *(1) y *(2)
	para que se produzcan dos acarreos y la primera lectura sea dudosa. Pero
	una transferencia DMA de 256 bytes entre 4 inports() no creo que sucedan
	ni en el Sinclair Spectrum. Por tanto, damos a la primera lectura como
	valida */

	return DMA_COUNT_L1(msb1,lsb1);
}

/**********************************************************/
/* {devuelve} la direccion de memoria de la siguiente transferencia
DMA que se va a efectuar por el canal {channel}, en formato de memoria
lineal (32 bits).
Por si las moscas, no esta de mas efectuar un dma_resetb() antes
de esta funcion...
....................
{returns} the memory address of the next memory position to be
transfered (read or write) by the DMA controller using
channel {channel}. This memory address is expressed as a linear
address (32 bit).
It's not a bad idea to call dma_resetb() before calling this
function. */

UINT32 dma_get_addr( UINT8 channel )
{
    UINT8 lsb1, msb1, lsb2, msb2, page;
    UINT16 port;

	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
	port = DMA_REG_ADDR;
    page = INPORT8(DMA_REG_PAGE);

	INT_DISABLE();
    lsb1 = INPORT8(port);
    msb1 = INPORT8(port);
    lsb2 = INPORT8(port);
    msb2 = INPORT8(port);
	INT_ENABLE();

	/* comprobar si lsb es 0, pues al ir decrementando/incrementando la
	cuenta el acarreo puede modificar msb entre IN(lsb) e IN(msb). Ver
	comentarios en dma_get_count_l1() para explicacion detallada del
	metodo seguido */
	if (msb2==msb1)
		return DMA_ADDRESS(page,msb2,lsb2);

	return DMA_ADDRESS(page,msb1,lsb1);
}

/**********************************************************/
/* {devuelve} el registro de estado DMA. Al leer este registro,
se resetea su valor.
....................
{returns} the DMA status register. This register is cleared when
you read it! */

UINT8 dma_get_status( VOID )
{
    UINT8 channel = 0;  /* es necesario que este definido */
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
    return INPORT8(DMA_REG_STATUS);
}

/**********************************************************/
/* {devuelve} FALSE si no hay peticion DMA pendiente por la linea
{channel}. Se lee el registro de estado y por tanto se resetea,
borrando los posibles flags para otros canales. Usar
dma_get_status_dmareq() si se quieren mirar varios canales.
....................
{returns} FALSE if there is no DMA request pending on DMA line
{channel}. The status register must be read to test for this, and
it is automatically cleared. This deletes the flags for _all_ DMA
channels!!!.  You should use dma_get_status_dmareq() if you want
to check for various channels */

BOOL dma_get_dmareq( UINT8 channel )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
    return (DMA_GET_STATUS_REQ(INPORT8(DMA_REG_STATUS))!=0);
}

/**********************************************************/
/* {devuelve} FALSE si no hay peticion DMA pendiente por la linea
{channel}. El registro de estado, previamente leido con dma_get_status()
se debe enviar en {status}
....................
{returns} FALSE if there isn't any DMA request pending on line {channel}.
The status register, previosly read using dma_get_status(), must
be sent in {status}. */

BOOL dma_get_status_dmareq( UINT8 status, UINT8 channel )
{
	channel &= 0x3;  /* limita canales a rango 0-3 */
	return (DMA_GET_STATUS_REQ(status)!=0);
}

/**********************************************************/
/* {devuelve} FALSE si aun no se ha terminado el ciclo DMA.
Cuando termina el ciclo, se emite un TerminalCount(TC) que
se almacena en un flag interno del controlador DMA, y
se detiene el DMA; en tal situacion esta funcion devuelve
TRUE y borra el flag interno, pero si esta en modo de
autoinicializacion el DMA comenzara de nuevo, aunque el flag no
se borrara hasta que se lea con esta funcion.
Se lee el registro de estado y por tanto se resetea,
borrando los posibles flags para otros canales. Usar
dma_get_status_tc() si se quieren mirar varios canales
....................
{returns} FALSE if DMA cicle has not finished yet.
When the DMA cicle ends, a TerminalCount (TC) flag is raised
and the DMA transfer stops. This flag is stored in the DMA
controller.  This function {returns} the value of this flag
for DMA channel {channel}, and the flag is reset.
If auto-inititialize mode is programmed, the DMA transfer will
restart again and the TC flag will go on raised until you
read it.
TC flags for all channels are stored in a internal status register.
When we read this register it is cleared, reseting the flags for
_all_ DMA channels. You can use dma_get_status_tc() if you
want to test for more than one DMA channels. */

BOOL dma_get_tc( UINT8 channel )
{
	DMA_DMA16_INFO();  /* genera informacion 8/16 bits */
    return (DMA_GET_STATUS_TC(INPORT8(DMA_REG_STATUS))!=0);
}

/**********************************************************/
/* {devuelve} FALSE si aun no se ha terminado el ciclo DMA.
Ver dma_get_tc() para mas informacion.
El registro de estado, previamente leido con dma_get_status()
se debe enviar en {status}
....................
{returns} FALSE if DMA cicle is still working.
Refer to dma_get_tc() for detailed information.
You must send the status register to this function in the {status}
parameter. You can get this register using dma_get_status(). */

BOOL dma_get_status_tc( UINT8 status, UINT8 channel )
{
	channel &= 0x3;  /* limita canales a rango 0-3 */
	return (DMA_GET_STATUS_TC(status)!=0);
}

/**********************************************************/

