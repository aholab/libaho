/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL6.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    30/07/95  Borja     scope funciones explicito
1.1.1    02/03/95  Borja     LONG-->FLOAT para evitar posibles overflows
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Generacion de numeros aleatorios con distribucion pseudo-gaussiana
de valor medio cero.

Las funciones reciben un valor {c}. Cuanto mayor sea {c}, mas 'gaussiana'
es la distribucion. Con {c}=8 ya se obtienen buenos resultados.
Este valor {c} es el numero de valores aleatorios uniformes a sumar.
{c}>=1.

Hay 2 tipos de funciones noise_i_? y noise_n_?.

- noise_i_? se {devuelve} un valor aleatorio.
- noise_n_? rellenan un vector con valores aleatorios.

Hay 2 variantes para cada una de las anteriores:
- noise_?_v  distribucion con Varianza unidad (potencia normalizada).
- noise_?_m  distribucion con Maximo unidad (amplitud normalizada).

Las funciones gnoise_??? producen ruido pseudo-gaussiano generalizado
de forma similar a las funciones noise_??? pero con la media y la
varianza indicadas como parametros. Ademas de el valor {c}, reciben
la media en {mean} y la varianza en {var}. Hay 2 funciones, gnoise_i()
y gnoise_n() que respectivamente generan una sola muestra o rellenan
un vector.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>

#include "spli.h"

/**********************************************************/
/* {devuelve} numero aleatorio gaussiano entre +-1, sumando {c}
numeros aleatorios uniformes.
La varianza es 1/(3*{c}) = funcion noise_mvar(c). */

PUBLIC SPL_FLOAT XAPI noise_i_m( SPL_INT c )
{
	SPL_INT i;
	SPL_FLOAT l;
	assert(c>0);

	l = 0.0;
	for (i=0; i<c; i++)
		l += rand();  /* sumar para obtener numero aleatorio pseudo-gaussiano */

	/* normaliza a +-1 */
	return (l*2.0) / ((SPL_FLOAT)RAND_MAX*(SPL_FLOAT)c) - 1.0;
}

/**********************************************************/
/* {devuelve} numero aleatorio gaussiano con varianza unidad, sumando {c}
numeros aleatorios uniformes. */

PUBLIC SPL_FLOAT XAPI noise_i_v( SPL_INT c )
{
	SPL_INT i;
	SPL_FLOAT l;
	assert(c>0);

	l = 0.0;
	for (i=0; i<c; i++)
		l+=rand();

	return ((l*2.0) / ((SPL_FLOAT)RAND_MAX*(SPL_FLOAT)c) - 1.0) *
			sqrt(3.0*(SPL_FLOAT)c);
}

/**********************************************************/
/* rellena el vector {v} de {N} elementos con numeros aleatorios
gaussianos entre +-1, sumando {c} numeros aleatorios uniformes.
La varianza es 1/(3*{c}).
{devuelve} el puntero {v} */

PUBLIC SPL_pFLOAT XAPI noise_n_m( SPL_pFLOAT v, SPL_INT N, SPL_INT c )
{
	SPL_INT i;
	assert(c>0);

	for (i=0; i<N; i++)
		v[i]=noise_i_m(c);

	return v;
}

/**********************************************************/
/* rellena el vector {v} de {N} elementos con numeros aleatorios
gaussianos con varianza unidad sumando {c} numeros aleatorios uniformes.
{devuelve} el puntero {v} */

PUBLIC SPL_pFLOAT XAPI noise_n_v( SPL_pFLOAT v, SPL_INT N, SPL_INT c )
{
	SPL_INT i;
	assert(c>0);

	for (i=0; i<N; i++)
		v[i]=noise_i_v(c);

	return v;
}

/**********************************************************/
/* {devuelve} la varianza de los numeros generados con las funciones
noise_?_m(), para el parametro {c} que se utilice. */

PUBLIC SPL_FLOAT XAPI noise_mvar( SPL_INT c )
{
	return 1.0/(3.0*(SPL_FLOAT)c);
}

/**********************************************************/
/* {devuelve} numero aleatorio gaussiano con media {mean} y varianza
{var}, sumando {c} numeros aleatorios uniformes.
{c}>=1, {var}>0 */

PUBLIC SPL_FLOAT XAPI gnoise_i( SPL_INT c, SPL_FLOAT mean, SPL_FLOAT var )
{
	assert(c>0);
	assert(var>0.0);

	return mean+noise_i_v(c)*sqrt(var);
}

/**********************************************************/
/* rellena el vector {v} de {N} elementos con numeros aleatorios
gaussianos de media {mean} y varianza {var} sumando {c} numeros aleatorios
uniformes.
{devuelve} el puntero {v}.
{c}>=1, {var}>0 */

PUBLIC SPL_pFLOAT XAPI gnoise_n( SPL_pFLOAT v, SPL_INT N, SPL_INT c,
		SPL_FLOAT mean, SPL_FLOAT var )
{
	SPL_INT i;
	assert(c>0);
	assert(var>0.0);

	var = sqrt(var);
	for (i=0; i<N; i++)
		v[i]=mean+noise_i_v(c)*var;

	return v;
}

/**********************************************************/

