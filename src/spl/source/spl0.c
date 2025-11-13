/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL0.C
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
Evaluacion de funciones diversas. Por ahora, solo ha sido
necesario implementar la funcion de Bessel de primera
especie y orden 0.

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
/* evalua y {devuelve} la funcion modificada de Bessel de
primera especie y de orden 0 en el punto {x}. El resultado
tiene una precision minima dada por la constante BESSEL_PRECISION,
definida en SPL.H

Ref:
.    Manual de formulas y tablas matematicas.
.    Murray R. Spiegel
.    Schaum - McGraw-Hill
*/

PUBLIC SPL_FLOAT XAPI bessel_I0( SPL_FLOAT x )
{
	SPL_FLOAT sum, coef, i;

	sum = i = 0.0;
	coef = 1.0;
	x *= x;
	do {
		sum += coef;
		i += 2.0;
		coef *= x/(i*i);
	} while (coef/sum>BESSEL_PRECISION);

	return (sum + coef);
}

/**********************************************************/

