#ifndef __CHRONO_HPP__
#define __CHRONO_HPP__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ CHRONO.HPP
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... SI (timer PC)
Codigo condicional........... SYNC_CHRONO

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
2.0.0    16/05/97  Borja     rename class: chrono -> Chrono
1.0.1    20/03/97  Borja     funcionar en unix
1.0.0    17/09/92  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Implementacion de un cronometro. Para simplificar el
manejo, todos los metodos son inline y definidos en
este modulo, de forma que no es necesario un modulo
chrono.cpp.

La clase Chrono implementa un cronometro, con 'botones'
de start/stop/reset.
  * start : pone en marcha el crono.
  * stop : detiene en crono.
  * reset : pone a 0 la cuenta del crono.

El metodo on() [devuelve] un valor distinto de cero si el
crono esta en marcha.
El metodo ms() [devuelve] el valor actual del crono en milisegundos,
tanto si esta en marcha como si esta detenido.

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

#ifndef __cplusplus
#error Must use C++ compiler
#endif

/**********************************************************/

#include "arch.h"

#define SYNC_CHRONO

/**********************************************************/
//<DOC>

class Chrono {
  private :
	unsigned long _ms;
	int _on;

	inline unsigned long timer_ms( void );

  public :
	Chrono( void );  // constructor

	void reset( void );  // resetea el cronometro
	void start( void );  // arranca el cronometro
	void stop( void );  // para el cronometro
	void restart( void );  // resetea y arranca el cronometro
	void restop( void );  // para y resetea el cronometro

	int on( void );  // 1 si el crono esta en marcha
	unsigned long ms( void );  // {devuelve} el valor
							   // actual del crono (en milisegundos)
	};

//</DOC>
/**********************************************************/

#if !defined(__OS_MSDOS__) && !defined(__OS_WIN31__)
#include <stdlib.h>
#include <time.h>
#endif

inline unsigned long Chrono::timer_ms( void )
{
#if defined(__OS_MSDOS__) || defined(__OS_WIN31__)
	static unsigned long i=0;
	i = (*((unsigned long far *)((void _seg *)0x0000+(void near *)0x046C)))*55;
	return i;
#else
	 return time(NULL)*1000;
#endif
}

inline Chrono::Chrono( void )
{
_ms = _on = 0;
}

inline void Chrono::reset( void )
{
if (_on)
	_ms = timer_ms();
else
  _ms = 0;
}

inline void Chrono::start( void )
{
if (!_on) {
#ifdef SYNC_CHRONO
	unsigned long ot = timer_ms();
	while (ot==timer_ms())  {};     // espera una transicion
#endif
  _ms = timer_ms() - _ms;
  _on = 1;
  }
}

inline void Chrono::stop( void )
{
if (_on) {
  _ms = timer_ms() - _ms;
  _on = 0;
  }
}

inline void Chrono::restart( void )
{
reset();
start();
}

inline void Chrono::restop( void )
{
stop();
reset();
}

inline int Chrono::on( void )
{
return _on;
}

inline unsigned long Chrono::ms( void )
{
return ( (_on) ? timer_ms() - _ms : _ms);
}

/**********************************************************/

#endif
