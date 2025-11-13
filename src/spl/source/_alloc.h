#ifndef ___ALLOC_H
#define ___ALLOC_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ _ALLOC.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... funciones de reserva de memoria
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.1    11/01/93  Borja     include mas estandar <alloc.h> --> <stdlib.h>
1.1.0    11/01/93  Borja     cambio de nombre: XALLOC.H->_ALLOC.H
1.0.0	 11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
Definiciones de las funciones para reserva y liberacion
de memoria dinamica: Filtros _malloc() y _free() que
mapean a malloc() y free() o a las que sea necesario.

Prototipos que deben seguir:

void * _malloc( size )
void _free( void ptr )

_malloc() debe reservar un bloque de {size} BYTES donde size es
del tipo entero adecuado (generalmente size_t o el que use
el compilador) y {devuelve} un puntero generico al bloque, o
NULL si no se puede reservar.

_free() recibe en {ptr} un puntero a un bloque, y debe liberar la
memoria reservada.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>  /* necesario para malloc(), free()... */

#define _malloc(size)  malloc(size)

#define _free(ptr)  free(ptr)

/**********************************************************/

#endif

