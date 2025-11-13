/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ CHRONO.C
Nombre paquete............... -
Lenguaje fuente.............. C (BC31/GCC)
Estado....................... Utilizable
Dependencia Hard/OS.......... SI
Codigo condicional........... SI

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.1    20/03/97  Borja     funcionar en unix
1.0.0    17/09/92  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Implementacion de un cronometro

La estructura chrono implementa un cronometro, con 'botones'
de start/stop/reset.
  * start : pone en marcha el crono.
  * stop : detiene en crono.
  * reset : pone a 0 la cuenta del crono.

El metodo chrono_on() [devuelve] un valor distinto de cero si el
crono esta en marcha.
El metodo chrono_ms() [devuelve] el valor actual del crono en milisegundos,
tanto si esta en marcha como si esta detenido.

Nota de compilacion de chrono.c:
Si se define el simbolo SYNC_CHRONO, al arrancar el reloj
se espera a que se produzca una transicion en el reloj del
sistema. Esto hace que las medidas de tiempo sean mas
precisas, pues asi se evita contabilizar por igual ciclos a
medio terminar y recien comenzados. El inconveniente es
que se utiliza codigo no expandible inline
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "chrono.h"
#include "arch.h"
#include "tdef.h"

/**********************************************************/

#define SYNC_CHRONO

/**********************************************************/

#if defined(__OS_MSDOS__) || defined(__OS_WIN31__)
#else
#include <stdlib.h>
#include <time.h>
#endif

PRIVATE unsigned long get_system_ms( void )
{
#if defined(__OS_MSDOS__) || defined(__OS_WIN31__)
	static unsigned long i=0;
	i = (*((unsigned long far *)((void _seg *)0x0000+(void near *)0x046C)))*55;
	return i;
#else
	 return time(NULL)*1000;
#endif
}

/**********************************************************/
/*<DOC>*/
void chrono_initialize( chrono * c )
/*</DOC>*/
{
c->_on = 0;
c->_ms = 0;
}

/**********************************************************/
/*<DOC>*/
void chrono_reset( chrono * c )
/*</DOC>*/
{
if (c->_on)
  c->_ms = get_system_ms();
else
  c->_ms = 0;
}

/**********************************************************/
/*<DOC>*/
void chrono_start( chrono * c )
/*</DOC>*/
{
if (!c->_on) {
#ifdef SYNC_CHRONO
	unsigned long ot = get_system_ms();
	while (ot==get_system_ms()) ;     // espera una transicion
#endif
	c->_ms = get_system_ms() - c->_ms;
	c->_on = 1;
	}
}

/**********************************************************/
/*<DOC>*/
void chrono_stop( chrono * c )
/*</DOC>*/
{
if (c->_on) {
	c->_ms = get_system_ms() - c->_ms;
	c->_on = 0;
	}
}

/**********************************************************/
/*<DOC>*/
void chrono_restart( chrono * c )
/*</DOC>*/
{
chrono_stop(c);
chrono_reset(c);
chrono_start(c);
}

/**********************************************************/
/*<DOC>*/
void chrono_restop( chrono * c )
/*</DOC>*/
{
chrono_stop(c);
chrono_reset(c);
}

/**********************************************************/
/*<DOC>*/
int chrono_on( chrono * c )
/*</DOC>*/
{
return c->_on;
}

/**********************************************************/
/*<DOC>*/
unsigned long chrono_ms( chrono * c )
/*</DOC>*/
{
return ( (c->_on) ? get_system_ms() - c->_ms : c->_ms);
}

/**********************************************************/
