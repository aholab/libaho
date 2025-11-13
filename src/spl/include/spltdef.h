#ifndef __SPLTDEF_H
#define __SPLTDEF_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPLTDEF.H
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... definicion de tipos fundamentales
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    20/01/98  Borja     soporte MSVC++ (complex)
1.1.1    27/08/95  Borja     derivacion de tdef.h
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/12/94  Borja     Codificacion inicial.

======================== Contenido ========================
Definicion de tipos fundamentales para librerias SPL.

Por ejemplo:

SPL_PTR --> define el operador puntero (*)

SPL_INT --> tipo entero a utilizar en funciones SPL

SPL_pINT --> puntero a {SPL_INT}

Similar con SPL_FLOAT, SPL_VOID, etc.

En SPL se han usado asi los tipos:
SPL_INT .....  entero con signo
SPL_FLOAT ...  valor real
SPL_BOOL ....  valor booleano (SPL_TRUE, SPL_FALSE)
SPL_VOID ....  nulo
SPL_LONG ....  entero extendido; solo se usa en aquellos
.              raros casos en que es MUY evidente que un
.              INT puede sufrir overflow.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"

/**********************************************************/

#define SPL_PTR  PTR        /* Definicion de un puntero */

/**********************************************************/
/* Plantilla de definicion de un nuevo tipo SPL_NAME y puntero
al tipo (SPL_pNAME) (array), para el tipo TYPE. */

#define __SPL_TDEF(TYPE,NAME)  \
	typedef TYPE SPL_##NAME;  \
	typedef SPL_##NAME SPL_PTR SPL_p##NAME;

/**********************************************************/

__SPL_TDEF(INT, INT);   /* define SPL_INT, SPL_pINT */
__SPL_TDEF(DOUBLE, FLOAT);  /* define SPL_FLOAT, SPL_pFLOAT */
__SPL_TDEF(BOOL, BOOL);
__SPL_TDEF(VOID, VOID);
__SPL_TDEF(LONG, LONG);

#ifdef __cplusplus
__SPL_TDEF(COMPLEX, COMPLEX);  /* define SPL_COMPLEX, SPL_pCOMPLEX */
#endif

/**********************************************************/
/* valores booleanos */

#define SPL_FALSE  FALSE
#define SPL_TRUE  TRUE

/**********************************************************/

#endif

