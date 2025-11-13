/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ -
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.2    30/08/98  Borja     split en varios modules mark_?.cpp

======================== Contenido ========================
ver mark.cpp
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "mark.hpp"
#include "uti.h"
#include "spl.h"

/**********************************************************/

Lix Mark1DList::seek_p_ms(DOUBLE posms, Lix from) const
{
	return seek_p((LONG)fround(srate?ms2samples(posms,srate):posms),from);
}

/**********************************************************/

Lix Mark1DList::seek_s_ms(DOUBLE spanms, Lix from) const
{
	return seek_s((LONG)fround(srate?ms2samples(spanms,srate):spanms),from);
}

/**********************************************************/

Lix Mark1DList::seek_ps_ms(DOUBLE posms, DOUBLE spanms, Lix from) const
{
	return seek_ps((LONG)fround(srate?ms2samples(posms,srate):posms),
			(LONG)fround(srate?ms2samples(spanms,srate):spanms),from);
}

/**********************************************************/

Lix Mark1DList::seek_pl_ms(DOUBLE posms, String label, Lix from) const
{
	return seek_pl((LONG)fround(srate?ms2samples(posms,srate):posms),label,from);
}

/**********************************************************/

Lix Mark1DList::seek_sl_ms(DOUBLE spanms, String label, Lix from) const
{
	return seek_sl((LONG)fround(srate?ms2samples(spanms,srate):spanms),label,from);
}

/**********************************************************/

Lix Mark1DList::seek_psl_ms(DOUBLE posms, DOUBLE spanms, String label, Lix from) const
{
	return seek_psl((LONG)fround(srate?ms2samples(posms,srate):posms),
			(LONG)fround(srate?ms2samples(spanms,srate):spanms),label,from);
}

/**********************************************************/

Lix Mark1DList::seek_p_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return seek_p((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),from);
}

/**********************************************************/

Lix Mark1DList::seek_s_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return seek_s((LONG)fround(sr?ms2samples(m.span,sr):m.span),from);
}

/**********************************************************/

Lix Mark1DList::seek_ps_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return seek_ps((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),
		(LONG)fround(sr?ms2samples(m.span,sr):m.span),from);
}

/**********************************************************/

Lix Mark1DList::seek_pl_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return seek_pl((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),m.label,from);
}

/**********************************************************/

Lix Mark1DList::seek_sl_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return seek_sl((LONG)fround(sr?ms2samples(m.span,sr):m.span),m.label,from);
}

/**********************************************************/

Lix Mark1DList::seek_psl_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return seek_psl((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),
			(LONG)fround(sr?ms2samples(m.span,sr):m.span),m.label,from);
}

/**********************************************************/

Lix Mark1DList::rseek_p_ms(DOUBLE posms, Lix from) const
{
	return rseek_p((LONG)fround(srate?ms2samples(posms,srate):posms),from);
}

/**********************************************************/

Lix Mark1DList::rseek_s_ms(DOUBLE spanms, Lix from) const
{
	return rseek_s((LONG)fround(srate?ms2samples(spanms,srate):spanms),from);
}

/**********************************************************/

Lix Mark1DList::rseek_ps_ms(DOUBLE posms, DOUBLE spanms, Lix from) const
{
	return rseek_ps((LONG)fround(srate?ms2samples(posms,srate):posms),
			(LONG)fround(srate?ms2samples(spanms,srate):spanms),from);
}

/**********************************************************/

Lix Mark1DList::rseek_pl_ms(DOUBLE posms, String label, Lix from) const
{
	return rseek_pl((LONG)fround(srate?ms2samples(posms,srate):posms),label,from);
}

/**********************************************************/

Lix Mark1DList::rseek_sl_ms(DOUBLE spanms, String label, Lix from) const
{
	return rseek_sl((LONG)fround(srate?ms2samples(spanms,srate):spanms),label,from);
}

/**********************************************************/

Lix Mark1DList::rseek_psl_ms(DOUBLE posms, DOUBLE spanms, String label, Lix from) const
{
	return rseek_psl((LONG)fround(srate?ms2samples(posms,srate):posms),
			(LONG)fround(srate?ms2samples(spanms,srate):spanms),label,from);
}

/**********************************************************/

Lix Mark1DList::rseek_p_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return rseek_p((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),from);
}

/**********************************************************/

Lix Mark1DList::rseek_s_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return rseek_s((LONG)fround(sr?ms2samples(m.span,sr):m.span),from);
}

/**********************************************************/

Lix Mark1DList::rseek_ps_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return rseek_ps((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),
		(LONG)fround(sr?ms2samples(m.span,sr):m.span),from);
}

/**********************************************************/

Lix Mark1DList::rseek_pl_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return rseek_pl((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),m.label,from);
}

/**********************************************************/

Lix Mark1DList::rseek_sl_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return rseek_sl((LONG)fround(sr?ms2samples(m.span,sr):m.span),m.label,from);
}

/**********************************************************/

Lix Mark1DList::rseek_psl_ms(const Mark1D& m, DOUBLE sr, Lix from) const
{
	return rseek_psl((LONG)fround(sr?ms2samples(m.pos,sr):m.pos),
			(LONG)fround(sr?ms2samples(m.span,sr):m.span),m.label,from);
}

/**********************************************************/
