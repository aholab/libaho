/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ CODEC.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... definicion de tipos fundamentales
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.1    04/03/97  Borja     quitar warning (inicializar variable inocente)
1.1.0    27/08/95  Borja     ADPCM ZyXEL.
1.0.0    23/07/95  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Modulos para la codificacion/decodificacion de audio PCM
con diferentes formatos de compresion.

Estos algoritmos necesitan almancenar informacion de estado
y de control que les permita continuar el proceso en llamadas
sucesivas al algoritmo con bloques de datos del bitstream a
procesar. Esta informacion de contexto se almacena en una
estructura de control adecuada a cada caso.

Antes de comenzar cualquier proceso de codificacion o
decodificacion, se debe inicializar una estructura de control
con la funcion ???_reset(). Una vez inicializada, se pasara
esta estructura a ???_code() o ???_decode() junto con el
bitstream a procesar. En sucesivas llamadas al algoritmo con
nuevos bloques de datos de un mismo bitstream, deberemos pasar
la misma estructura de control.

No se debe utilizar la misma estructura de control para hacer
???_code() y ???_decode(). Son procesos independientes y
deben utilizar estructuras diferentes. Una vez terminado con
???_code(), podremos utilizar la misma estructura para hacer
???_decode(), aunque deberemos reinicializarla con ???_reset().

La senyal descomprimida esta en formato PCM 16 bits. El
bitstream es una array de valores INT16 (1 word con signo,
2 bytes). Cada word es una muestra. Los bloques de datos
enviados al codificador o recibidos del decodificador son
vectores de elementos INT16 (pINT16). No se soporta
bitstreams stereo (entrelazado L-R de muestras). El bloque
de entrada debe estar alineado a una muestra (no vale empezar
con el segundo byte de una muestra).

La senyal comprimida tiene un formato dependiente del codec
utilizado. Los bloques de datos recibidos del codificador o
enviados al codificador son arrays genericos de bytes, vectores
de elementos UINT8 (pUINT8). El bloque codificado de entrada
debe estar alineado al comienzo de una muestra (o trama): ver
la explicacion sobre CGRAIN abajo.

Dado un bloque de muestras PCM16 formado por {d} muestras INT16,
la funcion (realmente macro)  ???_d2c(d) {devuelve} el numero
de bytes que ocupa el bitstream codificado. Puesto que se trabaja
a nivel de bit (muestra comprimida < 8 bits) en general puede suceder
que algun bit del ultimo byte quede sin utilizar.

Dado un bitstream codificado de {c} bytes, la funcion (macro)
????_c2d(c)  {devuelve} el numero de muestras INT16 que ocupa
el bloque decodificado. En general algun bit final en el bitstream
codificado no sea suficiente para decodificar una muestra y por
tanto se ignora.

La informacion de contexto almacenada entre una llamada y otra
al algoritmo es minima y no soporta un bitstream continuo. Como
ya se ha dicho, puede suceder que los ultimos bits del bitstream
comprimido no se utilizen, bien porque no hay muestras suficientes
(coder) o bien porque no hay suficientes para una muestra (decoder).
Para que al codificar se obtenga un bitstream continuo sin bits
desaprovechados, deberemos codificar tramas de longitud multiplo
de ????_DGRAIN (muestras, UINT16).  Para que al decodificar no se
ignoren bits finales deberemos decodificar bitstreams de longitud
multiplo de ????_CGRAIN (bytes, UINT8).

Un ejemplo para aclarar esto, utilizando por decir algo un codec
ADPCM-2 (2 bits/muestra). Si llamamos a code() con un vector de
6 muestras, el bitstream codificado ocupara 2 bytes, pero los 4
ultimos bits del segundo byte estaran sin utilizar:

.          11 22 33 44   55 66 -- --          n = usado
.          <--Byte1-->   <--Byte2-->          - = sin usar

Llamadas sucesivas a code() con bloques de 6 muestras produciran
por tanto un bitstream discontinuo, en el que hay bits sin aprovechar:

.     11223344 5566----  11223344 5566----  11223344 5566---- ...

Aparte del derroche de bits, esto obliga a que el decodificador trabaje
con bloques de 2 bytes que encajen con los anteriores, para evitar que
utilice los bits no utilizados.

Por ello, en este caso conviene utilizar en el codificador bloques de
muestras de longitud multiplo de 4 (DGRAIN), que produce siempre bytes
completamente aprovechados. En decodificacion podremos utilizar bloques
de bytes de cualquier longitud (== multiplo de 1 byte, CGRAIN) pues en
un byte siempre hay informacion suficiente para decodificar 4 muestras.
Si por ejemplo utilizasemos un ADPCM-3 entonces CGRAIN seria 3 bytes,
pues es la longitud en bytes necesaria que nos asegura un numero entero
de muestras:

.            11122233 34445556 66777888
.             byte0    byte1    byte2

Ademas tendremos que asegurarnos SIEMPRE de que la entrada al
decodificador esta sincronizada al comienzo de una muestra.
En el ejemplo anterior el decodificador debe recibir un bloque que
comience en el byte0 o byte3 o byte6, etc; si comenzase en el
byte1, o byte2 funcionaria mal.  Para asegurarnos que esto
es siempre asi, basta con trabajar siempre con bloques de longitud
multiplo de CGRAIN.

Se cumple:   ???_CGRAIN = ???_d2c(???_DGRAIN)


Codec ADPCM  Intel/DVI
----------------------
Es el function-set adpcm_idvi_???().
Algoritmo extraido de  IMA Compatibility Project Proceedings,
Vol 2, n. 2, May 1992.
Utiliza coma fija (numeros enteros) y es bastante mas rapido
que el algoritmo del estandar del CCITT. No son compatibles!

Codec ADPCM ZyXEL-2
-------------------
Function set adpcm_zy2_???().
Utilizado por los modem-voz ZyXEL, 2 bits por muestra. Esta
optimizado para trabajar a una frecuencia de muestreo de
9600 m/s.

Codec ADPCM ZyXEL-3
-------------------
Function set adpcm_zy3_???().
Utilizado por los modem-voz ZyXEL, 3 bits por muestra. Esta
optimizado para trabajar a una frecuencia de muestreo de
9600 m/s.
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#include "codec.h"

/**********************************************************/
/* Intel ADPCM step index variation table */
PRIVATE INT _adpcm_idvi_idxtab[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

/* Intel ADPCM step sizes */
PRIVATE INT16 _adpcm_idvi_steptab[89] = {
	7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
	19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
	130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/* <DOC> */
/**********************************************************/
/* Inicializa la estructura de tipo ADPCM_IDVI apuntada
por {a} a estado de reposo inicial. Se debe llamar a esta
funcion antes de comenzar el proceso de codificacion o
decodificacion ADPCM Intel/DVI.
Una vez inicializada la estructura, utilizaremos adpcm_idvi_code()
o adpcm_idvi_decode() para ir codificando/decodificando un bitstream.
No mezclar la misma estructura {a} para codificar y/o decodificar
bitstreams diferentes!  Utilizar una estructura diferente para cada
bitstream. */

PUBLIC SPL_VOID XAPI adpcm_idvi_reset( ADPCM_IDVI * a )
/* </DOC> */
{
	a->valprev = 0;
	a->index = 0;
}

/* <DOC> */
/**********************************************************/
/* Codificador ADPCM Intel/DVI. Codifica un bitstream
PCM 16bit_lineal/muestra a ADPCM 4bits/muestra.
- En {a} se envia un puntero a la estructura ADPCM_IDVI necesaria
para almanenar informacion de estado entre una llamada y otra a
esta funcion. Antes de utilizar esta funcion por primera vez
se debe inicializar la estructura con adpcm_idvi_reset(). Luego
ya podemos ir llamando a esta funcion tantas veces como se quiera.
Cada vez que se quiera codificar un bitstream nuevo se debe volver
a llamar a adpcm_idvi_reset() para reinicializar el proceso.
- En {v16} recibe un puntero a un bloque de muestras PCM16.
- En {v8} recibe un puntero a un buffer para salvar muestras ADPCM4.
- El numero de muestras a codificar se envia en {nd}.

En el vector {v16} el usuario debe enviar {nd} muestras de tipo
INT16. En el vector {v8} el usuario debe enviar un buffer de
{nd}/2 bytes (UINT8) si {nd} es par o de {nd}/2+1 bytes si {nd} es
impar (la funcion adpcm_idvi_d2c(nd) {devuelve} directamente
el valor adecuado).

La funcion codifica las {nd} muestras recibidas en {v16} y mete
el resultado en {v8}. Si {nd} es impar, los ultimos cuatro bits
del ultimo byte de {v8} quedan sin utilizar y se inicializan a 0.

No mezclar llamadas a adpcm_idvi_code() con adpcm_idvi_decode() sobre
la misma estructura {a}!!.

Formato del bitstream Intel/DVI ADPCM4:
- ADPCM4: array de valores UINT8 (1 byte). Cada uno de estos bytes
almacena 2 'muestras' ADPCM de 4 bits. Los 4 bits mas significativos
forman la primera muestra, y los 4 menos significativos forman la
muestra siguiente. */

PUBLIC SPL_VOID XAPI adpcm_idvi_code( ADPCM_IDVI * a, pINT16 v16, pUINT8 v8, SPL_INT nd )
/* </DOC> */
{
	UINT8 sign; /* Current adpcm sign bit */
	UINT8 delta; /* Current adpcm output value */
	INT16 diff; /* Difference between val and valprev */
	INT16 step; /* Stepsize */
	INT16 valpred; /* Predicted output value */
	INT32 lvalpred; /* Predicted output value (before trunc. to int) */
	INT16 vpdiff; /* Current change to valpred */
	INT index; /* Current step change index */
	UINT8 outputbuffer;	/* place to keep previous 4-bit value */
	BOOL bufferstep; /* toggle between outputbuffer/output */

	valpred = a->valprev;
	index = a->index;
	step = _adpcm_idvi_steptab[index];
	outputbuffer = 0;
	bufferstep = TRUE;

	for ( ; nd > 0 ; nd-- ) {
		/* Step 1 - compute difference with previous value */
		diff = (*(v16++)) - valpred;
		if (diff<0) {
			sign = 0x8;
			diff = -diff;
		}
		else
			sign = 0;

		/* Step 2 - Divide and clamp
		Note:
		This code *approximately* computes:
			delta = diff*4/step;
			vpdiff = (delta+0.5)*step/4;
		but in shift step bits are dropped. The net result of this is
		that even if you have fast mul/div hardware you cannot put it to
		good use since the fixup would be too expensive. */
		vpdiff = (step >> 3);
		if ( diff >= step ) {
			delta = 0x4;
			diff -= step;
			vpdiff += step;
		}
		else
			delta = 0;
		step >>= 1;
		if ( diff >= step  ) {
			delta |= 0x2;
			diff -= step;
			vpdiff += step;
		}
		step >>= 1;
		if ( diff >= step ) {
			delta |= 0x1;
			vpdiff += step;
		}

		/* Step 3 - Update previous value */
		if (sign)
			lvalpred = (INT32)valpred - (INT32)vpdiff;
		else
			lvalpred = (INT32)valpred + (INT32)vpdiff;

		/* Step 4 - Clamp previous value to 16 bits */
		if (lvalpred > 32767)
			valpred = 32767;
		else if (lvalpred < ((INT32)-32768L))
			valpred = ((INT16)-32768L);
		else
			valpred = (INT16)lvalpred;

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;

		index += _adpcm_idvi_idxtab[delta];
		if (index < 0)
			index = 0;
		if (index > 88)
			index = 88;
		step = _adpcm_idvi_steptab[index];

		/* Step 6 - Output value */
		if (bufferstep)
			outputbuffer = (delta << 4) /*& 0xf0*/;
		else
			(*(v8++)) = (delta /*& 0x0f*/) | outputbuffer;
		bufferstep = !bufferstep;
	}

	/* Output last step, if needed */
	if (!bufferstep)
		(*(v8++)) = outputbuffer;

	/* Save status for next call to coder */
	a->valprev = valpred;
	a->index = index;
}

/* <DOC> */
/**********************************************************/
/* Decodificador ADPCM Intel/DVI. Decodifica un bitstream
ADPCM 4bits/muestra a PCM 16bit_lineal/muestra.
- En {a} se envia un puntero a la estructura ADPCM_IDVI necesaria
para almanenar informacion de estado entre una llamada y otra a
esta funcion. Antes de utilizar esta funcion por primera vez
se debe inicializar la estructura con adpcm_idvi_reset(). Luego
ya podemos ir llamando a esta funcion tantas veces como se quiera.
Cada vez que se quiera decodificar un bitstream nuevo se debe volver
a llamar a adpcm_idvi_reset() para reinicializar el proceso.
- En {v8} recibe un puntero a un bloque de muestras ADPCM4.
- En {v16} recibe un puntero a un buffer para salvar muestras PCM16.
- El numero de muestras a decodificar se envia en {nd}.

En el vector {v8} el usuario debe enviar {nd} muestras de tipo
ADPCM4, es decir, {nd}/2 bytes (UINT8) si {nd} es par o {nd}/2+1
bytes si {nd} es impar (la funcion adpcm_idvi_d2c(nd) {devuelve}
directamente el valor adecuado). En el vector {v16} el usuario
debe enviar un buffer de {nd} elementos de tipo UINT16.

La funcion decodifica las {nd} muestras recibidas en {v8} y mete
el resultado en {v16}. Si {nd} es impar, los ultimos cuatro bits
del ultimo byte de {v8} no se decodifican.

No mezclar llamadas a adpcm_idvi_code() con adpcm_idvi_decode() sobre
la misma estructura {a}!!.

Ver adpcm_idvi_code() para el formato del bitstream Intel/DVI ADPCM4 */

PUBLIC SPL_VOID XAPI adpcm_idvi_decode( ADPCM_IDVI * a, pUINT8 v8, pINT16 v16, SPL_INT nd )
/* </DOC> */
{
	UINT8 sign; /* Current adpcm sign bit */
	UINT8 delta; /* Current adpcm output value */
	INT16 step; /* Stepsize */
	INT16 valpred; /* Predicted output value */
	INT32 lvalpred; /* Predicted output value (before trunc. to int) */
	INT16 vpdiff; /* Current change to valpred */
	INT index; /* Current step change index */
	UINT8 inputbuffer; /* place to keep next 4-bit value */
	BOOL bufferstep; /* toggle between inputbuffer/output */

	valpred = a->valprev;
	index = a->index;
	step = _adpcm_idvi_steptab[index];
	bufferstep = FALSE;
	inputbuffer = 0;

	for ( ; nd > 0 ; nd-- ) {
		/* Step 1 - get the delta value */
		if (bufferstep)
			delta = inputbuffer & 0xf;
		else {
			inputbuffer = (*(v8++));
			delta = (inputbuffer >> 4) /*& 0xf*/;
		}
		bufferstep = !bufferstep;

		/* Step 2 - Find new index value (for later) */
		index += _adpcm_idvi_idxtab[delta];
		if (index < 0)
			index = 0;
		if (index > 88)
			index = 88;

		/* Step 3 - Separate sign and magnitude */
		sign = delta & 0x8;
		delta = delta & 0x7;

		/* Step 4 - Compute difference and new predicted value
		Computes 'vpdiff = (delta+0.5)*step/4', but see comment
		in adpcm_coder. */
		vpdiff = step >> 3;
		if (delta & 0x4)
			vpdiff += step;
		if (delta & 0x2)
			vpdiff += step>>1;
		if (delta & 0x1)
			vpdiff += step>>2;

		if (sign)
			lvalpred = (INT32)valpred - (INT32)vpdiff;
		else
			lvalpred = (INT32)valpred + (INT32)vpdiff;

		/* Step 5 - clamp output value */
		if (lvalpred > 32767)
			valpred = 32767;
		else if (lvalpred < ((INT32)-32768L))
			valpred = ((INT16)-32768L);
		else
			valpred = (INT16)lvalpred;

		/* Step 6 - Update step value */
		step = _adpcm_idvi_steptab[index];

		/* Step 7 - Output value */
		(*(v16++)) = valpred;
	}

	a->valprev = valpred;
	a->index = index;
}

/**********************************************************/
/* macros internos utilizados por adpcm_zy2_???() */

#define ZY2_K0 0x3800
#define ZY2_K1 0x5600

#define ZY2_ADAPT(data, bsign, bmag) { \
	if (data&bsign) {  /* decrece */  \
		if (data&bmag) { /* mag=1 */ \
			a->ExtMax -= (a->Delta + (a->Delta>>1)); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY2_K1+8192)>>14); \
		} \
		else { /* mag=0 */ \
			a->ExtMax -= (a->Delta>>1); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY2_K0+8192)>>14); \
		} \
	} \
	else {  /* crece */ \
		/* a->Delta impar, incrementa en uno... no entiendo muy bien */ \
		if (a->Delta&1) \
			a->ExtMax++; \
		if (data&bmag) { /* mag=1 */ \
			a->ExtMax += (a->Delta + (a->Delta>>1)); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY2_K1+8192)>>14); \
		} \
		else { /* mag=0 */ \
			a->ExtMax += (a->Delta>>1); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY2_K0+8192)>>14); \
		} \
	} \
}

#define ZY2_TOADPCM(Edata, data, bsign, bmag) { \
	Edata -= a->ExtMax; \
	if (Edata<0) { \
		data |= bsign; \
		if (-Edata>a->Delta) \
			data |= bmag; \
	} \
	else if (Edata>a->Delta) \
		data |= bmag; \
	ZY2_ADAPT(data,bsign,bmag); \
}

/* <DOC> */
/**********************************************************/
/* Inicializa la estructura de tipo ADPCM_ZY2 apuntada
por {a} a estado de reposo inicial. Se debe llamar a esta
funcion antes de comenzar el proceso de codificacion o
decodificacion ADPCM ZyXEL-2.
Una vez inicializada la estructura, utilizaremos adpcm_zy2_code()
o adpcm_zy2_decode() para ir codificando/decodificando un bitstream.
No mezclar la misma estructura {a} para codificar y/o decodificar
bitstreams diferentes!  Utilizar una estructura diferente para cada
bitstream. */

PUBLIC SPL_VOID XAPI adpcm_zy2_reset( ADPCM_ZY2 * a )
/* </DOC> */
{
	a->Delta = 5;
	a->ExtMax = 0;
}

/* <DOC> */
/**********************************************************/
/* Codificador ADPCM ZyXEL-2. Codifica un bitstream
PCM 16bit_lineal/muestra a ADPCM 2bits/muestra.
Optimizado para frec_muestreo = 9600 m/s.
- En {a} se envia un puntero a la estructura ADPCM_ZY2 necesaria
para almanenar informacion de estado entre una llamada y otra a
esta funcion. Antes de utilizar esta funcion por primera vez
se debe inicializar la estructura con adpcm_zy2_reset(). Luego
ya podemos ir llamando a esta funcion tantas veces como se quiera.
Cada vez que se quiera codificar un bitstream nuevo se debe volver
a llamar a adpcm_zy2_reset() para reinicializar el proceso.
- En {v16} recibe un puntero a un bloque de muestras PCM16.
- En {v8} recibe un puntero a un buffer para salvar muestras ADPCM2.
- El numero de muestras a codificar se envia en {nd}.

En el vector {v16} el usuario debe enviar {nd} muestras de tipo
INT16. En el vector {v8} el usuario debe enviar un buffer de
{nd}/4 bytes (UINT8) si {nd} es multiplo de 4 o de {nd}/4+1 bytes si
{nd} es no lo es (la funcion adpcm_zy2_d2c(nd) {devuelve} directamente
el valor adecuado).

La funcion codifica las {nd} muestras recibidas en {v16} y mete
el resultado en {v8}. Si {nd} no es multiplo de 4, los ultimos bits
del ultimo byte de {v8} quedan sin utilizar y se inicializan a 0.

No mezclar llamadas a adpcm_zy2_code() con adpcm_zy2_decode() sobre
la misma estructura {a}!!.

Formato del bitstream ADPCM2 de ZyXEL:
- ADPCM2 array de valores UINT8 (1 byte). Cada uno de estos bytes
almacena 4 'muestras' ADPCM de 2 bits. Los 2 bits mas significativos
forman la primera muestra, y los 2 menos significativos forman la
cuarta muestra. */

PUBLIC SPL_VOID XAPI adpcm_zy2_code( ADPCM_ZY2 * a, pINT16 v16, pUINT8 v8, SPL_INT nd )
/* </DOC> */
{
	INT16 Edata;
	UINT8 Rdata;

	while (nd>=4) {
		Rdata = 0;

		Edata = (*v16++);
		ZY2_TOADPCM(Edata,Rdata,0x80,0x40);

		Edata = (*v16++);
		ZY2_TOADPCM(Edata,Rdata,0x20,0x10);

		Edata = (*v16++);
		ZY2_TOADPCM(Edata,Rdata,0x08,0x04);

		Edata = (*v16++);
		ZY2_TOADPCM(Edata,Rdata,0x02,0x01);

		(*v8++) = Rdata;

		nd -= 4;
	}

	if (nd) {
		Rdata = 0;

		Edata = (*v16++);
		ZY2_TOADPCM(Edata,Rdata,0x80,0x40);

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Edata = (*v16++);
		ZY2_TOADPCM(Edata,Rdata,0x20,0x10);

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Edata = (*v16);
		ZY2_TOADPCM(Edata,Rdata,0x08,0x04);

		*v8 = Rdata;
	}
}

/* <DOC> */
/**********************************************************/
/* Decodificador ADPCM ZyXEL-2. Decodifica un bitstream
ADPCM 2bits/muestra a PCM 16bit_lineal/muestra.
- En {a} se envia un puntero a la estructura ADPCM_ZY2 necesaria
para almanenar informacion de estado entre una llamada y otra a
esta funcion. Antes de utilizar esta funcion por primera vez
se debe inicializar la estructura con adpcm_zy2_reset(). Luego
ya podemos ir llamando a esta funcion tantas veces como se quiera.
Cada vez que se quiera decodificar un bitstream nuevo se debe volver
a llamar a adpcm_zy2_reset() para reinicializar el proceso.
- En {v8} recibe un puntero a un bloque de muestras ADPCM2.
- En {v16} recibe un puntero a un buffer para salvar muestras PCM16.
- El numero de muestras a decodificar se envia en {nd}.

En el vector {v8} el usuario debe enviar {nd} muestras de tipo
ADPCM2, es decir, {nd}/4 bytes (UINT8) si {nd} es multiplo
de 4 o {nd}/4+1 bytes si {nd} no lo es (la funcion adpcm_zy2_d2c(nd)
{devuelve} directamente el valor adecuado). En el vector {v16} el usuario
debe enviar un buffer de {nd} elementos de tipo UINT16.

La funcion decodifica las {nd} muestras recibidas en {v8} y mete
el resultado en {v16}. Si {nd} no es multiplo de 4, los ultimos bits
del ultimo byte de {v8} no se decodifican.

No mezclar llamadas a adpcm_zy2_code() con adpcm_zy2_decode() sobre
la misma estructura {a}!!.

Ver adpcm_zy2_code() para el formato del bitstream ADPCM ZyXEL-2 */

PUBLIC SPL_VOID XAPI adpcm_zy2_decode( ADPCM_ZY2 * a, pUINT8 v8, pINT16 v16, SPL_INT nd )
/* </DOC> */
{
	UINT8 in;

	while (nd>=4) {
		in = (*(v8++)); /* coge byte, y avanza puntero */

		ZY2_ADAPT(in,0x80,0x40); /* xx-- ---- */
		*(v16++) = a->ExtMax;

		ZY2_ADAPT(in,0x20,0x10); /* --xx ---- */
		*(v16++) = a->ExtMax;

		ZY2_ADAPT(in,0x08,0x04); /* ---- xx-- */
		*(v16++) = a->ExtMax;

		ZY2_ADAPT(in,0x02,0x01); /* ---- --xx */
		*(v16++) = a->ExtMax;

		nd -= 4;
	}

	if (nd) {
		in = (*v8); /* coge byte */

		ZY2_ADAPT(in,0x80,0x40); /* xx-- ---- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		ZY2_ADAPT(in,0x20,0x10); /* --xx ---- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		ZY2_ADAPT(in,0x08,0x04); /* ---- xx-- */
		*v16 = a->ExtMax;
	}
}

/**********************************************************/
/* macros internos utilizados por adpcm_zy3_???() */

#define ZY3_K0 0x399a
#define ZY3_K1 0x3a9f
#define ZY3_K2 0x4d14
#define ZY3_K3 0x6607

#define ZY3_ADAPT(data, bsign, bmag1, bmag0) { \
	if (data&bsign) { /* decrece */ \
		if (data&bmag1) { \
			if (data&bmag0) { /* mag=3 */ \
				a->ExtMax -= ((a->Delta<<1) + a->Delta + (a->Delta>>1)); \
				a->Delta = (INT16)(((INT32)a->Delta*ZY3_K3+8192)>>14); \
			} \
			else { /* mag=2 */  \
				a->ExtMax -= ((a->Delta<<1) + (a->Delta>>1)); \
				a->Delta = (INT16)(((INT32)a->Delta*ZY3_K2+8192)>>14); \
			} \
		} \
		else if (data&bmag0) { /* mag=1 */ \
			a->ExtMax -= (a->Delta + (a->Delta>>1)); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY3_K1+8192)>>14); \
		} \
		else { /* mag=0*/  \
			a->ExtMax -= (a->Delta>>1); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY3_K0+8192)>>14); \
		} \
	} \
	else { /* crece */  \
		/* a->Delta impar, incrementa en uno... no entiendo muy bien */ \
		if (a->Delta&1) \
			a->ExtMax++; \
		if (data&bmag1)  { \
			if (data&bmag0) { /* mag=3 */ \
				a->ExtMax += ((a->Delta<<1) + a->Delta + (a->Delta>>1)); \
				a->Delta = (INT16)(((INT32)a->Delta*ZY3_K3+8192)>>14); \
			} \
			else {/* mag=2 */ \
				a->ExtMax += ((a->Delta<<1) + (a->Delta>>1)); \
				a->Delta = (INT16)(((INT32)a->Delta*ZY3_K2+8192)>>14); \
			} \
		} \
		else if (data&bmag0) { /* mag=1 */ \
			a->ExtMax += (a->Delta + (a->Delta>>1)); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY3_K1+8192)>>14); \
		} \
		else { /* mag=0 */ \
			a->ExtMax += (a->Delta>>1); \
			a->Delta = (INT16)(((INT32)a->Delta*ZY3_K0+8192)>>14); \
		} \
	} \
}

#define ZY3_TOADPCM(Edata, data, bsign, bmag1, bmag0) { \
	Edata -= a->ExtMax; \
	if (Edata<0) { \
		data |= bsign; \
		Edata = -Edata; \
	} \
	if (Edata>(a->Delta<<1)) { \
		data |=bmag1; \
		if (Edata>(a->Delta+(a->Delta<<1))) \
			data |= bmag0; \
	} \
	else if (Edata>a->Delta) \
		data |= bmag0; \
	ZY3_ADAPT(data, bsign, bmag1, bmag0); \
}

/* <DOC> */
/**********************************************************/
/* Inicializa la estructura de tipo ADPCM_ZY3 apuntada
por {a} a estado de reposo inicial. Se debe llamar a esta
funcion antes de comenzar el proceso de codificacion o
decodificacion ADPCM ZyXEL-3.
Una vez inicializada la estructura, utilizaremos adpcm_zy3_code()
o adpcm_zy3_decode() para ir codificando/decodificando un bitstream.
No mezclar la misma estructura {a} para codificar y/o decodificar
bitstreams diferentes!  Utilizar una estructura diferente para cada
bitstream. */

PUBLIC SPL_VOID XAPI adpcm_zy3_reset( ADPCM_ZY3 * a )
/* </DOC> */
{
	a->Delta = 5;
	a->ExtMax = 0;
}

/* <DOC> */
/**********************************************************/
/* Codificador ADPCM ZyXEL-3. Codifica un bitstream
PCM 16bit_lineal/muestra a ADPCM 3bits/muestra.
Optimizado para frec_muestreo = 9600 m/s.
- En {a} se envia un puntero a la estructura ADPCM_ZY3 necesaria
para almanenar informacion de estado entre una llamada y otra a
esta funcion. Antes de utilizar esta funcion por primera vez
se debe inicializar la estructura con adpcm_zy3_reset(). Luego
ya podemos ir llamando a esta funcion tantas veces como se quiera.
Cada vez que se quiera codificar un bitstream nuevo se debe volver
a llamar a adpcm_zy3_reset() para reinicializar el proceso.
- En {v16} recibe un puntero a un bloque de muestras PCM16.
- En {v8} recibe un puntero a un buffer para salvar muestras ADPCM2.
- El numero de muestras a codificar se envia en {nd}.

En el vector {v16} el usuario debe enviar {nd} muestras de tipo
INT16. En el vector {v8} el usuario debe enviar un buffer de
longitud adecuada (la funcion adpcm_zy3_d2c(nd) {devuelve}
directamente el valor en bytes necesario).

La funcion codifica las {nd} muestras recibidas en {v16} y mete
el resultado en {v8}. Si {nd} no es multiplo de 4, los ultimos bits
del ultimo byte de {v8} quedan sin utilizar y se inicializan a 0.

No mezclar llamadas a adpcm_zy3_code() con adpcm_zy3_decode() sobre
la misma estructura {a}!!.

Formato del bitstream ADPCM3 de ZyXEL:
- ADPCM3 array de valores UINT8 (1 byte). Cada grupo de 3 bytes
almacena 8 'muestras' ADPCM de 3 bits:
.            11122233 34445556 66777888
.             byte0    byte1    byte2
*/

PUBLIC SPL_VOID XAPI adpcm_zy3_code( ADPCM_ZY3 * a, pINT16 v16, pUINT8 v8, SPL_INT nd )
/* </DOC> */
{
	INT16 Edata;
	UINT8 Rdata, Tmp;

	while (nd>=8) {
		Rdata = 0;

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x80,0x40,0x20); /* xxx- ---- */

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x10,0x08,0x04); /* ---x xx-- */

		Tmp = Rdata;
		Rdata = 0;
		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x02,0x01,0x80); /* ---- --xx x--- ---- */
		(*v8++) = Tmp | (Rdata &0x03);
		Rdata &= 0x80;

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x40,0x20,0x10); /* -xxx ---- */

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x08,0x04,0x02); /* ---- xxx- */

		Tmp = Rdata;
		Rdata = 0;
		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x01,0x80,0x40); /* ---- ---x xx-- ---- */
		(*v8++) = Tmp | (Rdata &0x01);
		Rdata &= 0xc0;

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x20,0x10,0x08); /* --xx x--- */

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x04,0x02,0x01); /* ---- -xxx */
		(*v8++) = Rdata;

		nd -= 8;
	}

	if (nd) {
		Rdata = 0;

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x80,0x40,0x20); /* xxx- ---- */

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x10,0x08,0x04); /* ---x xx-- */

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Tmp = Rdata;
		Rdata = 0;
		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x02,0x01,0x80); /* ---- --xx x--- ---- */
		(*v8++) = Tmp | (Rdata &0x03);
		Rdata &= 0x80;

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x40,0x20,0x10); /* -xxx ---- */

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x08,0x04,0x02); /* ---- xxx- */

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Tmp = Rdata;
		Rdata = 0;
		Edata = (*v16++);
		ZY3_TOADPCM(Edata,Rdata,0x01,0x80,0x40); /* ---- ---x xx-- ---- */
		(*v8++) = Tmp | (Rdata &0x01);
		Rdata &= 0xc0;

		if (!(--nd)) {
			*v8 = Rdata;
			return;
		}

		Edata = (*v16);
		ZY3_TOADPCM(Edata,Rdata,0x20,0x10,0x08); /* --xx x--- */

		*v8 = Rdata;
	}

}

/* <DOC> */
/**********************************************************/
/* Decodificador ADPCM ZyXEL-3. Decodifica un bitstream
ADPCM 3bits/muestra a PCM 16bit_lineal/muestra.
- En {a} se envia un puntero a la estructura ADPCM_ZY3 necesaria
para almanenar informacion de estado entre una llamada y otra a
esta funcion. Antes de utilizar esta funcion por primera vez
se debe inicializar la estructura con adpcm_zy3_reset(). Luego
ya podemos ir llamando a esta funcion tantas veces como se quiera.
Cada vez que se quiera decodificar un bitstream nuevo se debe volver
a llamar a adpcm_zy3_reset() para reinicializar el proceso.
- En {v8} recibe un puntero a un bloque de muestras ADPCM3.
- En {v16} recibe un puntero a un buffer para salvar muestras PCM16.
- El numero de muestras a decodificar se envia en {nd}.

En el vector {v8} el usuario debe enviar {nd} muestras de tipo
ADPCM3, la funcion adpcm_zy2_d2c(nd) {devuelve} directamente el valor
en bytes adecuado. En el vector {v16} el usuario debe enviar un buffer
de {nd} elementos de tipo UINT16.

La funcion decodifica las {nd} muestras recibidas en {v8} y mete
el resultado en {v16}. Si {nd} no es multiplo de 8, los ultimos bits
del ultimo byte de {v8} no se decodifican.

No mezclar llamadas a adpcm_zy3_code() con adpcm_zy3_decode() sobre
la misma estructura {a}!!.

Ver adpcm_zy3_code() para el formato del bitstream ADPCM ZyXEL-3 */

PUBLIC SPL_VOID XAPI adpcm_zy3_decode( ADPCM_ZY3 * a, pUINT8 v8, pINT16 v16, SPL_INT nd )
/* </DOC> */
{
	UINT8 in, tmp;

	while (nd>=8) {
		in = (*v8++);

		ZY3_ADAPT(in,0x80,0x40,0x20);  /* xxx- ---- */
		*(v16++) = a->ExtMax;
		ZY3_ADAPT(in,0x10,0x08,0x04);  /* ---x xx-- */
		*(v16++) = a->ExtMax;

		tmp = (in&0x03);
		in = (*v8++);

		tmp |= (in&0x80);
		ZY3_ADAPT(tmp,0x02,0x01,0x80);  /* ---- --xx  x--- ---- */
		*(v16++) = a->ExtMax;

		ZY3_ADAPT(in,0x40,0x20,0x10);  /* -xxx ---- */
		*(v16++) = a->ExtMax;
		ZY3_ADAPT(in,0x08,0x04,0x02);  /* ---- xxx- */
		*(v16++) = a->ExtMax;

		tmp = (in&0x01);
		in = (*v8++);

		tmp |= (in&0xc0);
		ZY3_ADAPT(tmp,0x01,0x80,0x40);  /* ---- ---x  xx-- ---- */
		*(v16++) = a->ExtMax;

		ZY3_ADAPT(in,0x20,0x10,0x08);  /* --xx x--- */
		*(v16++) = a->ExtMax;
		ZY3_ADAPT(in,0x04,0x02,0x01);  /* ---- -xxx */
		*(v16++) = a->ExtMax;

		nd -= 8;
	}

	if (nd) {
		in = (*v8++);

		ZY3_ADAPT(in,0x80,0x40,0x20);  /* xxx- ---- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		ZY3_ADAPT(in,0x10,0x08,0x04);  /* ---x xx-- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		tmp = (in&0x03);
		in = (*v8++);

		tmp |= (in&0x80);
		ZY3_ADAPT(tmp,0x02,0x01,0x80);  /* ---- --xx  x--- ---- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		ZY3_ADAPT(in,0x40,0x20,0x10);  /* -xxx ---- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		ZY3_ADAPT(in,0x08,0x04,0x02);  /* ---- xxx- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		tmp = (in&0x01);
		in = (*v8);

		tmp |= (in&0xc0);
		ZY3_ADAPT(tmp,0x01,0x80,0x40);  /* ---- ---x  xx-- ---- */
		*(v16++) = a->ExtMax;

		if (!(--nd))
			return;

		ZY3_ADAPT(in,0x20,0x10,0x08);  /* --xx x--- */
		*v16 = a->ExtMax;
	}
}

/**********************************************************/
