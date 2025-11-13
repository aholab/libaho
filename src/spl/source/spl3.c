/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL3.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Cruces por cero, filtrado de primer orden (preenfasis y
deenfasis).
Mas abajo se comenta el formato de las funciones de deenfasis
y preenfasis.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/**********************************************************/
/* Calcula los cruces por cero del vector {v} de {N} elementos.

Parametros:

- SPL_pFLOAT {v} : vector de longitud {N} del que se quieren calcular los cc.
- SPL_INT {N} : longitud del vector {v}. {N}>=0

La funcion {devuelve} el numero de cruces por cero encontrados.
Los puntos 'tangentes' en cero no se tienen en cuenta. */

PUBLIC SPL_INT XAPI zero_cross( SPL_pFLOAT v, SPL_INT N )
{
#define __getsign(n,os) (((n)>0)? 1 : (((n)<0)? -1 : (os)))
	SPL_INT i, cc;
	SPL_INT os;  /* signo viejo */
	SPL_INT ns;  /* signo nuevo */
	assert(N>=0);

	cc = 0;
	os = __getsign(*v,1);
	for (i=1; i<N; i++) {
		v++;
		ns = __getsign(*v,os);
		if (ns!=os)
			cc++;
		os=ns;
	}

	return cc;
#undef __getsign
}

/**********************************************************/
/* Funciones de filtrado de preenfasis y deenfasis.

Las funciones ????_i procesan un valor. Reciben
la entrada y la memoria del filtro, y devuelve la salida del
filtro.

Parametros:

- SPL_FLOAT {a} : constante de preenfasis/deenfasis. Comprendida
.                  entre 0 y 1, aunque funciona con cualquier valor.
.                  0 no hace nada, y 1 tiene efecto maximo.
- SPL_FLOAT {i} : entrada (input) al filtro.
- SPL_FLOAT {i0} : entrada anterior al filtro (memoria de la entrada
.                   anterior)
- SPL_FLOAT {o0} : salida anterior del filtro (memoria de la salida
.                   anterior del filtro).

Cada funcion {devuelve} la salida del sistema ('o', output).

Las funciones ?????_n procesan un vector. Reciben el vector y su
longitud, la memoria inicial del filtro y el vector de salida,
y se {devuelve} un puntero al propio vector de salida.

Parametros:

- SPL_FLOAT {a} : constante de preenfasis/deenfasis. Comprendida
.                  entre 0 y 1, aunque funciona con cualquier valor.
.                  0 no hace nada, y 1 tiene efecto maximo.
- SPL_FLOAT {i0} : entrada anterior al filtro (memoria de la entrada
.                   anterior al primer elemento del vector)
- SPL_FLOAT {o0} : salida anterior del filtro (memoria de la salida
.                   anterior al primer elemento del vector).
- SPL_pFLOAT {vi} : vector de entrada al filtro.
- SPL_pFLOAT {vo} : vector de salida del filtro
- SPL_INT {N} : longitud de los vectores de entrada  y salida. {N}>0

Cada funcion {devuelve} el propio puntero {vo}.
Puede enviarse {vo} igual a {vi} si se quiere reemplazar la entrada
con la salida. */
/**********************************************************/
/* {devuelve} la salida de preenfatizar con {a} la muestra de
entrada {i}, si la entrada anterior al sistema era {i0} */

PUBLIC SPL_FLOAT XAPI preen_i( SPL_FLOAT a, SPL_FLOAT i, SPL_FLOAT i0 )
{
	return i-a*i0;
}

/**********************************************************/
/* preenfatiza con {a} el vector {vi} de {N} elementos. Mete el resultado
en {vo}. {i0} es la entrada anterior al primer elemento de {vi} (memoria
inicial). {vi} e {vo} pueden ser el mismo. {N}>0 */

PUBLIC SPL_pFLOAT XAPI preen_n( SPL_FLOAT a, SPL_pFLOAT vi, SPL_pFLOAT vo,
		SPL_INT N, SPL_FLOAT i0 )
{
	SPL_INT i;
	assert(N>0);

	for (i=N-1; i>0; i--)  /* hacia atras, por si vi==vo */
		vo[i] = vi[i]-a*vi[i-1];
	vo[0] = vi[0]-a*i0;

	return vo;
}

/**********************************************************/
/* {devuelve} la salida de deenfatizar con {a} la muestra de
entrada {i}, si la salida anterior del sistema era {o0} */

PUBLIC SPL_FLOAT XAPI deen_i( SPL_FLOAT a, SPL_FLOAT i, SPL_FLOAT o0 )
{
	return i+a*o0;
}

/**********************************************************/
/* deenfatiza con {a} el vector {vi} de {N} elementos. Mete el resultado
en {vo}. {o0} es la salida anterior al primer elemento de {vo} (memoria
de salida inicial). {vi} e {vo} pueden ser el mismo. {N}>0 */

PUBLIC SPL_pFLOAT XAPI deen_n( SPL_FLOAT a, SPL_pFLOAT vi, SPL_pFLOAT vo,
		SPL_INT N, SPL_FLOAT o0 )
{
	SPL_INT i;
	assert(N>0);

	vo[0] = vi[0]+o0*a;
	for (i=1; i<N; i++)
		vo[i]=vi[i]+vo[i-1]*a;

	return vo;
}

/**********************************************************/

