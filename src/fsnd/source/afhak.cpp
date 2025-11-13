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
1.0.1    09/04/99  Borja     uso tipos UINT
0.0.0    15/07/96  Borja     codificacion inicial.

======================== Contenido ========================
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"
#include "caudiox.hpp"
#include "uti.h"
#include "xalloc.h"

/**********************************************************/

PRIVATE CHAR *_hak_id8 = "AhoHak1\n";

#define FIELD_PROP 10
#define FIELD_MARK 15
#define FIELD_EOH  3

/**********************************************************/

INT32 int32read( FILE *f, BOOL bigendian )
{
  INT32 n;
  
  xfread(&n,sizeof(n),1,f);
  if (bigendian) endian_frombig32(&n);
  else endian_fromlittle32(&n);
  return n;
}

/**********************************************************/

VOID int32write( INT32 n, FILE *f, BOOL bigendian )
{
  if (bigendian) endian_tobig32(&n);
  else endian_tolittle32(&n);
  xfwrite(&n,sizeof(n),1,f);
}

/**********************************************************/

VOID strwrite( const CHAR *s, FILE *f, BOOL bigendian )
{
  INT32 len;
  
  len = strlen(s);
  int32write(len,f,bigendian);
  if (len) xfwrite(s,1,(size_t)len,f);
}

/**********************************************************/

CHAR * strread( FILE *f, BOOL bigendian )
{
  CHAR *s;
  INT32 len = int32read(f,bigendian);
	INT ilen=(INT)len;
	cdie_beep(len+1 != ilen+1, "String too long (%ld) for this machine's arquitecture",(long)len);
	s=(CHAR*)xmalloc(ilen+1);
	s[ilen]='\0';
	if (ilen) xfread(s,1,ilen,f);
  return s;
}

/**********************************************************/

LONG AFHak::HdrR( FILE *f, KVStrList &, BOOL override )
{
	LONG nSamp;
  UINT8 byte;
  BOOL bigendian;

	if (!testFile(f))
		fprintf(stderr,"%s warning: probably not a .hak file!\n",fFormat());
	xfseek(f,8,SEEK_SET);
  
  xfread(&byte,sizeof(byte),1,f);
  bigendian=(byte!=0);
  ADDIFNOV(CAUDIO_BIGENDIAN,bigendian?"yes":"no");

  while (1) {
    xfread(&byte,sizeof(byte),1,f);
    if (byte==FIELD_EOH) break;
    else if (byte==FIELD_PROP) {
      char *k=strread(f,bigendian);
      char *v=strread(f,bigendian);      
      fa->info().add(k,v);
      xfree(k);
      xfree(v);
    }
    else if (byte==FIELD_MARK) {
      LONG pos = int32read(f,bigendian);
      LONG span = int32read(f,bigendian);
      char *lbl=strread(f,bigendian);
			fa->marks().append(pos,span,lbl);
      xfree(lbl);
    }
    else die_beep("%s error: invalid field code in HAK file (%d)",fFormat(),(int)byte);
  }
  /* calculamos la longitud de la cabecera */
	lc = xftell(f);

	nSamp = fa->info().lval(CAUDIO_NSAMPLES,-1);
  fa->info().add(CAUDIO_SAMPTYPE,SAMPTYPE_STR_FLOAT64); // siempre float64

	return nSamp;
}

/**********************************************************/
/* escribe a disco la cabecera */

VOID AFHak::HdrW( FILE *f, LONG nSamp )
{
  LONG nlc;
  UINT8 byte;
  Lix p;
  BOOL bigendian;

	ADD(CAUDIO_NSAMPLES,nSamp);  //siempre
	ADD(CAUDIO_SRATE,fa->getSRate());  // siempre
	ADD(CAUDIO_NCHAN,(LONG)fa->getNChan());

	// calc. nueva longitud.
  nlc=8+2;
  for (p=fa->info().first(); p!=0; p=fa->info().next(p)) {
    nlc++;
		nlc += (4+strlen(fa->info().itemkey(p)));
		nlc += (4+strlen(fa->info().itemval(p)));
  }
  for (p=fa->marks().first(); p!=0; p=fa->marks().next(p)) {
    nlc+= 1+4+4;
    nlc+= (4+strlen(fa->marks().label(p)));
  }

	// nueva cabecera mayor que la original => inserta hueco
	if (nlc>lc) xfinsn(f,0,nlc-lc);
	// nueva menor que original => quita hueco
	else if (nlc<lc) xfdeln(f,0,lc-nlc);
	lc = nlc;  // salva nueva longitud

	fseek(f,0,SEEK_SET);
	xfwrite(_hak_id8,8,1,f);
	bigendian = fa->getBigEndian();
	byte= bigendian? 1:0;
	xfwrite(&byte,sizeof(byte),1,f);

	byte=FIELD_PROP;
	for (p=fa->info().first(); p!=0; p=fa->info().next(p)) {
		xfwrite(&byte,sizeof(byte),1,f);
		strwrite(fa->info().itemkey(p),f,bigendian);
		strwrite(fa->info().itemval(p),f,bigendian);
	}

	byte=FIELD_MARK;
	for (p=fa->marks().first(); p!=0; p=fa->marks().next(p)) {
		xfwrite(&byte,sizeof(byte),1,f);
		int32write(fa->marks().pos(p),f,bigendian);
		int32write(fa->marks().span(p),f,bigendian);
		strwrite(fa->marks().label(p),f,bigendian);
	}

	byte=FIELD_EOH;
	xfwrite(&byte,sizeof(byte),1,f);

	cdie_beep(lc!=xftell(f),
		"%s error: header length incorrectly computed (%d!=%d)",fFormat(),(int)lc,(int)xftell(f));
}

/**********************************************************/
/* genera una cabecera nueva */

VOID AFHak::HdrG( KVStrList &def, BOOL override )
{
	lc = 0; // longitud de cabecera salvada = 0

	ADD(CAUDIO_NSAMPLES,0);
	ADDIFNOV(CAUDIO_SRATE,def.dval(CAUDIO_SRATE,8000));
	// endian por defecto
	ADDIFNOV(CAUDIO_BIGENDIAN,def.val(CAUDIO_BIGENDIAN,CAUDIO_BIGENDIAN_DEFAULT?"yes":"no"));
	ADDIFNOV(CAUDIO_NCHAN,def.lval(CAUDIO_NCHAN,1));

  fa->info().add(CAUDIO_SAMPTYPE,SAMPTYPE_STR_FLOAT64); // siempre float64
}

/**********************************************************/

BOOL AFHak::testFile( FILE *f )
{
	CHAR id[9];

	memset(id,0,sizeof(id));
	fseek(f,0,SEEK_SET);
	fread(id,8,1,f);
	return (!strcmp(id,_hak_id8));
}

/**********************************************************/
