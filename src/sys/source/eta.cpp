/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1997 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ ETA.CPP
Nombre paquete............... -
Lenguaje fuente.............. C
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.0    20/03/97  Borja     inicial.

======================== Contenido ========================
<DOC>
Gestion de "Tiempo Estimado de LLegada" (ETA).
Para hacer tipicas barritas de porcentaje para terminar
el proceso, o simplemente indicar cuanto tiempo se estima
para terminar un proceso.
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>

#include "eta.hpp"

//<DOC>
/**********************************************************/
/* Inicializa (lo mismo que el constructor).
{n} es el "Punto de destino" sabiendo que partimos de 0.
{refreshTimeSec} es el numero de segundos para generar un
valor de refresco (ver getRefresh()) */

VOID ETA::init( DOUBLE n, DOUBLE refreshTimeSec )
//</DOC>
{
	nf = n;
	ni = 0;
	rt = refreshTimeSec;
	ti = -rt*1000;
	c.reset();
	c.start();
}

//<DOC>
/**********************************************************/
/* {devuelve} los segundos estimados para terminar el
proceso. En {pos} debemos indicar la posicion actual (sabiendo
que partimos de 0 y el final es el parametro {n} del constructor
o de la funcion init() */

DOUBLE ETA::getETASec( DOUBLE pos )
//</DOC>
{
	if (!pos) return 0;

	return ((nf-pos)/pos*c.ms())/1000;
}

//<DOC>
/**********************************************************/
/* Como getETASec() solo que {devuelve} una cadena de
texto formateada como hh:mm:ss (con dias si son dias!
xxd/hh:mm:ss)
con el tiempo estimado para terminar el proceso .  La cadena
es estatica e interna, no modificarla ni liberar memoria! */

CHAR *ETA::getETATxt( DOUBLE pos )
//<DOC>
{
	DOUBLE t;
	LONG h, m, s;
	DOUBLE d;
	static CHAR internalstr[40];

	t = getETASec(pos);

	d = floor(t/(24.*3600));
	t -= d*(24.*3600);
	h = (LONG)(t/3600);
	t -= h*3600;
	m = (LONG)(t/60);
	t -= m*60;
	s = (LONG)t;

	if (d) sprintf(internalstr,"%.0fd/%ld:%02ld:%02ld",d,h,m,s);
	else sprintf(internalstr,"%ld:%02ld:%02ld",h,m,s);

	return internalstr;

}

//<DOC>
/**********************************************************/
/* {devuelve} TRUE cuando debemos actualizar (refrescar)
la estimacion de tiempo para terminar, y FALSE el resto del
tiempo. Se produce un refresco cada {refreshTimeSec} segundos
(parametro indicado en init() o en el constructor). */

BOOL ETA::getRefresh( VOID )
//</DOC>
{
	DOUBLE t = c.ms();
	BOOL r = (((t-ti)/1000)>=rt);

	if (r) ti = t;
	return r;
}

//<DOC>
/**********************************************************/
/* {devuelve} el porcentaje del proceso que ya hemos hecho.
Es un valor entre 0 (al inicio) y 100 (al final). En {pos}
debemos indicar la posicion actual (sabiendo que partimos
de 0 y el final es el parametro {n} del constructor o de
la funcion init() */

DOUBLE ETA::getPercent( DOUBLE pos )
//</DOC>
{
	if (!nf) return 100;

	return (pos/nf)*100;
}

/**********************************************************/
