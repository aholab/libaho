/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ uti_misc.c
Nombre paquete............... uti.h
Lenguaje fuente.............. C (BC31,GCC)
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
2.1.0    22/07/97  Borja     show_srcpos
2.0.1    01/07/97  Borja     fround() pasa a otro modulo
2.0.0    01/05/97  Borja     mix de xfiles, syst, xsyst, fname, vstr...
1.2.2    28/04/97  Borja     bug en fgetlns()
1.2.1    08/04/97  Borja     retoques en documentacion
1.2.0    31/07/96  Borja     die
1.1.0    30/07/96  Borja     ftrunc, fmoven, finsn, fdeln.
1.0.0    11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
Funciones diversas.
Generacion de mensajes de error (die).
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "uti.h"

/**********************************************************/
/* imprime un mensaje a stderr y sale al SO con codigo 1.
Los parametros son como con el printf() */

void die( const char *fmt, ... )
{
	va_list argptr;

	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
	exit(1);
}

/**********************************************************/
/* Como die, pero precede al mensaje con un \n, y al
final mete otro \n con un \7 (beep!) delante. */

void die_beep( const char *fmt, ... )
{
	va_list argptr;

	fprintf(stderr, "\n");
	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
	fprintf(stderr, "\7\n");

	exit(1);
}

/**********************************************************/
/* Como die, pero solo si se cumple {condition} */

void cdie( int condition, const char *fmt, ... )
{
   va_list argptr;

	if (!condition)
		return;

	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
	exit(1);
}

/**********************************************************/
/* Como diebeep, pero solo si se cumple {condition} */

void cdie_beep( int condition, const char *fmt, ... )
{
	va_list argptr;

	if (!condition)
		return;

	fprintf(stderr, "\n");
	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
	fprintf(stderr, "\7\n");

	exit(1);
}

/**********************************************************/
/* version va_list de die() */

void vdie( const char *fmt, va_list argptr )
{
	vfprintf(stderr, fmt, argptr);
	exit(1);
}

/**********************************************************/
/* version va_list de die_beep() */

void vdie_beep( const char *fmt, va_list argptr )
{
	fprintf(stderr, "\n");
	vfprintf(stderr, fmt, argptr);
	fprintf(stderr, "\7\n");
	exit(1);
}

/**********************************************************/
/* version va_list de cdie() */

void vcdie( int condition, const char *fmt, va_list argptr )
{
	if (!condition)
		return;

	vfprintf(stderr, fmt, argptr);
	exit(1);
}

/**********************************************************/
/* version va_list de cdie_beep() */

void vcdie_beep( int condition, const char *fmt, va_list argptr )
{
	if (!condition)
		return;

	fprintf(stderr, "\n");
	vfprintf(stderr, fmt, argptr);
	fprintf(stderr, "\7\n");

	exit(1);
}

/**********************************************************/

