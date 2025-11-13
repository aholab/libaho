/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao
									& CSTR - Univ. of Edinburgh

Nombre fuente................ -
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    09/04/99  Borja     uso tipos UINT
0.0.2    30/08/98  Borja     split en varios modulos afwav_?.cpp
0.0.1    27/07/97  Borja     adapted to compile on SunOS.
0.0.0    15/07/96  Borja     codificacion inicial.

======================== Contenido ========================
// $$$ arreglar newlines en propiedades
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "afwav_i.hpp"

#include "xalloc.h"

/**********************************************************/

int operator < (const UInt32& a, const UInt32& b) { return a.i<b.i; };
int operator == (const UInt32& a, const UInt32& b) { return a.i==b.i; };
int operator > (const UInt32& a, const UInt32& b) { return a.i>b.i; };

/**********************************************************/

char *wstr( FILE *f )
{
	UINT32 l;
	char *buf;
	xfread(&l,sizeof(l),1,f); endian_fromlittle32(&l);
	l++;
	cdie_beep(l!=(size_t)l,"Wav: field too long in .wav file");
	buf=(char*)xmalloc(l);
	xfread(buf,1,(size_t)(l-1),f);
	buf[(size_t)(l-1)]='\0';
	return buf;
}

/**********************************************************/

char *wstr( FILE *f, UINT32 l )
{
	char *buf;
	l++;
	cdie_beep(l!=(size_t)l,"Wav: field too long in .wav file");
	buf=(char*)xmalloc(l);
	xfread(buf,1,(size_t)(l-1),f);
	buf[(size_t)(l-1)]='\0';
	return buf;
}

/**********************************************************/

VOID mrk_labl(Mark1DList &m, CueNames &cn, CHAR8 *buf)
{
#ifndef IGNORE_CUE_LABEL
	struct { UINT32 n MINALIGN; CHAR s MINALIGN; } *b;
	(CHAR8*)b=buf;
	endian_fromlittle32(&(b->n));

	Lix p=cn.seek(b->n);
	if (p) p=m.lix(cn.index(p));
	cdie_beep(!p,"Wav: error in WAV file, invalid cue label");
	if (m.label(p)=="") m.label(p) = &(b->s);
	else m.label(p) = &(b->s) + String(" // ")+m.label(p);
#else
	(void)m;
	(void)cn;
#endif

	xfree(buf);
}

/**********************************************************/

VOID mrk_note(Mark1DList &m, CueNames &cn, CHAR8 *buf)
{
	struct mrk_note_struct { UINT32 n MINALIGN; CHAR s MINALIGN; };
#define b ((struct mrk_note_struct *)buf)	
	endian_fromlittle32(&(b->n));

	Lix p=cn.seek(b->n);
	if (p) p=m.lix(cn.index(p));
	cdie_beep(!p,"Wav: error in WAV file, invalid cue note");
	if (m.label(p)=="") m.label(p) = &(b->s);
	else m.label(p) = m.label(p) + String(" // ") + &(b->s);

	xfree(buf);
#undef b
}

/**********************************************************/

VOID mrk_ltxt(Mark1DList &m, CueNames &cn, CHAR8 *buf)
{
	struct mrk_ltxt_struct { UINT32 n MINALIGN; UINT32 len MINALIGN; UINT32 pur MINALIGN; 
	  UINT16 country MINALIGN; UINT16 lang MINALIGN; UINT16 dialec MINALIGN; 
		UINT16 cpage MINALIGN; CHAR s MINALIGN; };
#define b ((struct mrk_ltxt_struct *)buf)
	
	endian_fromlittle32(&(b->n));
	endian_fromlittle32(&(b->len));
	// note: endian conversion only for the two used fields!

	Lix p=cn.seek(b->n);
	if (p) p=m.lix(cn.index(p));
	cdie_beep(!p,"Wav: error in WAV file, invalid cue ltxt");
	m.span(p)=b->len;
	xfree(buf);
#undef b
}

/**********************************************************/

size_t strlen_zeven( const char *s )
{
	size_t l = strlen(s)+1;

	return (l&1)? l+1 : l;
}

/**********************************************************/

void puts_zeven( const char * s, FILE *f )
{
	size_t l=strlen(s)+1;
	CHAR8 z='\0';

	xfwrite(s,1,l,f);
	if (l&1)
		xfwrite(&z,sizeof(z),1,f);  // completamos a numero par
}

/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(pop)
#endif

/**********************************************************/
