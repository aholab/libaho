/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LPCROOTS.CPP
Nombre paquete............... SPL
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... LP_NEGSUM

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.2.0    24/10/97  Borja     cambio en lp_Aroots() (root_init)
1.1.3    24/06/97  Borja     bug en lp_Aroots()
1.1.2    04/03/97  Borja     eliminar uso de constructor complex(x,y)
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Calculo de polos en analisis predictivo, utilizando el
algoritmo de Laguerre para extraccion de raices complejas.

Hay codigo condicional, en funcion de como se envien los
coeficientes LPC (sumatorio positivo o negativo: ver SPL5?.c
controlado por el define LP_NEGSUM
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#ifndef __cplusplus
#error Must use C++ compiler
#endif

/**********************************************************/

#include "spl.h"
#include "zroots.hpp"
#include "lpcroots.hpp"

#ifdef __CC_MSVC__
_STD_BEGIN
#endif

/* <DOC> */
/**********************************************************/
/* a partir de los coeficientes lpc de orden {p} en el vector
{vai} ({p} elementos), calcula los ceros del A(z) (polos de H(z))
utilizando el metodo de laguerre para el calculo de raices complejas.

La funcion {devuelve} el numero de polos encontrados, o un numero
negativo en caso de que laguerre() produzca algun error (mirar
laguerre() en zroots.cpp).
Los polos se almacenan como numeros complejos en el array {Roots} (de
{p} elementos).
La funcion necesita un array temporal de tnel_lp_Aroots(p) elementos
de tipo SPL_COMPLEX. */

PUBLIC SPL_INT XAPI lp_Aroots( SPL_pFLOAT vai, SPL_INT p,
		SPL_pCOMPLEX Roots, SPL_pCOMPLEX Tmp, SPL_BOOL root_init )
/* </DOC> */
{
	SPL_INT i, NumRoots;

#ifdef LP_NEGSUM
	for (i=0; i<p; i++)  // crea polinomio complejo
		Tmp[i] = -vai[p-i-1];
#else
	for (i=0; i<p; i++)  // crea polinomio complejo
		Tmp[i] = vai[p-i-1];
#endif
	Tmp[p]=1.0;

	if (root_init)
		for (i=0; i<p; i++) Roots[i]=0;

	i=laguerre(Tmp,p,Roots,NumRoots,Tmp+(p+1),ZROOTS_MAXITER,ZROOTS_TOL);

	if (i>=0)
		return NumRoots;
	else
		return i;
}

/* <DOC> */
/**********************************************************/
/* {devuelve} el numero de elementos de tipo SPL_COMPLEX que debe
enviarse en el array temporal {Tmp} a lp_Aroots(), para un
analisis de orden {p} */

PUBLIC SPL_INT XAPI tnel_lp_Aroots( SPL_INT p )
/* </DOC> */
{
	return tnel_laguerre(p)+(p+1);
}

/* <DOC> */
/**********************************************************/
/*convierte los polos en frecuencias y ancho de banda.
Para {NumRoots} polos pasados en el array {Roots}, devuelve
las correspondientes frecuencias y anchos de banda en los
arrays {f} y {bw} (de {NumRoots} elementos cada uno).
La frecuencia de muestreo se indica en {freq}. */

PUBLIC SPL_VOID XAPI lp_roots2fbw( SPL_pCOMPLEX Roots, SPL_INT NumRoots,
		SPL_pFLOAT f, SPL_pFLOAT bw, SPL_FLOAT freq )
/* </DOC> */
{
	SPL_INT i;
	SPL_FLOAT d;

	for (i=0; i<NumRoots; i++) {
		if (fabs(imag(Roots[i]))<=ZROOTS_TOL)
			//redondea a eje real
			d = (real(Roots[i])>=0) ? 0.0:M_PI;
		else
			d=arg(Roots[i]);
		if (d<0.0)  // pasa de -Pi,Pi a 0,2*Pi
			d+=(2*M_PI);

		f[i]= rad2hz(d,freq);
		bw[i]=r2bw(abs(Roots[i]),freq);
	}
}

/**********************************************************/

#ifdef __CC_MSVC__
_STD_END
#endif
