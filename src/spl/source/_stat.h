#ifndef ___STAT_H
#define ___STAT_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ _STAT.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.2    30/07/95  Borja     scope funciones explicito
1.0.1    10/02/95  Borja     uso XALLOC.H -> _ALLOC.H
1.0.0    10/02/95  Borja     Codificacion inicial.

======================== Contenido ========================
Uso interno de STAT.C

NOTA: Para la reserva de memoria dinamica se han definido
dos macros: stat_malloc() y stat_free(), que mapean a las
funciones _malloc() y _free(), definidas en _ALLOC.H.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>
#include <assert.h>

#include "stat.h"

/**********************************************************/
/* Reserva/liberacion de memoria */

#include "_alloc.h"

#define stat_malloc(n)  _malloc(n)
#define stat_free(p)  _free(p)

/**********************************************************/

#endif

