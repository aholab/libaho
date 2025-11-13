/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ lpc10_c1.c
Nombre paquete............... LPC10E
Lenguaje fuente.............. C
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.0    10/03/97  Borja     codificacion inicial

======================== Contenido ========================
codificador LPC10E a 2400bps. Basado en unos fuentes de
dominio publico. Esta es la documentacion para los
objetos definidos en lpc10_c1.c, lpc10_d1.c, lpc10_c2.c
y lpc10_d2.c. Los prototipos y definiciones necesarias
para utilizar todo esto estan en lpc10.h.

NOTA IMPORTANTE: por ahora SOLO se soporta un unico objeto de
cada tipo (un solo CLPC10_2400, etc).

2 modos de funcionamiento: como bitstream o como datagrama.
El bitstream simula un flujo de bits, orientado a simular
el funcionamiento del sistema. Cada trama es un array
de 54 enteros, cada entero representa un bit. Se utiliza
el tipo LPC10_2400_idata para representar una trama.

El datagrama esta mas empaquetadito, orientado a un uso
practico del codec. Cada trama utiliza 7 bytes en los
que se empaquetan todos los bits codificados. Se utiliza
el tipo LPC10_dgram_data para representar una trama.
Ademas se ha eliminado parte de la codificacion de canal
(codigos de proteccion hamming, etc), en vistas a la
utilizacion como datagrama: o todo o nada.

Los tipos de objetos son:

CLPC10_2400 : codificador bitstream
DLPC10_2400 : decodificador bitstream
CLPC10_DGRAM : codificador datagrama
DLPC10_DGRAM : decodificador datagrama
PTHL10 : implementa un metodo de calculo de pitch en base a
este codec.

Cada tipo tiene su constructor y destructor (ej. para CLPC10_2400,
clpc10_2400_construct() y clpc10_2400_destruct()).

Ademas tienen un metodo ???_reset() para reinicializar el sistema
(estado "home", memorias reinicializadas a cero).

Para codificar con el codec bitstream se utiliza clpc10_2400_icode()
o tambien clpc10_2400_icode_i16(). La primera recibe las muestras
de audio como floats entre -1 y 1. La segunda utiliza muestras PCM
de 16 bits. Ambas {devuelven} un puntero a una trama LP10_2400_idata
(internamente almacenada).

Para decodificar un bitstream se utiliza dlpc10_2400_idecode() o
tambien dlpc10_2400_idecode_i16(), la primera decodifica las muestras
en formato float entre -1 y 1, y la segunda como muestras de 16 bits.
Ambas reciben una trama LPC10_2400_idata y {devuelven} el numero N
de muestras que se generan. IMPORTANTE: aunque LPC10_LFRAME muestras
de entrada se codifican en una trama LPC10_2400_idata, no sucede
lo contrario: una trama codificada no se decodifica/expande en
LPC10_LFRAME (a veces mas, a veces menos, aunque la media
si que es LPC10_LFRAME), por eso estas funciones {devuelven} el
numero N de muestras que se han obtenido. Para obtener el vector
de audio decodificado, inmediatamente despues de llamar a
dlpc10_2400_idecode() (o dlpc10_2400_idecode_i16()) llamaremos
a dlpc10_2400_outvec() (o dlpc10_2400_outvec_i16()), que
{devuelven} un puntero a un buffer interno que contiene las N
muestras generadas en la ultima llamada a idecode. El numero
N de muestras devueltas esta entre 0 y LPC10_DOUTMAX.

Con el codificador de datagramas, similar al de bitstream pero
con tramas codificadas de tipo LPC10_dgram_data. Para codificar
se utiliza clpc10_dgram_code() o clpc10_dgram_code_i16() y
para decodificar dlpc10_dgram_decode() o dlpc10_dgram_decode_i16(),
con dlpc10_dgram_outvec() o dlpc10_dgram_outvec_i16() para obtener
el correspondiente vector.

El codificador de bitstream genera tramas LPC10_2400_idata,
compuestas por 54 enteros, cada uno representando un bit
de informacion transmitida. Tambien se puede compactar
para almacenar dicha informacion (aunque para eso es mejor
utilizar el modelo orientado a datagrama) utilizando
la funcion clpc10_2400_pack(), que recibe una trama
codificada LPC10_2400_idata y rellena una trama codificada
compactada LPC10_2400_data. La funcion dlpc10_2400_unpack() se utiliza
para hacer el proceso contrario (expansion de la trama codificada
compactada a trama codificada LPC10_2400_idata).

En cuanto al tipo PTHL10, tambien tiene un constructor,
destructor y reset que funcionan como en los anteriores.
Ademas tiene la funcion pthl10_get_i16() que para cada
trama de LPC10_LFRAME muestras de tipo PCM16, {devuelve}
el valor de pitch calculado utilizando los algoritmos del
codec LPC10e. Hay que tener en cuenta el retardo: el valor
devuelto no corresponde a la trama recien enviada, sino a
una trama anterior, con un retardo de ??? tramas.

CODEC DE BAJO NIVEL:
Se proporcionan estas funciones para utilizar el codec como
un algoritmo de analisis de senyal, y obtener todos los
parametros: pitch, energia, voice/unvoice, rcs. Es una
generalizacion de PTHL10. Estas son las funciones:
parametros del codec

- c_lpc10_ini() para inicializar el codificador.
- d_lpc10_ini() para inicializar el decodificador.
- c_lpc10_code() para codificar trama de audio (FLOAT*)
- c_lpc10_code_i16() para codificar trama de audio (PCM16*)
- c_lpc10_peek() para obtener los parametros de la codificacion,
que son voice[2], pitch, rms, rc[10]. Atencion que hay un
retardo de tres tramas (es decir, se obtienen los parametros de
la trama de audio que se envio hace tres tramas).
- d_lpc10_decode() para recuperar audio a partir de los parametros
que c_lpc10_peek() genera. No se genera una trama completa (180
muestras) sino cualquier valor entre 0 y LPC10_DOUTMAX. La funcion
{devuelve} el numero N de muestras generado. Atencion que hay
un retraso de una trama (LPC10_LFRAME muestras = 180 muestras).
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "lpc10.h"
#include "c_lpc10.h"
#include "sr_lpc10.h"

/**********************************************************/

BOOL g_used_c_2400 = FALSE;
INDEX g_ibits[54];

/**********************************************************/

CLPC10_2400 *clpc10_2400_construct( VOID )
{
	if (g_used_c_2400) {
		fprintf(stderr,"\nPor ahora no soporto mas de una estructura CLPC10_2400!\7\n");
		exit(1);
	}

	g_used_c_2400 = TRUE;

	clpc10_2400_reset((CLPC10_2400*)1);
	return (CLPC10_2400*)1;
}

/**********************************************************/

VOID clpc10_2400_destruct( CLPC10_2400 *c )
{
	(void)c;  /* avoid "unused parameter" warning */
	g_used_c_2400 = FALSE;
}

/**********************************************************/

VOID clpc10_2400_reset( CLPC10_2400 *c )
{
	(void)c;  /* avoid "unused parameter" warning */
	coder_ini();
	send_2400_ini();
}

/**********************************************************/

LPC10_2400_idata *clpc10_2400_icode( CLPC10_2400 *c, FLOAT speech[LPC10_LFRAME] )
{
	(void)c;  /* avoid "unused parameter" warning */
/* shift sample and other buffers */
	shift();
/* prepare and copy input buffer */
	framein(speech);
/* analyze frame */
	analys();
/* channel encode and write */
	send_2400_i(CODED_VOICE, CODED_PITCH, CODED_RMS, CODED_RC, g_ibits);

	return (LPC10_2400_idata*)g_ibits;
}

/**********************************************************/

LPC10_2400_idata *clpc10_2400_icode_i16( CLPC10_2400 *c, INT16 speech[LPC10_LFRAME] )
{
	(void)c;  /* avoid "unused parameter" warning */
/* shift sample and other buffers */
	shift();
/* prepare and copy input buffer */
	framein_i16(speech);
/* analyze frame */
	analys();
/* channel encode and write */
	send_2400_i(CODED_VOICE, CODED_PITCH, CODED_RMS, CODED_RC, g_ibits);

	return (LPC10_2400_idata*)g_ibits;
}

/**********************************************************/

VOID clpc10_2400_pack( LPC10_2400_idata *in, LPC10_2400_data *o)
{
	INDEX i,j,n;
	BYTE b;

	for (i=0; i<7; i++) {
		b=0;
		for (j=0; j<8; j++) {
			n=i*8+j;
			if (n>=(INDEX)sizeof(LPC10_2400_idata)) break;
			b = ((*in)[n]?1:0) | (b<<1);
		}
		(*o)[i]=b;
	}
}

/**********************************************************/
