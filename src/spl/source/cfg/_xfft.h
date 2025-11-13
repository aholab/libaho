#ifndef ___XFFT_H
#define ___XFFT_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ _XFFT.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.3.2    30/07/95  Borja     scope funciones explicito
1.3.1    10/02/95  Borja     uso XALLOC.H -> _ALLOC.H
1.3.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.2.0    08/12/94  Borja     no existe. sinc. con otros ficheros
1.1.0    08/12/94  Borja     no existe. sinc. con otros ficheros
1.0.0    16/03/92  Borja     Codificacion inicial.

======================== Contenido ========================
Uso interno de modulos XFFT?.C

NOTA: Para la reserva de memoria dinamica se han definido
dos macros: xfft_malloc() y xfft_free(), que mapean a las
funciones _malloc() y _free(), definidas en _ALLOC.H.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>
#include <assert.h>

#include "xfft.h"

/**********************************************************/
/* Reserva/liberacion de memoria */

#include "_alloc.h"

#define xfft_malloc(n)  _malloc(n)
#define xfft_free(p)  _free(p)

/**********************************************************/

#endif

