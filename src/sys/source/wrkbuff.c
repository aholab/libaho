/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DET) ETSII/IT-Bilbao

Nombre fuente................ wrkbuff.c
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... en desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG, NOWRKBUFF

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.0    07/06/95  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Para usar un buffer de trabajo global y no tener que
andar haciendo mallocs/free, ni chupando mucha pila.

En este modulo simplemente se define una variable global
que se llama {_wrkbuff} y que es un array de CHARs de
longitud WRKBUFFLEN.

Cuando una funcion quiere usar un buffer temporal, como
suele ser normal al hacer operaciones con cadenas de
texto, podemos usar este buffer {_wrkbuff}, siempre que sea
suficientemente grande para las necesidades de la funcion.
Nos ahorramos asi el overhead de malloc/free y el chupar
demasiado espacio de pila con variables automaticas.

Conviene que la funcion haga WRKBUFF(n) justo despues de
definir las variables automaticas (es decir, ponerlo como
primera sentencia a ejecutar dentro de la funcion).
WRKBUFF(n) va a asegurarse de que el buffer global {_wrkbuff}
tenga por lo menos {n} elementos. De no ser asi, el programa
se interrumpe (se utiliza assert()). La longitud del
buffer {_wrkbuff} se selecciona a la hora de compilar, en el
fichero WRKBUFF.H, definido como WRKBUFFLEN.

Si se define NDEBUG, la sentencia assert() desaparece, por
lo que no se comprueba que {_wrkbuff} sea suficientemente
grande para las necesidades de la funcion!!! Conviene definir
NDEBUG solo al final del desarrollo, cuando estamos seguros de
que todo nuestro codigo es estable.

Si se define NOWRKBUFF, la variable global {_wrkbuff} no se
define, y la sentencia WRKBUFF(n) se sustituye por una
definicion automatica de la variable {_wrkbuff} en la pila:

CHAR _wrkbuff[n]

es decir, se usa una cadena local a la funcion en vez de
un buffer global al programa.

IMPORTANTE:  no modificar NUNCA el puntero {_wrkbuff}, por
ejemplo, no hacer _wrkbuff++ ni cosas de esas!!.
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#define __WRKBUFF_C__

#include "wrkbuff.h"

/**********************************************************/
/* Define el buffer de trabajo, excepto si NOWRKBUFF esta
definido, en cuyo caso no se usa un buffer global, sino que
se usaran variables locales automaticas (en pila). */

#ifndef NOWRKBUFF

PUBLIC CHAR __wrkbuff[WRKBUFFLEN];
#ifdef __OS_MSDOS__
PUBLIC CHAR _far *_wrkbuff=__wrkbuff;
#else
PUBLIC CHAR *_wrkbuff=__wrkbuff;
#endif

#ifndef NDEBUG
PUBLIC BOOL _wrkbuff_used=FALSE;
#endif

#endif

/**********************************************************/
