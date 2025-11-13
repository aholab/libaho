/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ xmouse.c
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    28/06/98  Borja     repasillo gordo
0.0.1    14/06/98  Borja     repasillo general
0.0.0    16/08/96  Borja     Codificacion inicial.

======================== Contenido ========================
User interface. Manejo de estados del raton.
Los codigos de evento son compatibles con los de teclado (xkbd()).
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "xmouse.h"

/**********************************************************/

#define XMOUSE_AUTOREP
#define XMOUSE_REP_IDELAY 200
#define XMOUSE_REP_RATE 40
#define XMOUSE_REP_DELAY (1000/XMOUSE_REP_RATE)

#ifdef XMOUSE_AUTOREP
#include "chrono.h"
#endif

/**********************************************************/

PRIVATE BOOL _ml, _mr, _mb;
PRIVATE INT16 _mx, _my;

#ifdef XMOUSE_AUTOREP
PRIVATE chrono _cr;
INT _crms;
#endif

/**********************************************************/

VOID xmouse_initialize( VOID )
{
	_ml = FALSE;
	_mr = FALSE;
	_mb = FALSE;
	_mx = -1;
	_my = -1;
#ifdef XMOUSE_AUTOREP
	_crms=XMOUSE_REP_IDELAY;
	chrono_start(&_cr);
#endif
}

/**********************************************************/

UINT16 xmouse_get( INT16* x, INT16* y )
{
	return xmouse_getstat(x,y) & (~(UINT16)(XMOUSE_LKEY|XMOUSE_RKEY));
}

/**********************************************************/

UINT16 xmouse_getstat( INT16* x, INT16* y )
{
#define LKEY (m&MOUSE_LEFTKEY)
#define RKEY (m&MOUSE_RIGHTKEY)
#ifdef XMOUSE_AUTOREP
#define CRIRESET() (chrono_reset(&_cr), _crms=XMOUSE_REP_IDELAY)
#else
#define CRIRESET()
#endif

	UINT16 m, code;

	m = mouse_get(x,y);
	code=M_None;
	if ((*x!=_mx)||(*y!=_my)) {
		code = M_Move;
		_mx=*x;
		_my=*y;
	}
	if (_mb && !(LKEY && RKEY)) { _mb=FALSE; code=M_BUp; CRIRESET(); }
	else if (_ml && !LKEY) { _ml=FALSE; code=M_LUp; CRIRESET(); }
	else if (_mr && !RKEY) { _mr=FALSE; code=M_RUp; CRIRESET(); }
	else if (!_ml && LKEY) { _ml=TRUE; code=M_LDown; CRIRESET(); }
	else if (!_mr && RKEY) { _mr=TRUE; code=M_RDown; CRIRESET(); }
	else if (!_mb && LKEY && RKEY) { _mb=TRUE; code=M_BDown; CRIRESET(); }
#ifdef XMOUSE_AUTOREP
	else if (LKEY||RKEY) {
		if (chrono_ms(&_cr)>_crms) {
			chrono_reset(&_cr);
			_crms=XMOUSE_REP_DELAY;
			if (LKEY&&RKEY) code=M_BDownRep;
			else if (LKEY) code=M_LDownRep;
			else if (RKEY) code=M_RDownRep;
		}
	}
#endif
	if (LKEY) code |= XMOUSE_LKEY;
	if (RKEY) code |= XMOUSE_RKEY;

	return code;
}

/**********************************************************/
