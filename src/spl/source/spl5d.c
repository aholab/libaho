/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL5d.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG, LP_NEGSUM

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    16/05/96  Borja     bugs en documentacion
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.1    07/12/93  Borja     soporte LP_NEGSUM_NEG() y LP_POSSUM_NEG()
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Algoritmos de prediccion lineal (LP). Ver SPL5A.C para mas detalles.

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
/* Metodo de las covarianzas. {mcv} es la pseudo matriz de covarianzas
de orden {p}. Efectua el analisis predictivo de orden {p} y devuelve
los coeficientes LPC en el vector {vai}.
Esta funcion NO {devuelve} la potencia del error (???_ne).
{vtmp} es de tnel_lpa_cov_chl_a(p) elementos. {p}>0 */

PUBLIC SPL_VOID XAPI lpa_cov_chl_a_ne( SPL_pFLOAT mcv, SPL_pFLOAT vai,
		SPL_INT p, SPL_pFLOAT vtmp )
{
#ifndef LP_NEGSUM
	SPL_INT j;
#endif
	assert(p>0);

	matrix_sim2tri_dgn(mcv+p+1,vtmp+p,vtmp,p);
	ecsys_tri_dgn(vtmp+p,vtmp,mcv+1,p,vai);

#ifndef LP_NEGSUM
	for (j=0; j<p; j++)
		vai[j]=-vai[j];
#endif
}

/**********************************************************/
/* Metodo de las covarianzas por cholesky.
{mcv} es la pseudo matriz de covarianzas de orden {p}.
Los coeficientes LPC de orden {p} se devuelven en el vector {vai}.
La funcion {devuelve} la potencia del error.
{vtmp} es de tnel_lpa_cov_chl_a(p) elementos. {p}>0 */

PUBLIC SPL_FLOAT XAPI lpa_cov_chl_a( SPL_pFLOAT mcv, SPL_pFLOAT vai,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_FLOAT sum;
	SPL_INT j;
	assert(p>0);

	matrix_sim2tri_dgn(mcv+p+1,vtmp+p,vtmp,p);
	ecsys_tri_dgn(vtmp+p,vtmp,mcv+1,p,vai);

	sum=mcv[0];
	for (j=0; j<p; j++) {
		sum-=vai[j]*mcv[j+1];
#ifndef LP_NEGSUM
		vai[j]=-vai[j];
#endif
	}

	return sum;
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cov_chl( SPL_INT p )
{
	assert(p>0);

	return nel_matrix_tri(p-1)+nel_matrix_dgn(p);
}

/**********************************************************/
/* Metodo de las covarianzas por choleski, estabilizado.
{mcv} es la pseudo matriz de covarianzas de orden {p}.
En el vector {vki} se devuelven los coeficientes de reflexion.
La funcion {devuelve} la potencia del error.
{vtmp} es de tnel_lpa_cov_sch_k(p) elementos. p>0 */

PUBLIC SPL_FLOAT XAPI lpa_cov_sch_k( SPL_pFLOAT mcv, SPL_pFLOAT vki,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT j;
	SPL_FLOAT sum, osum;
	assert(p>0);

	matrix_sim2tri(mcv+p+1,vtmp,p);
	ecsys_tri(vtmp,mcv+1,p,vki);

	sum=osum=mcv[0];

	for (j=0; j<p; j++) {
		sum-=vki[j]*vki[j];
		vki[j] /= LP_POSSUM_NEG(sqrt(osum));
		osum=sum;
	}

	return sum;
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cov_sch_k( SPL_INT p )
{
	assert(p>0);

	return nel_matrix_tri(p);
}

/**********************************************************/

