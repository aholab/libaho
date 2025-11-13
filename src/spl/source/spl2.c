/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL2.C
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
Evaluacion de funciones de enventanado (hamming, hanning,
etc).
Cada grupo de funciones (win_???, win2_???, wini_???) va
precedido por una descripcion general del contenido.

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
/* Funciones para generacion de ventanas (win_???).

Parametros:

- SPL_pFLOAT {v} : es un vector de {N} elementos SPL_FLOAT donde se va a
.                   meter la ventana.
- SPL_INT {N} : es la longitud de la ventana, y longitud del vector {v}.
.               {N}>=1

Las funciones rellenan el vector {v} con la ventana adecuada.
Ademas cada funcion {devuelve} el propio puntero SPL_pFLOAT {v}.
La longitud del vector {v} es la misma que la de la ventana, es decir
{N} elementos. Tambien se puede obtener este valor con la tonta-funcion
nel_win(N), que tonta-simplemente devuelve {N}.

Ref:
.    Digital Processing of Speech Signals
.    L.R. Rabiner / R.W Schafer
.    Prentice Hall

.    Mathcad 3.1 Electrical Engineering Handbook (WINDOWS.MCD)
*/
/**********************************************************/
/* Ventana rectangular */

PUBLIC SPL_pFLOAT XAPI win_rect( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i;
	assert(N>0);

	for (i=0; i<N; i++)
		v[i]=1.0;

	return v;
}

/**********************************************************/
/* ventana de Bartlett (triangular) */

PUBLIC SPL_pFLOAT XAPI win_bart( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/2.0;

	v[half]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
	v[N-i]=v[i]=(SPL_FLOAT)i/k;

	return v;
}

/**********************************************************/
/* ventana de Hanning */

PUBLIC SPL_pFLOAT XAPI win_hann( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/(2.0*M_PI);

	v[half]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
	v[N-i]=v[i]=0.5-0.5*cos((SPL_FLOAT)i/k);

	return v;
}

/**********************************************************/
/* ventana de Hamming */

PUBLIC SPL_pFLOAT XAPI win_hamm( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/(2.0*M_PI);

	v[half]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
	v[N-i]=v[i]=0.54-0.46*cos((SPL_FLOAT)i/k);

	return v;
}

/**********************************************************/
/* ventana de Blackman */

PUBLIC SPL_pFLOAT XAPI win_black( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/(2.0*M_PI);

	v[half]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
	v[N-i]=v[i]=0.42 - 0.5*cos((SPL_FLOAT)i/k) +
			0.08*cos((2.0*(SPL_FLOAT)i)/k);

	return v;
}

/**********************************************************/
/* ventana de Kaiser con parametro beta (b).
Esta funcion tiene un parametro extra:
- SPL_FLOAT {b} : {b}=0-->ventana rectangular, y segun crece {b},
.                  la ventana de Kaiser se va estrechando.
.                  {b}>=0
*/

PUBLIC SPL_pFLOAT XAPI win_kais( SPL_pFLOAT v, SPL_INT N, SPL_FLOAT b )
{
	SPL_INT i, half;
	SPL_FLOAT k, I0b;
	assert(N>0);
	assert(b>=0.0);

	half=N/2;
	N--;
	k=(SPL_FLOAT)N/(b*2.0);
	I0b=bessel_I0(b);

	v[half]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
	v[N-i]=v[i]=v[i]=bessel_I0(sqrt((SPL_FLOAT)i*
			(SPL_FLOAT)(N-i))/k)/I0b;

	return v;
}

/**********************************************************/
/* {devuelve} el numero de ELEMENTOS (no bytes, sino SPL_FLOAT) que
debe tener el vector {v} en las funciones de ventanas win_???(v,N).
{N}>=0 */

PUBLIC SPL_INT XAPI nel_win( SPL_INT N )
{
	assert(N>=0);

	return N;
}

/**********************************************************/
/* Funciones para generacion de 'medias' ventanas (win2_???).

Puesto que las ventanas son simetricas, basta generar media ventana.

Parametros:

- SPL_pFLOAT {v} : es un vector de XAPI elementos donde se va a meter la
.                   ventana. El valor XAPI lo devuelve la funcion nel_win2(N).
- SPL_INT {N} : es la longitud de la ventana (NO la del vector {v}).
.               {N}>=1

Las funciones rellenan el vector {v} con la mitad de la ventana adecuada.
Cada funcion {devuelve} el propio puntero SPL_pFLOAT {v}.

La longitud XAPI del vector {v} varia dependiendo de si la longitud de
la ventana {N} es par o impar.
- si {N} es par, el vector {v} tendra XAPI={N}/2 elementos y contiene media
.    ventana, siendo la segunda mitad de la ventana la simetrica de {v}.
- si {N} es impar, el vector {v} tendra XAPI=({N}-1)/2+1 elementos y contendra
.    una muestra central (la ultima de {v}), habiendo simetria respecto a
.    esta muestra (ella no tiene simetrica).
La longitud XAPI para una ventana de longitud {N} se puede obtener llamando
a la funcion nel_win2(N).
Dada una posicion i (de 0 a {N}-1) dentro de una ventana de longitud {N},
el indice (comenzando en 0) dentro del vector {v} que corresponde a i
se puede obtener con la funcion idx_win2(i,N).
Esta funcion simplemente refleja los valores de i que estan en la
segunda mitad de la ventana sobre la primera mitad, que es la contenida
en el vector {v}. */
/**********************************************************/
/* media-ventana rectangular */

PUBLIC SPL_pFLOAT XAPI win2_rect( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i;
	assert(N>0);

	N=(N+1)/2;
	for (i=0; i<N; i++)
		v[i]=1.0;

	return v;
}

/**********************************************************/
/* media-ventana de Bartlett (triangular) */

PUBLIC SPL_pFLOAT XAPI win2_bart( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/2.0;

	v[N/2]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
		v[i]=(SPL_FLOAT)i/k;

	return v;
}

/**********************************************************/
/* media-ventana de Hanning */

PUBLIC SPL_pFLOAT XAPI win2_hann( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/(2.0*M_PI);

	v[N/2]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
		v[i]=0.5-0.5*cos((SPL_FLOAT)i/k);

	return v;
}

/**********************************************************/
/* media-ventana de Hamming */

PUBLIC SPL_pFLOAT XAPI win2_hamm( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/(2.0*M_PI);

	v[N/2]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
		v[i]=0.54-0.46*cos((SPL_FLOAT)i/k);

	return v;
}

/**********************************************************/
/* media-ventana de Blackman */

PUBLIC SPL_pFLOAT XAPI win2_black( SPL_pFLOAT v, SPL_INT N )
{
	SPL_INT i, half;
	SPL_FLOAT k;
	assert(N>0);

	half=N/2;
	N--;
	k=((SPL_FLOAT)N)/(2.0*M_PI);

	v[N/2]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
		v[i]=0.42 - 0.5*cos((SPL_FLOAT)i/k) +
				0.08*cos((2.0*(SPL_FLOAT)i)/k);

	return v;
}

/**********************************************************/
/* media-ventana de Kaiser con parametro beta (b).
Esta funcion tiene un parametro extra:
- SPL_FLOAT {b} : {b}=0-->ventana rectangular, y segun crece {b}, la ventana
.                  de Kaiser se va estrechando. {b}>=0
*/

PUBLIC SPL_pFLOAT XAPI win2_kais( SPL_pFLOAT v, SPL_INT N, SPL_FLOAT b )
{
	SPL_INT i, half;
	SPL_FLOAT k, I0b;
	assert(N>0);
	assert(b>=0.0);

	half=N/2;
	N--;
	k=(SPL_FLOAT)N/(b*2.0);
	I0b=bessel_I0(b);

	v[N/2]=1.0; /* N puede ser impar */
	for (i=0; i<half; i++)
		v[i]=bessel_I0(sqrt((SPL_FLOAT)i*(SPL_FLOAT)(N-i))/k)/I0b;

	return v;
}

/**********************************************************/
/* {devuelve} el numero de ELEMENTOS que debe tener el vector {v}
en las funciones de medias-ventanas win2_???(v,N).
{N}>=0 */

PUBLIC SPL_INT XAPI nel_win2( SPL_INT N )
{
	assert(N>=0);

	return (N+1)/2;
}

/**********************************************************/
/* Dado un vector {v} de media-ventana obtenido con las funciones
win2_???(v,N), esta funcion {devuelve} que elemento de ese vector
representa al elemento {i} de la ventana completa.

Parametros:
- SPL_INT {N} : Longitud de la ventana (completa). {N}>=0
- SPL_INT {i} : Posicion en la ventana. 0 <= {i} <= {N}-1

{i}=0 --> {devuelve} 0
{i}={N}-1 --> {devuelve} 0
{i}=1 --> {devuelve} 1
{i}={N}-2 --> {devuelve} 1
{i}=2 --> {devuelve} 2
{i}={N}-3 --> {devuelve} 2
etc.
*/

PUBLIC SPL_INT XAPI idx_win2( SPL_INT i, SPL_INT N )
{
	assert(N>=0);
	assert(i>=0);
	assert(i<N);

	return (i<((N+1)/2)) ? i : N-i-1;
}

/**********************************************************/
/* Funciones para obtener valores de ventanas en puntos
concretos (wini_???)

Parametros:

- SPL_INT {i} : punto del que se quire saber el valor de la ventana
.               Si {i}<0 o {i}>=N, las funciones estan indefinidas
- SPL_INT {N} : es la longitud de la ventana.
.               {N}>=0

Cada funcion {devuelve} el valor de la ventana de longitud {N} en
el punto {i} */
/**********************************************************/
/* Ventana rectangular */

PUBLIC SPL_FLOAT XAPI wini_rect( SPL_INT i, SPL_INT N )
{
	assert(i>=0);
	assert(i<N);

#ifdef NDEBUG
	(void)i;
	(void)N;
#endif

	return 1.0;
}

/**********************************************************/
/* ventana de Bartlett (triangular) */

PUBLIC SPL_FLOAT XAPI wini_bart( SPL_INT i, SPL_INT N )
{
	SPL_FLOAT k;
	assert(i>=0);
	assert(i<N);

	k=(2.0*(SPL_FLOAT)i)/(SPL_FLOAT)(N-1);
	if (k<=1.0)
		return k;
	else
		return 2.0-k;
}

/**********************************************************/
/* ventana de Hanning */

PUBLIC SPL_FLOAT XAPI wini_hann( SPL_INT i, SPL_INT N )
{
	assert(i>=0);
	assert(i<N);

	return 0.5-0.5*cos(((2.0*M_PI)*(SPL_FLOAT)i)/(SPL_FLOAT)(N-1));
}

/**********************************************************/
/* ventana de Hamming */

PUBLIC SPL_FLOAT XAPI wini_hamm( SPL_INT i, SPL_INT N )
{
	assert(i>=0);
	assert(i<N);

	return 0.54-0.46*cos(((2.0*M_PI)*(SPL_FLOAT)i)/(SPL_FLOAT)(N-1));
}

/**********************************************************/
/* ventana de Blackman */

PUBLIC SPL_FLOAT XAPI wini_black( SPL_INT i, SPL_INT N )
{
	assert(i>=0);
	assert(i<N);

	return 0.42 - 0.5*cos(((2.0*M_PI)*(SPL_FLOAT)i)/(SPL_FLOAT)(N-1))
			+ 0.08*cos(((4.0*M_PI)*
			(SPL_FLOAT)i)/(SPL_FLOAT)(N-1));
}

/**********************************************************/
/* ventana de Kaiser con parametro b (beta).
Esta funcion tiene un parametro extra:
- SPL_FLOAT {b} : {b}=0-->ventana rectangular, y segun crece {b},
.                  la ventana de Kaiser se va estrechando.
.                  {b}>=0
*/

PUBLIC SPL_FLOAT XAPI wini_kais( SPL_INT i, SPL_INT N, SPL_FLOAT b )
{
	assert(i>=0);
	assert(i<N);
	assert(b>=0.0);

	return bessel_I0((b*2.0*sqrt((SPL_FLOAT)i*(SPL_FLOAT)(N-1-i)))/
			(SPL_FLOAT)(N-1))/bessel_I0(b);
}

/**********************************************************/

