/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ ACCEL.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... reloj PC
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.1.0    01/07/98  Borja     nuevo algoritmo aceleracion
0.0.1    22/05/96  Borja     bug. en documentacion
0.0.0    23/11/95  Borja     Codificacion inicial.

======================== Contenido ========================
Acelerador (para cursores graficos...)

Uso tipico, algo asi:


INT16 key;
UINT16 step;
BOOL salir;

accel_ini(40,1500);  <-- de 1 a 40 en 1.5 segundos, con Repsol Super Sport
salir = FALSE;

	do {
		key = xgetch();   <-- sig. tecla
		step = accel_get(key);   <-- lee aceleracion

		switch (key) {
		LEFT:                  <-- cursor izquierdo
			pos -= step;         <-- decrementa tanto como indique el acelerador
			if (!xkbnextsame())  <-- si la sig. tecla es igual, no pinta nada
				repinta();
			break;
		RIGHT:
			pos += step;
			if (!xkbnextsame())
				repinta();
			break;
		ESC: 
			salir = TRUE; 
			break;
		default:
			beep();
			break;
		}
	} while (!salir);

===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "accel.h"

/**********************************************************/

#define ms2tic(ms)  ((ms+27)/55)

/**********************************************************/

PRIVATE UINT32 _old_timer;
PRIVATE UINT16 _old_event;
PRIVATE UINT16 _max, _thres;
PRIVATE UINT16 _accel;

/**********************************************************/

PRIVATE UINT32 long get_tic( void )
{
	return *((pfUINT32)0x046C);
}

/**********************************************************/
/* Inicializa acelerador.
{max} es la aceleracion maxima a alcanzar (la minima es 1).
{thres_ms} es el tiempo (en milisegundos) por debajo del cual
se acelera el teclado. Por encima de este tiempo, se resetea
la aceleracion al valor minimo. */

VOID accel_ini( UINT16 max, UINT16 thres_ms )
{
	_old_event = 0;
	_thres= ms2tic(thres_ms);
	_accel = 1;
	_max = max;
	_old_timer = get_tic()-2*_thres;
}

/**********************************************************/
/* {devuelve} el valor de la aceleracion (1 a {max}) a utilizar
para el evento {event}. */

UINT16 accel_get( UINT16 event )
{
	UINT32 timer;

	timer = get_tic();

	if ((timer-_old_timer<_thres)&&(event==_old_event)) {
		if (_accel<_max) _accel++;
	}
	else
		_accel = 1;

	_old_event = event;
	_old_timer = timer;
	return _accel;
}

/**********************************************************/
