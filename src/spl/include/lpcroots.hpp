#ifndef __LPCROOTS_HPP
#define __LPCROOTS_HPP

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LPCROOTS.HPP
Nombre paquete............... SPL
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.2.0    24/10/97  Borja     cambio en lp_Aroots() (root_init)
1.1.1    27/08/95  Borja     spltdef.hpp --> spltdef.h
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera de LPCROOTS.CPP
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

SPL_INT API lp_Aroots( SPL_pFLOAT vai, SPL_INT p,
		SPL_pCOMPLEX Roots, SPL_pCOMPLEX Tmp, SPL_BOOL root_init=TRUE );

SPL_INT API tnel_lp_Aroots( SPL_INT p );

SPL_VOID API lp_roots2fbw( SPL_pCOMPLEX Roots, SPL_INT NumRoots,
		SPL_pFLOAT f, SPL_pFLOAT bw, SPL_FLOAT freq );

/**********************************************************/

#endif

