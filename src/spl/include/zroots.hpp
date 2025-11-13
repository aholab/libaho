#ifndef __ZROOTS_HPP
#define __ZROOTS_HPP

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ ZROOTS.HPP
Nombre paquete............... SPL
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.1    27/08/95  Borja     spltdef.hpp --> spltdef.h
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Cabecera de ZROOTS.CPP. Calculo de raices complejas de
polinomios complejos (metodo de Laguerre).

Definicion de diversas constantes y codigos necesarios.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#ifndef __cplusplus
#error Must use C++ compiler
#endif

/**********************************************************/

#include "spltdef.h"

/**********************************************************/
// codigos de error de retorno
#define ZROOTS_ERR_NOERR 0  // no hay error
#define ZROOTS_ERR_ITEROUT -1  // demasiadas iteraciones
#define ZROOTS_ERR_NOMEM -2  // no hay memoria (no utilizado)

// valores tipicos de tolerancia y numero de iteraciones
#define ZROOTS_TOL 1e-6
#define ZROOTS_MAXITER 300

// valor minimo considerado como cero al evaluar un polinomio
#define ZROOTS_NEARLYZERO 1e-15

/**********************************************************/


SPL_INT API laguerre( SPL_pCOMPLEX vPoly, SPL_INT & Degree,
		SPL_pCOMPLEX vRoots, SPL_INT & NumRoots,
		SPL_pCOMPLEX Tmp,
		SPL_INT MaxIter = ZROOTS_MAXITER,
		SPL_FLOAT Tol = ZROOTS_TOL );

SPL_INT API tnel_laguerre( SPL_INT Degree );

/**********************************************************/

#endif

