/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ MOUSE.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... MSDOS
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.1.0    26/06/98  Borja     soporte cursor software (mfunc)
0.0.0    16/08/96  Borja     Codificacion inicial.

======================== Contenido ========================
Manejo del raton
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <dos.h>

#include "mouse.h"

/**********************************************************/

#ifdef MOUSESOFT
VOID (*_mfunc)(BOOL show, INT16 x, INT16 y);
INT _omc;
INT16 _omx, _omy;
#endif

/**********************************************************/
/* {devuelve} !=0 en caso de error */

BOOL mouse_open( VOID )
{
	struct REGPACK preg;
#ifdef MOUSESOFT
	_mfunc=0;
	_omc=1; /*hiden*/
#endif
	preg.r_ax = 0;
	intr(0x33,&preg);
	return (preg.r_ax==0);
}

/**********************************************************/

VOID mouse_close( VOID )
{
	mouse_hide();
}

/**********************************************************/

PRIVATE VOID mouse_xshow( VOID )
{
	struct REGPACK preg;
	preg.r_ax = 1;
	intr(0x33,&preg);
}

/**********************************************************/

PRIVATE VOID mouse_xhide( VOID )
{
	struct REGPACK preg;
	preg.r_ax = 2;
	intr(0x33,&preg);
}

/**********************************************************/

PRIVATE UINT16 mouse_xget( INT16* x, INT16* y )
{
	struct REGPACK preg;
	preg.r_ax = 3;
	intr(0x33,&preg);
	*x = preg.r_cx;
	*y = preg.r_dx;
	return preg.r_bx;
}

/**********************************************************/

VOID mouse_show( VOID )
{
#ifdef MOUSESOFT
	if (_mfunc) {
		if (!_omc) return;
		_omc--;
		if (!_omc) {
			mouse_xget(&_omx,&_omy);
			_mfunc(TRUE,_omx,_omy);
		}
		return;
	}
#endif

	mouse_xshow();
}

/**********************************************************/

VOID mouse_hide( VOID )
{
#ifdef MOUSESOFT
	if (_mfunc) {
		_omc++;
		if (_omc==1) _mfunc(FALSE,_omx,_omy);
		return;
	}
#endif

	mouse_xhide();
}

/**********************************************************/

VOID mouse_set_scale( INT16 xratio, INT16 yratio )
{
	struct REGPACK preg;
	preg.r_ax = 15;
	preg.r_cx = xratio;
	preg.r_dx = yratio;
	intr(0x33,&preg);
}

/**********************************************************/

UINT16 mouse_get( INT16* x, INT16* y )
{
	UINT16 c;
	c=mouse_xget(x,y);

#ifdef MOUSESOFT
	if (_mfunc&&(!_omc)&&((*x!=_omx)||(*y!=_omy))) {
		_mfunc(FALSE,_omx,_omy);
		_mfunc(TRUE,*x,*y);
		_omx=*x;
		_omy=*y;
	}
#endif

	return c;
}

/**********************************************************/

VOID mouse_set_xy( INT16 x, INT16 y )
{
	struct REGPACK preg;
	preg.r_ax = 4;
	preg.r_cx = x;
	preg.r_dx = y;
	intr(0x33,&preg);
}

/**********************************************************/

VOID mouse_set_xrange( INT16 x0, INT16 x1 )
{
	struct REGPACK preg;
	preg.r_ax = 7;
	preg.r_cx = x0;
	preg.r_dx = x1;
	intr(0x33,&preg);
}

/**********************************************************/

VOID mouse_set_yrange( INT16 y0, INT16 y1 )
{
	struct REGPACK preg;
	preg.r_ax = 8;
	preg.r_cx = y0;
	preg.r_dx = y1;
	intr(0x33,&preg);
}

/**********************************************************/

VOID mouse_set_icon( mouse_cursor * mc )
{
	struct REGPACK preg;

	preg.r_ax = 9;
	preg.r_bx = mc->horz_hot_spot;
	preg.r_cx = mc->vert_hot_spot;
	preg.r_dx = FP_OFF(&(mc->mask));
	preg.r_es = FP_SEG(&(mc->mask));
	intr(0x33,&preg);
}

/**********************************************************/

VOID mouse_set_showfunc( VOID (*mfunc)(BOOL show, INT16 x, INT16 y) )
{
#ifdef MOUSESOFT
	if (_mfunc) { if (!_omc) _mfunc(FALSE,_omx,_omy); }
	else mouse_xhide();

	_mfunc=mfunc;
	if (_mfunc) {
		if (!_omc) { mouse_xget(&_omx,&_omx); _mfunc(TRUE,_omx,_omy); }
	}
	else mouse_xshow();
#endif
}

/**********************************************************/

VOID mouse_update(VOID)
{
#ifdef MOUSESOFT
	INT16 x, y;
	mouse_get(&x,&y);
#endif
}

/**********************************************************/
