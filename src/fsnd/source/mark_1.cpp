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

PRIVATE int cmpf( const Mark1D*a, const Mark1D*b)
{
return Mark1D::compare_psl(*a,*b);
}

/**********************************************************/

PRIVATE int rcmpf( const Mark1D*a, const Mark1D*b)
{
return -Mark1D::compare_psl(*a,*b);
}

/**********************************************************/

VOID Mark1DList::sort( BOOL reverse )
{
	if (reverse) l.sortf(rcmpf);
	else l.sortf(cmpf);
}

/**********************************************************/

Lix Mark1DList::seek_p(LONG pos, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if (pos==l(p).pos) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::seek_s(LONG span, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if (span==l(p).span) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::seek_l(String label, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if (!compare(label,l(p).label)) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::seek_ps(LONG pos, LONG span, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if ((pos==l(p).pos)&&(span==l(p).span)) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::seek_pl(LONG pos, String label, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if ((pos==l(p).pos)&&(!compare(label,l(p).label))) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::seek_sl(LONG span, String label, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if ((span==l(p).span)&&(!compare(label,l(p).label))) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::seek_psl(LONG pos, LONG span, String label, Lix from) const
{
	for (Lix p=from?from:l.first(); p!=0; p=l.next(p))
		if ((pos==l(p).pos)&&(span==l(p).span)&&(!compare(label,l(p).label)))
			return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_p(LONG pos, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if (pos==l(p).pos) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_s(LONG span, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if (span==l(p).span) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_l(String label, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if (!compare(label,l(p).label)) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_ps(LONG pos, LONG span, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if ((pos==l(p).pos)&&(span==l(p).span)) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_pl(LONG pos, String label, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if ((pos==l(p).pos)&&(!compare(label,l(p).label))) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_sl(LONG span, String label, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if ((span==l(p).span)&&(!compare(label,l(p).label))) return p;
	return 0;
}

/**********************************************************/

Lix Mark1DList::rseek_psl(LONG pos, LONG span, String label, Lix from) const
{
	for (Lix p=from?from:l.last(); p!=0; p=l.prev(p))
		if ((pos==l(p).pos)&&(span==l(p).span)&&(!compare(label,l(p).label)))
			return p;
	return 0;
}

/**********************************************************/

VOID Mark1DList::uniq_p( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_p(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

VOID Mark1DList::uniq_s( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_s(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

VOID Mark1DList::uniq_l( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_l(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

VOID Mark1DList::uniq_ps( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_ps(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

VOID Mark1DList::uniq_pl( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_pl(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

VOID Mark1DList::uniq_sl( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_sl(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

VOID Mark1DList::uniq_psl( VOID )
{
	for (Lix p=first(); p!=0; p=next(p)) {
		Lix q=next(p);
		while (q) if ((q=seek_psl(l(p),q))!=0) q=del(q,1);
	}
}

/**********************************************************/

