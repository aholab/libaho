/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XKBD.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... conio.h
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    28/06/98  Borja     repasillo gordo
0.0.1    14/06/98  Borja     repasillo general
0.0.0    23/11/95  Borja     Codificacion inicial.

======================== Contenido ========================
Gestion de teclado.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <conio.h>
#include <dos.h>
#include <ctype.h>

#include "xkbd.h"

/*<DOC>*/
/**********************************************************/
/* {devuelve} un caracter del buffer de teclado. Si no se ha
pulsado ninguno, se bloquea hasta recibir uno.

Las teclas de funcion {devuelven} un codigo "negativo" (realmente
bit msb a 1, como unsigned es >32767) (en vez de devolver 0 y luego
otro codigo, como hace getch()). En xkbd.h se definen nombres
sencillos para todas las teclas de funcion posibles (constantes K_??). */

UINT16 xkbd_getch( VOID )
/*</DOC>*/
{
	int key = getch();
	if (key) return (UINT16)key;
	return XKBD_FUNC | ((UINT16)getch());
}

/*<DOC>*/
/**********************************************************/
/* {devuelve}!=0 si se ha pulsado alguna tecla. */

BOOL xkbd_kbhit( VOID )
/*</DOC>*/
{
	return kbhit();
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} una mascara combinacion de teclas Shift, Ctrl,
Alt, pulsadas */

UINT16 xkbd_getstat( VOID )
/*</DOC>*/
{
	UINT16 m = peek(0x40,0x17);
	if (m&0x1) m |= 0x2;
	return (m&0xE)<<11;
}

/*<DOC>*/
/**********************************************************/
/*  {devuelve} combinacion de tecla pulsada y bits de estado
shift-ctrl-alt. Es un OR entre xkbd_getstat() y xkbd_getch().

Las macros
xkbd_testshift(n)
xkbd_testctrl(n)
xkbd_testalt(n)

permiten comprobar si es una tecla de funcion, o si shift
ctrl o alt se han pulsado. */

UINT16 xkbd_getchstat( VOID )
/*</DOC>*/
{
	return xkbd_getch()|xkbd_getstat();
}

/**********************************************************/
/* Convierte a mayusculas. Tiene en cuenta que puede ser
un codigo de funcion, en cuyo caso no hace nada. */

UINT16 xkbd_toupper( UINT16 code )
{
	if (xkbd_isfunc(code)) return code;
	return (code & 0xFF00) | toupper((UINT8)code);
}

/**********************************************************/
/* Convierte a minusculas. Tiene en cuenta que puede ser
un codigo de funcion, en cuyo caso no hace nada. */

UINT16 xkbd_tolower( UINT16 code )
{
	if (xkbd_isfunc(code)) return code;
	return (code & 0xFF00) | tolower((UINT8)code);
}

/**********************************************************/

