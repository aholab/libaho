/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SB.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Sound Blaster
Codigo condicional........... DSP_GET_DOUBLE_RATE

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.0	 03/03/95  Borja	 funciones IRQ_ack_???
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Primitivas basicas para tarjetas SoundBlaster.

La SB16 soporta comandos de las tarjetas anteriores,
pero se recomienda usar sb_DSP_16() y sb_DSP_8() para
grabar/reproducir.

Si DSP_GET_DOUBLE_RATE esta definido, se genera la
funcion sb_DSP_tc2dsr() que utiliza numeros en coma
flotante.
....................
Basic primitives for SoundBlaster cards.

SB16 supports commands from previous SB cards, but the
recommended commands to play/rec are sb_DSP_16() and sb_DSP_8().

If DSP_GET_DOUBLE_RATE is defined, the function
sb_DSP_tc2dsr() is generated. This function uses floating
point numbers.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "sb.h"

#include "ports.h"  /* leer/escribir puertos */

/**********************************************************/
/* numero maximo de reintentos en operaciones con el DSP.
Debe entrar en un UINT16 */

#define DSP_TEST_LOOPS  0xFFFF

/**********************************************************/
/* registros del DSP */
#define DSP_REG_RESET  (_sb_baseport+0x06)
#define DSP_REG_RDATA  (_sb_baseport+0x0A)
#define DSP_REG_RSTATUS  (_sb_baseport+0x0E)
#define DSP_REG_WDATA  (_sb_baseport+0x0C)
#define DSP_REG_WSTATUS  (_sb_baseport+0x0C)

/* respuesta del DSP al hacer un reset */
#define DSP_READY  0xAA
/* mascara para comprobar si se puede enviar dato al DSP */
#define DSP_MASK_BUSY  0x80
/* mascara para comprobar si se puede leer dato del DSP */
#define DSP_MASK_DATA_AVAIL  0x80

/* informacion del mixer, necesaria para ACK de interrupcion al DSP */
#define MIX_REG_ADDR   (_sb_baseport+0x04)
#define MIX_REG_DATA   (_sb_baseport+0x05)
#define MIX_ADDR_IRQ_STAT   0x82

/* leer en estos registros para enviar ACK para el DSP,
en funcion de las diversas interrupciones posibles */
#define DSP_REG_IRQ_ACK_DMA8MIDI  DSP_REG_RSTATUS
#define DSP_REG_IRQ_ACK_DMA16  (_sb_baseport+0x0F)
#define DSP_REG_IRQ_ACK_MPU  (_sb_baseport+0x10)

/**********************************************************/
/* no hace nada, solo para perder tiempo al resetear el DSP
....................
iddle function to wait a little time*/

VOID do_nothing( VOID )
{
}

/**********************************************************/
/* lee y {devuelve} dato del DSP (es un byte),
o {devuelve} un 0xFFFF (DSP_ERROR) en caso de error
....................
{returns} a data byte from the DSP.
{returns} 0xFFFF (DSP_ERROR) if error. */

UINT16 sb_DSP_read( UINT16 _sb_baseport )
{
    UINT16 i;

	for (i=DSP_TEST_LOOPS; i; i--) {
        if ((INPORT8(DSP_REG_RSTATUS)&DSP_MASK_DATA_AVAIL) != 0)
            return INPORT8(DSP_REG_RDATA);
	}

	return DSP_ERROR;
}

/**********************************************************/
/* resetea el DSP. {devuelve} DSP_ERROR (0xFFFF) en caso de error,
o 0 si todo va bien.
....................
resets the DSP. {returs} DSP_ERROR (0xFFFF) if error, or 0 if
reset is correctly performed */

UINT16 sb_DSP_reset( UINT16 _sb_baseport )
{
    UINT16 i;
    UINT16 data;

    OUTPORT8(DSP_REG_RESET,0x01);
	for (i=DSP_TEST_LOOPS; i; i--)  /* perder tiempo */
		do_nothing();
    OUTPORT8(DSP_REG_RESET,0x00);

	for (i=DSP_TEST_LOOPS; i; i--) {
		data = sb_DSP_read(_sb_baseport);
		if (data==DSP_READY)
			return 0;
	}

	return DSP_ERROR;
}

/**********************************************************/
/* escribe en el dsp el dato o comando enviado en el byte {data}.
La funcion {devuelve} 0 si tiene exito, o DSP_ERROR (0xFFFF) en
caso de error
....................
writes a data byte {data} to the DSP  (data or command).
The function {returns} 0 if succeeds, or DSP_ERROR (0xFFFF) on
error */

UINT16 sb_DSP_write( UINT16 _sb_baseport, UINT8 data )
{
    UINT16 i;

	for (i=DSP_TEST_LOOPS; i; i--) {
        if ((INPORT8(DSP_REG_WSTATUS)&DSP_MASK_BUSY) == 0) {
            OUTPORT8(DSP_REG_WDATA,data);
			return 0;
		}
	}

	return DSP_ERROR;
}

/**********************************************************/
/* {devuelve} el registro de estado de interrupciones de la SB16,
necesario para saber quien ha producido la interrupcion.

	bit 0: DMA 8 bits o MIDI
	bit 1: DMA 16 bits
	bit 2: MPU

Estos bits estan definidos en las mascaras:

DSP_MASK_IRQ_DMA8MIDI
DSP_MASK_IRQ_DMA16
DSP_MASK_IRQ_MPU

el ack de la interrupcion se debe hace en funcion del causante de
ella, leyendo el puerto definido para cada caso como:

DSP_REG_IRQ_ACK_DMA8MIDI
DSP_REG_IRQ_ACK_DMA16
DSP_REG_IRQ_ACK_MPU

Como DMA8 y MIDI usan el mismo bit, no se deben utilizar simultaneamente.

Version DSP >= 4.0
....................
{returns} the SB16 interrupt status register, needed to know the
interrupt reason:

	bit 0: DMA 8  or MIDI
	bit 1: DMA 16
	bit 2: MPU

These bits are defined with this masks (#define):

DSP_MASK_IRQ_DMA8MIDI
DSP_MASK_IRQ_DMA16
DSP_MASK_IRQ_MPU

the interrupt ACK should be send acording to the reason who produced
it. The ACK for each of the three reasons previously defined is sent
reading respectively the port:

DSP_REG_IRQ_ACK_DMA8MIDI
DSP_REG_IRQ_ACK_DMA16
DSP_REG_IRQ_ACK_MPU

As DMA8 and MIDI use the same interrupt status bit, there is no
way to diferentiate them, so you shuld never perform simultaneous DMA8
and MIDI operations.

DSP version >= 4.0 */

UINT8 sb_IRQ_stat( UINT16 _sb_baseport )
{
    OUTPORT8(MIX_REG_ADDR,MIX_ADDR_IRQ_STAT);
    return INPORT8(MIX_REG_DATA);
}

/**********************************************************/
/* envia un ACK al DSP para la interrupcion pendiente, sea del
tipo que sea.
....................
Sends an ACK to the DSP for a received interrupt signal.
The function selects the correct interrupt reason(s) and sends
the correct ACK signal(s)*/

VOID sb_IRQ_ack( UINT16 _sb_baseport )
{
    UINT8 irqstat = sb_IRQ_stat(_sb_baseport);

	if (irqstat & DSP_MASK_IRQ_DMA8MIDI)
        INPORT8(DSP_REG_IRQ_ACK_DMA8MIDI);

	if (irqstat & DSP_MASK_IRQ_DMA16)
        INPORT8(DSP_REG_IRQ_ACK_DMA16);

	if (irqstat & DSP_MASK_IRQ_MPU)
        INPORT8(DSP_REG_IRQ_ACK_DMA16);
}

/**********************************************************/
/* envia un ACK al DSP para la interrupcion de tipo DMA8-MIDI
....................
Sends an ACK to the DSP for an interrupt of tye DMA8-MIDI */

VOID sb_IRQ_ack_dma8midi( UINT16 _sb_baseport )
{
    INPORT8(DSP_REG_IRQ_ACK_DMA8MIDI);
}

/**********************************************************/
/* envia un ACK al DSP para la interrupcion de tipo DMA16
....................
Sends an ACK to the DSP for an interrupt of tye DMA16 */

VOID sb_IRQ_ack_dma16( UINT16 _sb_baseport )
{
    INPORT8(DSP_REG_IRQ_ACK_DMA16);
}

/**********************************************************/
/* envia un ACK al DSP para la interrupcion de tipo MPU
....................
Sends an ACK to the DSP for an interrupt of tye MPU */

VOID sb_IRQ_ack_mpu( UINT16 _sb_baseport )
{
    INPORT8(DSP_REG_IRQ_ACK_DMA16);
}

/**********************************************************/
/* Comando DSP 10h.
Escribe directamente una muestra de 8 bits, enviada en {sample_val}.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 1.xx, 2.0, 2.01x, 3.xx, 4.xx
....................
DSP command 10h.
Directly writes an 8 bit sample {sample_val}.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 1.xx, 2.0, 2.01x, 3.xx, 4.xx */

UINT16 sb_DSP_out8( UINT16 _sb_baseport, UINT8 sample_val )
{
	if (sb_DSP_write(_sb_baseport,0x10) == DSP_ERROR)
		return DSP_ERROR;

	return sb_DSP_write(_sb_baseport,sample_val);
}

/**********************************************************/
/* Comando DSP 14h.
Conversion D/A PCM de 8 bits por DMA (de 8 bits), utilizando el
modo de ciclo unico.  Se reproducen {samples_minus_1}+1 muestras de
8 bits cada una de ellas.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP >= 1.0
....................
DSP command 14h.
PCM 8 bit D/A conversion using DMA 8, in single cicle mode.
The SB will play {sample_minus_1}+1 samples (8 bit each).
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version >= 1.0  */

UINT16 sb_DSP_dma_out8_single( UINT16 _sb_baseport, UINT16 samples_minus_1 )
{
	if (sb_DSP_write(_sb_baseport,0x14) == DSP_ERROR)

		return DSP_ERROR;
    if (sb_DSP_write(_sb_baseport,(UINT8)samples_minus_1) == DSP_ERROR)
		return DSP_ERROR;
    return sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1>>8));
}

/**********************************************************/
/* Comando DSP 1Ch.
Conversion D/A PCM de 8 bits por DMA (de 8 bits), utilizando el
modo de autoinicializacion.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP >= 2.0
....................
DSP command 1Ch.
PCM 8 bit D/A conversion using DMA 8, in autoinit mode.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version >= 2.0  */

UINT16 sb_DSP_dma_out8_auto( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0x1C);
}

/**********************************************************/
/* Comando DSP 20h.
Lee y {devuelve} directamente una muestra de 8 bits.
{devuelve} DSP_ERROR (0xFFFF) si error.
version DSP 1.xx, 2.0, 2.01x, 3.xx, 4.xx
....................
DSP command 20h.
Directly reads and {returns} an 8 bit sample.
{returns} DSP_ERROR (0xFFFF) if error.
DSP version 1.xx, 2.0, 2.01x, 3.xx, 4.xx */

UINT16 sb_DSP_in8( UINT16 _sb_baseport )
{
	if (sb_DSP_write(_sb_baseport,0x20) == DSP_ERROR)
		return DSP_ERROR;

	return sb_DSP_read(_sb_baseport);
}

/**********************************************************/
/* Comando DSP 24h.
Conversion A/D PCM de 8 bits por DMA (de 8 bits), utilizando el
modo de ciclo unico.  Se graban {samples_minus_1}+1 muestras de
8 bits cada una de ellas.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP >= 1.0
....................
DSP command 24h.
PCM 8 bit A/D conversion using DMA 8, in single cicle mode.
The SB will record {sample_minus_1}+1 samples (8 bit each).
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version >= 1.0  */

UINT16 sb_DSP_dma_in8_single( UINT16 _sb_baseport, UINT16 samples_minus_1 )
{
	if (sb_DSP_write(_sb_baseport,0x24) == DSP_ERROR)
		return DSP_ERROR;
    if (sb_DSP_write(_sb_baseport,(UINT8)samples_minus_1) == DSP_ERROR)
		return DSP_ERROR;
    return sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1 >> 8));
}

/**********************************************************/
/* Comando DSP 2Ch.
Conversion A/D PCM de 8 bits por DMA (de 8 bits), utilizando el
modo de autoinicializacion.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP >= 2.0
....................
DSP command 2Ch.
PCM 8 bit A/D conversion using DMA 8, in autoinit mode.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version >= 2.0  */

UINT16 sb_DSP_dma_in8_auto( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0x2C);
}

/**********************************************************/
/* {devuelve} la constante de tiempo a utilizar para tener
la frecuencia de muestreo mas aproximada a {sampling_rate}
si se utilizan {channels} canales (1=mono, 2=stereo)
....................
{returns} the time constant that must be used to select
the sampling rate nearest to {sampling_rate}, if the SBlaster
will play/record {channels} audio channels (1=mono, 2=stereo) */

UINT8 sb_DSP_sr2tc( UINT16 sampling_rate, UINT8 channels )
{
    return ((65536L - 256000000L / ((UINT32)sampling_rate * channels)) >> 8);
}

/**********************************************************/
/* {devuelve} la frecuencia de muestreo resultante de utilizar
la constante de tiempo {time_constant} si se utilizan
{channels} canales (1=mono, 2=stereo)
....................
{returns} the used sampling rate when {time_constant} is used
as the time constant, and the SBlaster will play/record {channels}
audio channels (1=mono, 2=stereo). */

UINT16 sb_DSP_tc2sr( UINT8 time_constant, UINT8 channels )
{
    return (UINT16)((256000000L / (65536L - ((UINT16)time_constant << 8))) /
			channels);
}

/**********************************************************/
/* Similar a sb_DSP_tc2sr(), pero la frecuencia que se
{devuelve} es exacta (un numero real).
Solo si DSP_GET_DOUBLE_RATE esta definido
....................
This function is similar to sb_DSP_tc2sr(), but the {returned}
sampling rate is a more precise value (a floating point number).
This function is defined only if DSP_GET_DOUBLE_RATE is defined,
so as to easily avoid linking floating point math if you don't
need it. */

#ifdef DSP_GET_DOUBLE_RATE

DOUBLE sb_DSP_tc2dsr( UINT8 time_constant, UINT8 channels )
{
	return (1000000.0 / (256 - time_constant) / channels);
}

#endif

/**********************************************************/
/* Comando DSP 40h.
Selecciona la constante de tiempo {time_constant} para la
frecuencia de muestreo en operaciones AD/DA.
Utiliza sb_DSP_sr2tc() para calcular la constante de tiempo
a partir de la frecuencia de muestro (Hz).
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP >= 1.0
....................
DSP command 40h.
Programs the time constant {time_constant} to select the
sampling rate in AD/DA conversion.
Use sb_DSP_sr2tc() to convert the sampling rate (Hz) to time
constant.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version >= 1.0 */

UINT16 sb_DSP_tconst( UINT16 _sb_baseport, UINT8 time_constant )
{
	if (sb_DSP_write(_sb_baseport,0x40) == DSP_ERROR)
		return DSP_ERROR;
	return sb_DSP_write(_sb_baseport,time_constant);
}

/**********************************************************/
/* Comando DSP 41h.
Selecciona la frecuencia de muestreo {sampling_rate} para D/A.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command 41h.
Selects the sampling rate {sampling_rate} for D/A.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 4.xx */

UINT16 sb_DSP_out_rate( UINT16 _sb_baseport, UINT16 sampling_rate )
{
#ifdef __SHAREWARE_VERSION__
	sampling_rate = 1000;
	sampling_rate +=500;
	sampling_rate *= 5;  /* 7500 Hz, shareware */
#endif

	if (sb_DSP_write(_sb_baseport,0x41) == DSP_ERROR)
		return DSP_ERROR;
	if (sb_DSP_write(_sb_baseport,(UINT8)(sampling_rate >> 8)) == DSP_ERROR)
		return DSP_ERROR;
	return sb_DSP_write(_sb_baseport,(UINT8)sampling_rate);
}

/**********************************************************/
/* Comando DSP 42h.
Selecciona la frecuencia de muestreo {sampling_rate} para A/D.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command 42h.
Selects the sampling rate {sampling_rate} for A/D.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 4.xx */

UINT16 sb_DSP_in_rate( UINT16 _sb_baseport, UINT16 sampling_rate )
{
#ifdef __SHAREWARE_VERSION__
	sampling_rate = 1000;
	sampling_rate +=500;
	sampling_rate *= 5;  /* 7500 Hz, shareware */
#endif

	if (sb_DSP_write(_sb_baseport,0x42) == DSP_ERROR)
		return DSP_ERROR;
    if (sb_DSP_write(_sb_baseport,(UINT8)(sampling_rate >> 8)) == DSP_ERROR)
		return DSP_ERROR;
    return sb_DSP_write(_sb_baseport,(UINT8)sampling_rate);
}

/**********************************************************/
/* Comando DSP 48h.
Selecciona la longitud {block_transfer_size_minus_1} en bytes del
bloque de transferencia. El DSP genera una interrupcion cada tal
numero de bytes.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 2.00, 2.01+, 2.xx, 4.xx
....................
DSP command 48h.
Programs the block transfer length in bytes. Send the length-1
in {block_transfer_size_minus_1}. The DSP raises an interrupt
for each block transferred.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 2.00, 2.01+, 2.xx, 4.xx */

UINT16 sb_DSP_blk_size( UINT16 _sb_baseport, UINT16 block_transfer_size_minus_1 )
{
	if (sb_DSP_write(_sb_baseport,0x48) == DSP_ERROR)
		return DSP_ERROR;
    if (sb_DSP_write(_sb_baseport,(UINT8)block_transfer_size_minus_1) == DSP_ERROR)
		return DSP_ERROR;
    return sb_DSP_write(_sb_baseport,(UINT8)(block_transfer_size_minus_1 >> 8));
}

/**********************************************************/
/* Comando DSP 80h.
Silencio en conversion D/A durante {samples_minus_1} muestras.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 1.xx, 2.00, 2.01+, 2.xx, 4.xx
....................
DSP command 80h.
Performs a silence D/A playback of {samples_minus_1} samples.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 1.xx, 2.00, 2.01+, 2.xx, 4.xx */

UINT16 sb_DSP_out_silence( UINT16 _sb_baseport, UINT16 samples_minus_1 )
{
	if (sb_DSP_write(_sb_baseport,0x80) == DSP_ERROR)
		return DSP_ERROR;
    if (sb_DSP_write(_sb_baseport,(UINT8)samples_minus_1) == DSP_ERROR)
		return DSP_ERROR;
    return sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1 >> 8));
}

/**********************************************************/
/* Comando DSP 90h.
Conversion D/A PCM de 8 bits de alta velocidad por DMA (de 8 bits),
utilizando el modo de autoinicializacion.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 2.01+, 3.xx
....................
DSP command 90h.
High speed DMA D/A conversion (PCM 8 bit), using autoinitialize
mode.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 2.01+, 3.xx */

UINT16 sb_DSP_dma_out8hs_auto( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0x90);
}

/**********************************************************/
/* Comando DSP 91h.
Conversion D/A PCM de 8 bits de alta velocidad por DMA (de 8 bits),
utilizando el modo de ciclo unico.  Se reproducen tantas muestras como
se hayan indicado con el comando 48h.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 2.01+, 3.xx
....................
DSP command 91h.
High speed DMA D/A conversion (PCM 8 bit), using single ciclye mode.
The number of samples to play must be previously programmed using
command 48h.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 2.01+, 3.xx */

UINT16 sb_DSP_dma_out8hs_single( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0x91);
}

/**********************************************************/
/* Comando DSP 98h.
Conversion A/D PCM de 8 bits de alta velocidad por DMA (de 8 bits),
utilizando el modo de autoinicializacion.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 2.01+, 3.xx
....................
DSP command 98h.
High speed DMA A/D conversion (PCM 8 bit), using autoinitialize
mode.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 2.01+, 3.xx */

UINT16 sb_DSP_dma_in8hs_auto( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0x98);
}

/**********************************************************/
/* Comando DSP 99h.
Conversion A/D PCM de 8 bits de alta velocidad por DMA (de 8 bits),
utilizando el modo de ciclo unico.  Se graban tantas muestras como
se hayan indicado con el comando 48h.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 2.01+, 3.xx
....................
DSP command 99h.
High speed DMA A/D conversion (PCM 8 bit), using single ciclye mode.
The number of samples to record must be previously programmed using
command 48h.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 2.01+, 3.xx */

UINT16 sb_DSP_dma_in8hs_single( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0x99);
}

/**********************************************************/
/* Comando DSP A0h.
Selecciona modo mono.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 3.xx
....................
DSP command A0h.
Selects mono mode.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 3.xx */

UINT16 sb_DSP_mono( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xA0);
}

/**********************************************************/
/* Comando DSP A8h.
Selecciona modo estereo.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 3.xx
....................
DSP command A8h.
Selects stereo mode.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 3.xx */

UINT16 sb_DSP_stereo( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xA8);
}

/**********************************************************/
/* Comando DSP Bxh.
Programa modo AD/DA de 16 bits (muestras de 16 bits, DMA de 16 (o 8) bits.
{input} = FALSE para DA, TRUE para AD.
{auto_init} = FALSE para ciclo unico, TRUE para autoinicializacion.
{fifo} = FALSE para off, TRUE para on.  FIFO se reinicializa con cada
nuevo comando (????).
{stereo} = FALSE para mono, TRUE para estereo.
{signed_val} = FALSE para muestras sin signo, TRUE para muestras con signo.
{samples_minus_1} es el numero de muestras menos 1 que deben grabarse o
reproducirse en modo de ciclo unico. En modo de autoinicializacion indica
el numero de muestras menos 1 cada cuanto se tiene que generar la
interrupcion.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command set Bxh.
Programs 16 bits AD/DA (16 bit samples, DMA channel may be 16 or 8 bit).
{input} = FALSE for DA (play), TRUE for AD (rec) conversion.
{auto_init} = FALSE for single cicle, TRUE for autoinit.
{fifo} = FALSE for fifo-off,  TRUE for fifo-on. The FIFO is re-initialized
with each command.
{stereo} = FALSE for mono, TRUE for stereo.
{signed_val} = FALSE for unsigned int samples, TRUE for signed int samples.
{samples_minus_1} is the number of samples-1 to play/record on single cicle
mode. On autoinit mode, it is the block length-1 (SB will raise an
interrupt signal whenever it processes a full block).
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 4.xx */

UINT16 sb_DSP_16( UINT16 _sb_baseport, BOOL input, BOOL auto_init,
        BOOL fifo, BOOL stereo, BOOL signed_val, UINT16 samples_minus_1 )
{
	if (sb_DSP_write(_sb_baseport,0xB0 | (input?0x08:0x00) | (auto_init?0x04:0x00) |
			(fifo?0x02:0x00)) == DSP_ERROR)
		return DSP_ERROR;

	if (sb_DSP_write(_sb_baseport,(stereo?0x20:0x00) | (signed_val?0x10:0x00)) ==
			DSP_ERROR)
		return DSP_ERROR;

    if (sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1)) == DSP_ERROR)
		return DSP_ERROR;

    return sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1 >> 8));
}

/**********************************************************/
/* Comando DSP Cxh.
Programa modo AD/DA de 8 bits (muestras de 8 bits, DMA de 8 bits.
{input} = FALSE para DA, TRUE para AD.
{auto_init} = FALSE para ciclo unico, TRUE para autoinicializacion.
{fifo} = FALSE para off, TRUE para on.  FIFO se reinicializa con cada
nuevo comando (????).
{stereo} = FALSE para mono, TRUE para estereo.
{signed_val} = FALSE para muestras sin signo, TRUE para muestras con signo.
{samples_minus_1} es el numero de muestras menos 1 que deben grabarse o
reproducirse en modo de ciclo unico. En modo de autoinicializacion indica
el numero de muestras menos 1 cada cuanto se tiene que generar la
interrupcion.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command set Cxh.
Programs 8 bits AD/DA (16 bit samples, DMA 8 bits).
{input} = FALSE for DA (play), TRUE for AD (rec) conversion.
{auto_init} = FALSE for single cicle, TRUE for autoinit.
{fifo} = FALSE for fifo-off,  TRUE for fifo-on. The FIFO is re-initialized
with each command.
{stereo} = FALSE for mono, TRUE for stereo.
{signed_val} = FALSE for unsigned char samples, TRUE for signed char samples.
{samples_minus_1} is the number of samples-1 to play/record on single cicle
mode. On autoinit mode, it is the block length-1 (SB will raise an
interrupt signal whenever it processes a full block).
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 4.xx */

UINT16 sb_DSP_8( UINT16 _sb_baseport, BOOL input, BOOL auto_init,
        BOOL fifo, BOOL stereo, BOOL signed_val, UINT16 samples_minus_1 )
{
	if (sb_DSP_write(_sb_baseport,0xC0 | (input?0x08:0x00) | (auto_init?0x04:0x00) |
			(fifo?0x02:0x00)) == DSP_ERROR)
		return DSP_ERROR;

	if (sb_DSP_write(_sb_baseport,(stereo?0x20:0x00) | (signed_val?0x10:0x00)) ==
			DSP_ERROR)
		return DSP_ERROR;

    if (sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1)) == DSP_ERROR)
		return DSP_ERROR;

    return sb_DSP_write(_sb_baseport,(UINT8)(samples_minus_1 >> 8));
}

/**********************************************************/
/* Comando DSP D0h.
Pausa en AD/DA de 8 bits.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 1.xx, 2.00, 2.01+, 3.xx, 4.xx
....................
DSP command D0h.
Pause 8 bits AD/DA conversion.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 1.xx, 2.00, 2.01+, 3.xx, 4.xx */

UINT16 sb_DSP_8_pause( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD0);
}

/**********************************************************/
/* Comando DSP D1h.
Enciende speaker (necesario para escuchar D/A en DSPs anteriores
a 4.xx).
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 1.xx, 2.00, 2.01+, 3.xx, 4.xx
....................
DSP command D1h.
Switch on SB speaker. You must send this command to enable D/A
conversion on DSPs previous to 4.xx.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 1.xx, 2.00, 2.01+, 3.xx, 4.xx */

UINT16 sb_DSP_speaker_on( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD1);
}

/**********************************************************/
/* Comando DSP D3h.
Apaga speaker.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 1.xx, 2.00, 2.01+, 3.xx, 4.xx
....................
DSP command D3h.
Switch off SB speaker.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 1.xx, 2.00, 2.01+, 3.xx, 4.xx */

UINT16 sb_DSP_speaker_off( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD3);
}

/**********************************************************/
/* Comando DSP D4h.
Continua AD/DA de 8 bits, despues de poner en modo pausa con
comando D0h.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 1.xx, 2.00, 2.01+, 3.xx, 4.xx
....................
DSP command D4h.
Continue 8 bit AD/DA conversion if we have previously sent
the pause command (D0h) to the DSP.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 1.xx, 2.00, 2.01+, 3.xx, 4.xx */

UINT16 sb_DSP_8_continue( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD4);
}

/**********************************************************/
/* Comando DSP D5h.
Pausa en AD/DA de 16 bits.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command D5h.
Pause 16 bits AD/DA conversion.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 4.xx */

UINT16 sb_DSP_16_pause( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD5);
}

/**********************************************************/
/* Comando DSP D6h.
Continua AD/DA de 16 bits, despues de poner en modo pausa con
comando D5h.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command D6h.
Continue 16 bit AD/DA conversion if we have previously sent
the pause command (D5h) to the DSP.
{returns} DSP_ERROR (0xFFFF) if error, or 0 if succeeds.
DSP version 4.xx */

UINT16 sb_DSP_16_continue( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD6);
}

/**********************************************************/
/* Comando DSP D8h.
{devuelve} el estado del speaker. 0 para off, 1 para on.
{devuelve} DSP_ERROR (0xFFFF) si error.
version DSP 2.00, 2.01+, 3.xx, 4.xx
....................
DSP command D8h.
{returns} the SB internal speaker status. 0=off, 1=on.
{returns} DSP_ERROR (0xFFFF) on error.
DSP version 2.00, 2.01+, 3.xx, 4.xx */

UINT16 sb_DSP_speaker( UINT16 _sb_baseport )
{
    UINT16 spk;

	if (sb_DSP_write(_sb_baseport,0xD8))
		return DSP_ERROR;


	spk = sb_DSP_read(_sb_baseport);
	if (spk == DSP_ERROR)
		return DSP_ERROR;
	else if (spk == 0xFF)
		return 1;

	return 0;
}

/**********************************************************/
/* Comando DSP D9h.
Al final de la transferencia en curso, se abandono el modo de
autoinicializacion AD/DA de 16 bits, y se detiene el proceso.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
Programs the DSP so that when the current DMA transfer ends,
the cards exits the autoinitialize mode, stopping the AD/DA 16 bit
conversion.
The function {returns} 0 if succeeds, or DSP_ERROR (0xFFFF) on
error.
DSP version 4.xx */


UINT16 sb_DSP_16_exit( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xD9);
}

/**********************************************************/
/* Comando DSP DAh.
Al final de la transferencia en curso, se abandona el modo de
autoinicializacion AD/DA de 8 bits, y se detiene el proceso.
{devuelve} DSP_ERROR (0xFFFF) si error, o 0 si tiene exito.
version DSP 4.xx
....................
DSP command DAh
Programs the DSP so that when the current DMA transfer ends,
the cards exits the autoinitialize mode, stopping the AD/DA 8 bit
conversion.
The function {returns} 0 if succeeds, or DSP_ERROR (0xFFFF) on
error.
DSP version 4.xx  */

UINT16 sb_DSP_8_exit( UINT16 _sb_baseport )
{
	return sb_DSP_write(_sb_baseport,0xDA);
}

/**********************************************************/
/* Comando DSP E1h.
{devuelve} la version del DSP, o 0 en caso de error.
version DSP >= 1.0
....................
DSP command E1h.
{returns} the DSP version, or 0 on error.
DSP version >= 1.0 */

UINT16 sb_DSP_version( UINT16 _sb_baseport )
{
    UINT16 major, minor;

	if (sb_DSP_write(_sb_baseport,0xE1) == DSP_ERROR)
		return 0x0000;

	major = sb_DSP_read(_sb_baseport);
	if (major == DSP_ERROR)
		return 0x0000;

	minor = sb_DSP_read(_sb_baseport);
	if (minor == DSP_ERROR)
		return 0x0000;

	return (major << 8) | minor;
}

/**********************************************************/

