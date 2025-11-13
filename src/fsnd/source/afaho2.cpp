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
#include "cabecer.h"
#include "cabctrl.h"
#include "cabcod.h"

/**********************************************************/
/* definir MRK_SORT para que se salven las marcas ordenadas */

//#define MRK_SORT

/**********************************************************/

LONG AFAho2::HdrR( FILE *f, KVStrList &, BOOL override )
{
	UCHAR8 tdato;
	struct cgsenal cgs;
	LONG nSamp;

	if (!testFile(f))
		fprintf(stderr,"%s warning: probably not a .aho (v.2) file!\n",fFormat());

	xfseek(f,0,SEEK_SET);
	reset_cgsenal(&cgs);
	tdato = leecfs(f,&cgs,-1);

	/* calculamos la longitud de la cabecera de senyal leida del fichero */
	lcgs = xftell(f) - sizeof(struct cfich) - sizeof(UINT32);

	nSamp = cgs.nm;  // numero de muestras

	ADDIFNOV(CAUDIO_SRATE,(DOUBLE)cgs.fm);
  ADDIFNOV(CAUDIO_BIGENDIAN,"no");  // aho es little endian    
	if (cgs.locu.ltxt) ADDIFNOV(INFO_SPEAKER,cgs.locu.txt);
	if (cgs.texto.ltxt) ADDIFNOV(INFO_TEXT, cgs.texto.txt);
	if (cgs.version.ltxt) ADDIFNOV(INFO_VERSION,cgs.version.txt);
	if (cgs.obs.ltxt) ADDIFNOV(INFO_COMMENTS,cgs.obs.txt);

	if ( (cgs.marc.nmarc>0)
		&& (fa->marks().length()==0) || override) { // leemos las marcas?
		fa->marks().clear(); //borrar marcas viejas
		for (LONG i=0; i<(LONG)cgs.marc.nmarc; i++)
			fa->marks().append(cgs.marc.marcas[(size_t)i]*cgs.marc.unimarc);
//		fa->marks().sort();
	}

	free_cgsenal(&cgs);

	switch (tdato) {
	case TD_INT2: ADD(CAUDIO_SAMPTYPE, SAMPTYPE_STR_PCM16); break;
	case TD_FLOAT4: ADD(CAUDIO_SAMPTYPE, SAMPTYPE_STR_FLOAT32); break;
	case TD_FLOAT8: ADD(CAUDIO_SAMPTYPE, SAMPTYPE_STR_FLOAT64); break;
	default: die_beep("%s: data type %u not supported",fFormat(),(unsigned)tdato);
	}

	ADD(CAUDIO_NCHAN,1); // siempre mono

  if (fa->info().bval(CAUDIO_BIGENDIAN))
    fprintf(stderr,"%s warning: reading big endian file!.\n",fFormat());

	return nSamp;
}
/**********************************************************/

#ifdef MRK_SORT
int ui32cmp(const void *a, const void *b)
{
	INT32 i= *(UINT32*)a - *(UINT32*)b;

	if (i>0) return 1;
	if (i<0) return -1;
	return 0;
}
#endif

/**********************************************************/
/* escribe a disco la cabecera */

VOID AFAho2::HdrW( FILE *f, LONG nSamp )
{
#define TXT(prop,field) { \
	cgs.##field##.txt = (char *)fa->info().val(prop,""); \
	cgs.##field##.ltxt = strlen(cgs.##field##.txt); \
}
	UCHAR8 tdato;
	struct cgsenal cgs;
	Lix p;

	cdie_beep(fa->getNChan()!=1,"%s error: multichannel (%ld) not supported!",
		fFormat(),(long)(fa->getNChan()));

	cgs.nm = nSamp;
	cgs.fm = fa->getSRate();

	//TXT(INFO_SPEAKER,locu);
	cgs.locu.txt = (char*)fa->info().val(INFO_SPEAKER,""); 
	cgs.locu.ltxt = strlen(cgs.locu.txt); 
	//TXT(INFO_TEXT,texto);
	cgs.texto.txt = (char*)fa->info().val(INFO_TEXT,""); 
	cgs.texto.ltxt = strlen(cgs.texto.txt); 
	//TXT(INFO_VERSION,version);
	cgs.version.txt = (char*)fa->info().val(INFO_VERSION,""); 
	cgs.version.ltxt = strlen(cgs.version.txt); 
	//TXT(INFO_COMMENTS,obs);
	cgs.obs.txt = (char*)fa->info().val(INFO_COMMENTS,""); 
	cgs.obs.ltxt = strlen(cgs.obs.txt); 

	LONG max=0;
	for (p=fa->marks().first(); p!=0; p=fa->marks().next(p)) {
		LONG x= fa->marks().pos(p);
		if (max<x) max=x;
	}
	cgs.marc.unimarc = (UINT32)(max/32768L+1);
	cgs.marc.nmarc = fa->marks().length();
	cgs.marc.marcas = (pUINT32)xmalloc(sizeof(UINT32)*cgs.marc.nmarc);
	LONG i=0;
	for (p=fa->marks().first(); p!=0; p=fa->marks().next(p))
		cgs.marc.marcas[(size_t)i++] = (fa->marks().pos(p)+cgs.marc.unimarc/2)/cgs.marc.unimarc;
#ifdef MRK_SORT
	qsort(cgs.marc.marcas,(size_t)i,sizeof(UINT32),ui32cmp);
#endif

	switch (fa->getSampType()) {
	case SAMPTYPE_PCM16:
		cgs.rango.min = -32768.;
		cgs.rango.max = +32768.;
		tdato = TD_INT2;
		break;
	case SAMPTYPE_FLOAT32:
		cgs.rango.min = -1.;
		cgs.rango.max = +1.;
		tdato = TD_FLOAT4;
		break;
	case SAMPTYPE_FLOAT64:
		cgs.rango.min = -1.;
		cgs.rango.max = +1.;
		tdato = TD_FLOAT8;
		break;
	default:
		tdato = 0;
		die_beep("%s: sampType %s not supported",fFormat(),fa->getSampType_a());
	}

	LONG nlcgs = long_cgsenal(&cgs);

	// nueva cabecera mayor que la original => inserta hueco
	if (nlcgs>lcgs) xfinsn(f,0,nlcgs-lcgs);
	// nueva menor que original => quita hueco
	else if (nlcgs<lcgs) xfdeln(f,0,lcgs-nlcgs);
	lcgs = nlcgs;  // salva nueva longitud

	escribecfs_ftyp(f,&cgs,TF_SORIG,tdato);
	if (cgs.marc.marcas) xfree(cgs.marc.marcas);

  if (fa->getBigEndian())
    fprintf(stderr,"%s warning: saving big endian file!\n",fFormat());
  
}

/**********************************************************/
/* genera una cabecera nueva */

VOID AFAho2::HdrG( KVStrList &def, BOOL override )
{
	lcgs = 0; // longitud de cabecera salvada = 0

	ADD(CAUDIO_NSAMPLES,0);
	ADDIFNOV(CAUDIO_SRATE,def.dval(CAUDIO_SRATE,8000));
	ADDIFNOV(CAUDIO_SAMPTYPE,def.val(CAUDIO_SAMPTYPE,SAMPTYPE_STR_PCM16));
	ADDIFNOV(CAUDIO_NCHAN,def.lval(CAUDIO_NCHAN,1));

	switch (SampType_a2i(fa->info().val(CAUDIO_SAMPTYPE))) {
	case SAMPTYPE_PCM16:
	case SAMPTYPE_FLOAT32:
	case SAMPTYPE_FLOAT64: break;
	default:
		die_beep("%s: sampType %s not supported",fFormat(),
				(const char *)fa->info().val(CAUDIO_SAMPTYPE));
	}
	cdie_beep(fa->info().lval(CAUDIO_NCHAN)!=1,"%s error: multichannel (%ld) not supported!",
		fFormat(),(long)(fa->info().lval(CAUDIO_NCHAN)));

	ADDIFNOV(CAUDIO_BIGENDIAN,"no");  // aho es little endian
}

/**********************************************************/
// este test no es a prueba de bombas!!!

BOOL AFAho2::testFile( FILE *f )
{
	struct cfich cf;

	memset(&cf,0,sizeof(cf));
	fseek(f,0,SEEK_SET);
	if (fread(&cf,sizeof(cf),1,f)) {
		endian_fromlittle16(&(cf.tfich));
		endian_fromlittle32(&(cf.mcab));
		if ((cf.tfich==TF_SORIG)&&
			((cf.tdato==TD_INT2)||(cf.tdato==TD_FLOAT4)||
			(cf.tdato==TD_FLOAT8))&&(cf.mcab==MC_SENAL))
			return TRUE;
	}
	return FALSE;
}

/**********************************************************/
