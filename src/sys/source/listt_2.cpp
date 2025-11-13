/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LISTT.CPP
Nombre paquete............... -
Lenguaje fuente.............. C++
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.1.0    07/05/99  Borja     modif ??_mv()
1.0.1    30/08/98  Borja     split varios modulos listt_?.cpp

======================== Contenido ========================
ver doc. en listt.cpp
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "listt_i.hpp"

/**********************************************************/

Lix _SetPList::__seek(const _PList&a, const VOID * x) const
{
	for (Lix p=a.first(); p!=0; p=a.next(p))
		if (_sameData(x,a._item(p))) return p;
	return 0;
}

/**********************************************************/

_PList& _SetPList::_create_d(const _PList& a)
{
	if (h != a.h) { clear(); _add_d(a); }
	return *this;
}

/**********************************************************/

_PList& _SetPList::_create_p(_PList& a)
{
	if (h != a.h) { clear(); _add_p(a); }
	return *this;
}

/**********************************************************/

BOOL _SetPList::OK(VOID) const
{
	if (!_PList::OK()) return FALSE;
	for (Lix p=first(); p!=0; p=next(p))
		if (_seek(_item(p))!=p) return FALSE;
	return TRUE;
}

/**********************************************************/

Lix _SetPList::_add_d(const VOID* x)
{
	if (!_contains(x)) return _append_d(x);
	return NULL;
}

/**********************************************************/

Lix _SetPList::_add_p(VOID* x)
{
	if (!_contains(x)) return _append_p(x);
	return NULL;
}

/**********************************************************/

VOID _SetPList::_add_d(const _PList& a)
{
	for (Lix p=a.first(); p!=0; p=a.next(p))
		if (!_contains(a._item(p))) __append(__newNode_d((_PListNode*)p));
}

/**********************************************************/

VOID _SetPList::_add_p(_PList& a)
{
	for (Lix p=a.first(); p!=0; p=a.next(p))
		if (!_contains(a._item(p))) __append(__newNode_p((_PListNode*)p));
}

/**********************************************************/

VOID _SetPList::_add_mv(_PList& a, Lix ni, Lix nf, BOOL ovwrt, BOOL cdata )
{
	_PListNode* t1 = (_PListNode*)(ni?ni:a.h);
	_PListNode* t2 = (_PListNode*)(nf?nf: a.h?a.h->b:0);
#ifdef LIST_PTRVERIFY
	XLISTVERIFY(a,(Lix)t1,"_add_mv");
	XLISTVERIFY(a,(Lix)t2,"_add_mv");
	if (t1 && t2) assert(a.index(t1)<=a.index(t2));
#endif

	for (Lix p=(Lix)t1; p!=0; ) {
		Lix q = _seek(a._item(p));
		if (!q || ovwrt) {
			if (q) _del(q,cdata);
			q=a.__del(p,1);
			__append((_PListNode*)p);
			if (p==(Lix)t2) break;
			p=q;
		}
		else {
			if (p==(Lix)t2) break;
			p=a.next(p);
		}
	}
}

/**********************************************************/

BOOL _SetPList::_contains(const _PList& a) const
{
	Lix p=a.first();
	if (!p) return (length()==0);
	for ( ; p!=0; p=a.next(p))
		if (!_contains(a._item(p))) return FALSE;
	return TRUE;
}

/**********************************************************/

BOOL _SetPList::_equals(const _PList& a, BOOL allowrep) const
{
	if ((a.length()!=length()) && !allowrep) return FALSE;
	else return _contains(a);
}

/**********************************************************/

BOOL _SetPList::_erase(const VOID* x, BOOL cdata)
{
	Lix p=_seek(x);
	if (p) {
		_del(p,cdata);
		return TRUE;
	}
	return FALSE;
}

/**********************************************************/

BOOL _SetPList::_erase(const _PList& a, BOOL cdata)
{
	BOOL e = FALSE;
	for (Lix p=a.first(); p!=0; p=a.next(p))
		if (_erase(a._item(p),cdata)) e=TRUE;
	return e;
}

/**********************************************************/

VOID _SetPList::_keep(const _PList& a, BOOL cdata)
{
	for (Lix p=first(); p!=0; ) {
		if (__seek(a,_item(p))==0)
			p=_del(p,1,cdata);
		else
			p=next(p);
	}
}

/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/

_PListNode *_KVPList::__newNode_d_kv( const VOID *key, const VOID *val )
{
	VOID * k = _newData(key);
	VOID * v = _newVal(val);
	if ((!k) || (!v))
		LISTERROR("__newNode_d_kv","can't create data object (null new)");

	return __newNode_p_kv(k,v);
}

/**********************************************************/

_PListNode *_KVPList::__newNode_p_kv( VOID *key, VOID *val )
{
	_KVPListNode * t = new _KVPListNode(key,val);

	if (!t)
		LISTERROR("__newNode_p_kv","can't create node object (null new)");

	return t;
}

/**********************************************************/

#ifdef LIST_PTRVERIFY
const VOID *_KVPList::_itemval(Lix p) const { if (p == 0) { LISTNONULL(p,"_itemval"); return 0; }; LISTVERIFY(p,"_itemval"); return ((_KVPListNode*)p)->vp; }
const VOID *_KVPList::_itemval_first(VOID) const { if (h == 0) { LISTERROR("_itemval_first","empty list"); return 0; }; return ((_KVPListNode*)h)->vp; }
const VOID *_KVPList::_itemval_last(VOID) const { if (h == 0) { LISTERROR("_itemval_last","empty list"); return 0; }; return ((_KVPListNode*)h->b)->vp; }
#endif

/**********************************************************/

VOID _KVPList::__exchange_p( _PListNode *n1, _PListNode *n2 )
{
	VOID *tmp;
	__SWAP__( ((_KVPListNode*)n1)->dp, ((_KVPListNode*)n2)->dp, tmp);
	__SWAP__( ((_KVPListNode*)n1)->vp, ((_KVPListNode*)n2)->vp, tmp);
}

/**********************************************************/

Lix _KVPList::_insbefore_d_kv(LIINT i, const VOID* key, const VOID* val)
{
	return _insbefore_d_kv(LIX_SAFE(i,"_insbefore_d_kv",TRUE),key,val);
}

/**********************************************************/

Lix _KVPList::_insbefore_p_kv(LIINT i, VOID* key, VOID* val)
{
	return _insbefore_p_kv(LIX_SAFE(i,"_insbefore_p_kv",TRUE),key,val);
}

/**********************************************************/

Lix _KVPList::_insafter_d_kv(LIINT i, const VOID* key, const VOID* val)
{
	return _insafter_d_kv(LIX_SAFE(i,"_insafter_d_kv",TRUE),key,val);
}

/**********************************************************/

Lix _KVPList::_insafter_p_kv(LIINT i, VOID* key, VOID* val)
{
	return _insafter_p_kv(LIX_SAFE(i,"_insafter_p_kv",TRUE),key,val);
}

/**********************************************************/

const VOID* _KVPList::_itemval(LIINT i) const
{
	return ((_KVPListNode*)LIX_SAFE(i,"_itemval",FALSE))->vp;
}

/**********************************************************/

const VOID * _KVPList::_val( const VOID *key ) const
{
	Lix p=_seek(key);
	if (!p) {
		LISTERROR("_val","non existant key");
		return NULL;
	}
	return ((_KVPListNode*)p)->vp;
}

/**********************************************************/

const VOID * _KVPList::_val( const VOID *key, const VOID *defval ) const
{
	Lix p=_seek(key);
	if (!p) return defval;
	return ((_KVPListNode*)p)->vp;
}

/**********************************************************/
//		BOOL rewrite,  //aunque exista, ponerlo
//		BOOL ovwrt // si hay que machacar el anterior en vez de copiar
//		BOOL dup, // si hay que hacer _p o _d
//		BOOL cdata,  // si hay que borrar lo anterior

Lix _KVPList::_add(const VOID* k, const VOID* v,
		BOOL rewrite, BOOL ovwrt, BOOL dup, BOOL cdata )
{
	Lix p = _seek(k);
	if (!p) {
		if (dup) return _append_d_kv(k,v);
		else return _append_p_kv((VOID*)k,(VOID*)v);
	}
	if (rewrite) {
		if (ovwrt) _copyVal(*__itemvalptr(p),v);
		else {
			if (cdata) _deleteVal(*__itemvalptr(p));
			if (dup) *__itemvalptr(p)=_newVal(v);
			else *__itemvalptr(p)=(VOID*)v;
		}
	}
	return p;
}

/**********************************************************/

VOID _KVPList::_add(const _PList& a,
		BOOL rewrite, BOOL ovwrt, BOOL dup, BOOL cdata )
{
	for (Lix p=a.first(); p!=0; p=a.next(p))
		_add(a._item(p),((_KVPList&)a)._itemval(p),rewrite,ovwrt,dup,cdata);
}

/**********************************************************/
