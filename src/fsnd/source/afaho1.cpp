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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "caudiox.hpp"

#include "xalloc.h"
#include "uti.h"
#include "cabecer.h"
#include "cabctrl.h"

/**********************************************************/
/* Lee de disco una cabecera (.aho-1.0) y crea todos los campos
de informacion etc etc */

LONG AFAho1::HdrR( FILE *f, KVStrList &, BOOL override )
{
	LONG nSamp;
	struct cgsenal cgs;

	xfseek(f,0,SEEK_SET);
	reset_cgsenal(&cgs);
	vleecfs(f,&cgs);

	nSamp = (LONG)cgs.nm;  // numero muestras

	ADDIFNOV(CAUDIO_SRATE,(DOUBLE)cgs.fm);
  ADDIFNOV(CAUDIO_BIGENDIAN,"no");  // aho1 es little endian      
	ADD(CAUDIO_SAMPTYPE, SAMPTYPE_STR_PCM16);  // fuerza samptype
	ADD(CAUDIO_NCHAN, 1);  // fuerza nchan

	if (cgs.locu.ltxt) ADDIFNOV(INFO_SPEAKER,cgs.locu.txt);
	if (cgs.texto.ltxt) ADDIFNOV(INFO_TEXT, cgs.texto.txt);
	if (cgs.version.ltxt) ADDIFNOV(INFO_VERSION,cgs.version.txt);
	if (cgs.obs.ltxt) ADDIFNOV(INFO_COMMENTS,cgs.obs.txt);

	if ( (cgs.marc.nmarc>0)
		&& (fa->marks().length()==0) || override) { // leemos las marcas?
		fa->marks().clear(); //borrar marcas viejas
		for (LONG i=0; i<(LONG)cgs.marc.nmarc; i++)
			fa->marks().append(cgs.marc.marcas[(size_t)i]*cgs.marc.unimarc);
	}

	free_cgsenal(&cgs);

  if (fa->info().bval(CAUDIO_BIGENDIAN))
    fprintf(stderr,"%s warning: reading big endian file!.\n",fFormat());

	return nSamp;
}

/**********************************************************/
/* genera una cabecera nueva */

VOID AFAho1::HdrG( KVStrList &def, BOOL override )
{
	ADD(CAUDIO_NSAMPLES,0);
	ADDIFNOV(CAUDIO_SRATE,def.dval(CAUDIO_SRATE,8000));
	ADDIFNOV(CAUDIO_SAMPTYPE,def.val(CAUDIO_SAMPTYPE,SAMPTYPE_STR_PCM16));
	ADDIFNOV(CAUDIO_NCHAN,def.dval(CAUDIO_NCHAN,1));

	cdie_beep(SampType_a2i(fa->info().val(CAUDIO_SAMPTYPE))!=SAMPTYPE_PCM16,
			"%s: sampType %s not supported",fFormat(),
			(const char *)fa->info().val(CAUDIO_SAMPTYPE));
	cdie_beep(fa->info().lval(CAUDIO_NCHAN)!=1,
			"%s: multichannel (%ld) not supported",fFormat(),
			(long)(fa->info().lval(CAUDIO_NCHAN)));

	ADDIFNOV(CAUDIO_BIGENDIAN,"no");  // aho1 es little endian
}

/**********************************************************/
/* escribe a disco la cabecera */

VOID AFAho1::HdrW( FILE *f, LONG nSamp )
{
#define TXT(prop,field)  { \
	cgs.##field##.txt = (char*)fa->info().val(prop,""); \
	cgs.##field##.ltxt = strlen(cgs.##field##.txt); \
}

	struct cgsenal cgs;
	Lix p;

	cdie_beep(fa->getSampType()!=SAMPTYPE_PCM16,
			"%s: sampType %s not supported",fFormat(),fa->getSampType_a());
	cdie_beep(fa->getNChan()!=1,
			"%s: multichannel (%ld) not supported",fFormat(),
			(long)(fa->getNChan()));

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
	vescribecfs(f,&cgs);

	if (cgs.marc.marcas) xfree(cgs.marc.marcas);
  
  if (fa->getBigEndian())
    fprintf(stderr,"%s warning: saving big endian file!\n",fFormat());
 
}

/**********************************************************/

BOOL AFAho1::testFile( FILE * )
{
	return FALSE;
}

/**********************************************************/
