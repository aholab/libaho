/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL7.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    27/05/96  Borja     correccion en documentacion
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Filtros de fase lineal, por el metodo de las ventanas (ventana
de Kaiser).
La respuesta impulsional que producen es simetrica respecto
al origen, son _siempre_ de longitud _impar_, y las funciones solo
calculan la mitad positiva mas el valor del origen. Por tanto,
cuando se tiene un filtro de longitud {L} (impar),
realmente se almacenan ({L}+1)/2 valores (funcion nel_fil(L)).

.         -(L-1)/2            0                (L-1)/2
.   --------|-----------------|-----------------|------------>n
.                             ^^^^^^^^^^^^^^^^^^^
.   L impar                   (L+1)/2 almacenados

Los filtros se definen de la siguiente forma:

.              A|
.               |
.             1 ************  } +/- deltaA
.               |           *
.               |            *                              f
.         ------------------|-*********************|-------->
.               0           fc                     fm/2
.                          \--/
.                         deltaF

- {deltaA}: Amplitud del rizado: De 1+{deltaA} a 1-{deltaA}.
.           0<{deltaA}<1.
- {fc}: Frecuencia de corte (A(fc)=0.5) de la banda de transicion
- {deltaF}: Anchura total de la banda de transicion.

Los valores frecuenciales ({deltaF} y {fc}) vienen normalizados
(es decir, en tanto por uno) a la frecuencia de muestreo {fm}.
Por tanto  0<{fc}<0.5

Fijados un rizado {deltaA}, y una banda de transicion {deltaF},
la funcion fil_get_L(deltaA,deltaF) {devuelve} la longitud {L} minima
que debe tener el filtro para cumplir las especificaciones. Las
funciones fil_get_deltaA(L,deltaF) y fil_get_deltaF(L,deltaA) son
otras conversiones posibles.

Las funciones fil_??? con ???=(lpf, hpf, bpf, rbf) generan la
respuesta impulsional del filtro. Reciben un vector {vh} de longitud
nel_fil(L), el rizado {deltaA}, la frecuencia de corte {fc} (paso bajo
y paso alto) o {fc1} y {fc2} (paso banda y banda eliminada), la longitud
del filtro {L} (impar) y un factor de ganancia {g} (amplificacion nula para
g=1.0).

.    fil_lpf() : Filtro paso bajo
.    fil_hpf() : Filtro paso alto
.    fil_bpf() : Filtro paso banda
.    fil_rbf() : Filtro banda eliminada

El vector {vh} se rellena con la respuesta impulsional h(n):
.   vh[0]=h(0)  vh[1]=h(1)=h(-1)  vh[2]=h(2)=h(-2) ..... vh[(L-1)/2]

Para filtrar una muestra s[n] con el filtro {vh} el proceso es:

.                         i=(L-1)/2
.     y[n] = s[n]*vh[0] + SUM ( (s[n-i]+s[n+i])*vh[i] )
.                         i=1

La funcion fil_fil() hace este proceso. Recibe:
.  - SPL_pFLOAT {vx} : vector de {L} muestras, vx[0] a vx[L-1], del que
.                       se va a filtrar la muestra vx[(L-1)/2].
.  - SPL_pFLOAT {vh} : Respuesta impulsional de {L} puntos, que realmente
.                       es un vector de ({L}+1)/2 valores.
.  - SPL_INT {L} : Longitud del filtro completo.
La funcion {devuelve} el valor de vx[(L-1)/2] filtrado.

Ref:
.    Digital Processing of Speech Signals
.    L.R. Rabiner / R.W Schafer
.    Prentice Hall

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
/* a partir de la amplitud de rizado {deltaA} y de la banda
de transicion {deltaF} {devuelve} la longitud L del filtro.
L es siempre impar */

PUBLIC SPL_INT XAPI fil_get_L( SPL_FLOAT deltaA, SPL_FLOAT deltaF )
{
	SPL_INT L;
	assert(deltaA>0.0);
	assert(deltaF>0.0);

	L=(SPL_INT)(ceil((-a2db(deltaA)-7.95)/(14.36*deltaF)))+1;
	if (L&1)      /* L debe ser impar */
		return L;
	else
		return L+1;
}

/**********************************************************/
/* a partir de la longitud del filtro {L} y de la banda de
transicion {deltaF} {devuelve} la amplitud del rizado deltaA */

PUBLIC SPL_FLOAT XAPI fil_get_deltaA( SPL_INT L, SPL_FLOAT deltaF )
{
	assert(L>0);
	assert(L&1);  /* L impar */
	assert(deltaF>0.0);

	return db2a(-((L-1)*(14.36*deltaF)+7.95));
}

/**********************************************************/
/* a partir de la longitud del filtro {L} y de la amplitud del
rizado {deltaA} {devuelve} la banda de transicion deltaF */

PUBLIC SPL_FLOAT XAPI fil_get_deltaF( SPL_INT L, SPL_FLOAT deltaA )
{
	assert(L>0);
	assert(L&1);  /* L impar */
	assert(deltaA>0.0);

	return (-a2db(deltaA)-7.95)/((L-1)*14.36);
}

/**********************************************************/
/* Funcion de uso interno.
a partir de la amplitud de rizado {deltaA} {devuelve} la constante
alpha necesaria para el prototipo del filtro */

PRIVATE SPL_FLOAT fil_get_alpha( SPL_FLOAT deltaA )
{
	assert(deltaA>0.0);

	deltaA = -a2db(deltaA);
	if (deltaA>50.0)
		return 0.1102*(deltaA-8.7);
	else if (deltaA>21.0)
		return 0.5842*pow(deltaA-21.0,0.4)+0.07886*(deltaA-21);
	else
		return 0;
}

/**********************************************************/
/* Rellena el vector {vh} de ({L}+1)/2 puntos con la mitad positiva (de
0 a ({L}-1)/2) de la respuesta impulsional de un filtro PASO BAJO,
con amplitud de rizado {deltaA}, frecuencia de corte {fc}, y longitud
del filtro de {L} puntos, y con una ganancia {g} (factor multiplicativo
de escalado).
{devuelve} el puntero {vh}.
{vh} tiene ({L}+1)/2 puntos.
0<{deltaA}<1, 0<{fc}<0.5, {L}>0, {L} impar. */

PUBLIC SPL_pFLOAT XAPI fil_lpf( SPL_pFLOAT vh, SPL_FLOAT deltaA,
		SPL_FLOAT fc, SPL_INT L, SPL_FLOAT g )
{
	SPL_INT i, n;
	SPL_FLOAT nd, alpha, I0alpha, tmp, wp;
	assert(deltaA>0.0);
	assert(fc>0.0);
	assert(fc<=0.5);
	assert(L>0);
	assert(L&1);  /* L debe ser impar */

	n = (L-1)/2;
	nd = n;
	alpha = fil_get_alpha(deltaA);
	I0alpha = bessel_I0(alpha);
	wp = (M_PI*2.0)*fc;

	vh[0]= g*2.0*fc;
	for (i=1; i<=n; i++) {  /* sinc enventanada con Kaiser */
		tmp = i/nd;
		vh[i] =g*sin(wp*i)/(M_PI*i)*
				bessel_I0(alpha*sqrt(1.0-tmp*tmp))/I0alpha;
	}
	return vh;
}

/**********************************************************/
/* Rellena el vector {vh} de ({L}+1)/2 puntos con la mitad positiva (de
0 a ({L}-1)/2) de la respuesta impulsional de un filtro PASO ALTO,
con amplitud de rizado {deltaA}, frecuencia de corte {fc}, y longitud
del filtro de {L} puntos, y con una ganancia {g} (factor multiplicativo
de escalado).
{devuelve} el puntero {vh}.
{vh} tiene ({L}+1)/2 puntos.
0<{deltaA}<1, 0<{fc}<0.5, {L}>0, {L} impar. */

PUBLIC SPL_pFLOAT XAPI fil_hpf( SPL_pFLOAT vh, SPL_FLOAT deltaA,
		SPL_FLOAT fc, SPL_INT L, SPL_FLOAT g )
{
	SPL_INT i, n;

	fil_lpf(vh,deltaA,0.5-fc,L,g);
	n=(L+1)/2;

	for (i=1; i<n; i+=2)
		vh[i] = (-vh[i]);   /* multiplicar por cos(fm/2*t) */

	return vh;
}

/**********************************************************/
/* Rellena el vector {vh} de ({L}+1)/2 puntos con la mitad positiva (de
0 a ({L}-1)/2) de la respuesta impulsional de un filtro PASO BANDA,
con amplitud de rizado {deltaA}, frecuencia de corte inferior {fc1} y
superior {fc2}, y longitud del filtro de {L} puntos, y con una
ganancia {g} (factor multiplicativo de escalado).
{devuelve} el puntero {vh}.
{vh} tiene ({L}+1)/2 puntos.
0<{deltaA}<1, 0<{fc1}<0.5, 0<{fc1}<0.5, {fc1}<{fc2}, {L}>0, {L} impar. */

PUBLIC SPL_pFLOAT XAPI fil_bpf( SPL_pFLOAT vh, SPL_FLOAT deltaA,
		SPL_FLOAT fc1, SPL_FLOAT fc2,
		SPL_INT L, SPL_FLOAT g )
{
	SPL_INT i, n;
	SPL_FLOAT w;
	assert(fc2>fc1);

	fil_lpf(vh,deltaA,(fc2-fc1)/2.0,L,g);
	n=(L+1)/2;
	w = (fc2+fc1)*M_PI;

	for (i=0; i<n; i++)   /* modula cos(w*t) */
		vh[i] *= 2*cos(w*i);

	return vh;
}

/**********************************************************/
/* Rellena el vector {vh} de ({L}+1)/2 puntos con la mitad positiva (de
0 a ({L}-1)/2) de la respuesta impulsional de un filtro BANDA ELIMINADA,
con amplitud de rizado {deltaA}, frecuencia de corte inferior {fc1} y
superior {fc2}, y longitud del filtro de {L} puntos, y con una
ganancia {g} (factor multiplicativo de escalado).
{devuelve} el puntero {vh}.
{vh} tiene ({L}+1)/2 puntos.
0<{deltaA}<1, 0<{fc1}<0.5, 0<{fc1}<0.5, {fc1}<{fc2}, {L}>0, {L} impar. */

PUBLIC SPL_pFLOAT XAPI fil_rbf( SPL_pFLOAT vh, SPL_FLOAT deltaA,
		SPL_FLOAT fc1, SPL_FLOAT fc2,
		SPL_INT L, SPL_FLOAT g )
{
	SPL_INT i, n;
	SPL_FLOAT w;
	assert(fc2>fc1);

	fil_lpf(vh,deltaA,(fc2-fc1)/2.0,L,g);
	n = (L+1)/2;
	w = (fc2+fc1)*M_PI;

	for (i=0; i<n; i++)  /* modula cos(w*t) */
		vh[i] *= -2*cos(w*i);
	vh[0] += g;

	return vh;
}

/**********************************************************/
/* {devuelve} la longitud en puntos que debe tener un vector
para almacenar una respuesta impulsional de longitud {L}, simetrica
respecto al origen */

PUBLIC SPL_INT XAPI nel_fil( SPL_INT L )
{
	assert(L>0);
	assert(L&1);  /* L debe ser impar */

	return (L+1)/2;
}

/**********************************************************/
/* {devuelve} el valor filtrado para la muestra vx[(L-1)/2],
es decir, que por delante de la muestra a filtrar debe haber
(nel_fil(L)-1) muestras.
-  {vx} es un vector de longitud L.
-  {vh} es la respuesta impulsional del filtro,
.       de longitud ({L}+1)/2 (nel_fil(L)).
-  {L} es la longitud total del filtro. {L}>0 impar. */

PUBLIC SPL_FLOAT XAPI fil_fil( SPL_pFLOAT vx, SPL_pFLOAT vh, SPL_INT L )
{
	SPL_INT i,n;
	SPL_FLOAT sum;
	assert(L>0);
	assert(L&1);  /* L debe ser impar */

	n=(L-1)/2;

	__sum(i,1,n,(vx[n-i]+vx[n+i])*vh[i],sum,vh[0]*vx[n]);

	return sum;
}

/**********************************************************/

