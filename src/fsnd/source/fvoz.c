/************************************/
/*   1992 (c) Grupo de Voz - DAET   */
/*         ETSII/IT Bilbao          */
/************************************/


/**********************************************************/

#include <stdio.h>

#include "tdef.h"
#include "uti.h"
#include "xalloc.h"
#include "cabdef0.h"
#include "cabdef1.h"
#include "cabctrl.h"
#include "cabecer.h"
#include "fvoz.h"
#include "spl.h"

/**********************************************************/

fseni * fseni_construct( pCHAR fname )
{
fseni * fs;

fs = (fseni *)xmalloc(sizeof(fseni));

fs->f = xfopen(fname,"rb");
fs->tdata = leecfs(fs->f,&fs->cgs,-1);
fs->min = fs->cgs.rango.min;
fs->max = fs->cgs.rango.max;

fs->cgs.rango.min = -1;
fs->cgs.rango.max = 1;

return fs;
}

/**********************************************************/

fseni * fseni_v_construct( pCHAR fname )
{
fseni * fs;

fs = (fseni *)xmalloc(sizeof(fseni));

fs->f = xfopen(fname,"rb");
vleecfs(fs->f,&(fs->cgs));
fs->tdata = TD_INT2;
fs->min = fs->cgs.rango.min;
fs->max = fs->cgs.rango.max;

fs->cgs.rango.min = -1;
fs->cgs.rango.max = 1;

return fs;
}


/**********************************************************/

void fseni_destruct( fseni * fs )
{
free_cgsenal(&fs->cgs);
xfclose(fs->f);
xfree(fs);
}

/**********************************************************/

struct cgsenal * fseni_getcgs( fseni * fs )
{
return &fs->cgs;
}

/**********************************************************/

UINT4 fseni_getnm( fseni * fs )
{
return fs->cgs.nm;
}

/**********************************************************/

FLOAT4 fseni_getfm( fseni * fs )
{
return fs->cgs.fm;
}

/**********************************************************/

FLOAT4 fseni_getsamp( fseni * fs )
{
INT16 samp;

switch (fs->tdata) {
  case TD_INT2 :
    xfread(&samp,sizeof(samp),1,fs->f);
    return (FLOAT4)normalize(samp,fs->min,fs->max);
  case TD_FLOAT4 :
    return (FLOAT4)normalize(xlee_sfloat(fs->f),fs->min,fs->max);
  default :
	die_beep("error: tipo de datos no soportado");
	break;
  }
return 0;
}

/**********************************************************/

INT16 fseni_geti16samp( fseni * fs )
{
INT16 samp;

switch (fs->tdata) {
  case TD_INT2 :
    xfread(&samp,sizeof(samp),1,fs->f);
    return samp;
	case TD_FLOAT4 :
		return sunnorm_i16(normalize(xlee_sfloat(fs->f),fs->min,fs->max));
  default :
	die_beep("error: tipo de datos no soportado");
	break;
  }
return 0;
}

/**********************************************************/

fseno * fseno_construct( pCHAR fname,
                         struct cgsenal * cgs,
                         UINT2 ftyp,
                         UCHAR1 tdata,
                         pCHAR tmpname )
{
fseno * fs;
struct fminmax rango;

fs = (fseno *)xmalloc(sizeof(fseno));
fs->fn = fntmp_construct(fname,tmpname);

fs->f = xfopen(fntmp_getname(fs->fn),"wb+");
rango = cgs->rango;
if (tdata==TD_INT2) {
	cgs->rango.min = -INT16_RANGE;
	cgs->rango.max = +INT16_RANGE;
  }
else {
  cgs->rango.min = -1;
  cgs->rango.max = +1;
  }

escribecfs_ftyp(fs->f,cgs,ftyp,tdata);
fs->tdata = tdata;
cgs->rango = rango;

fs->nm = 0;

return fs;
}

/**********************************************************/

void fseno_destruct( fseno * fs )
{
UINT4 n;

n = fs->nm;
vete_cgsenal(fs->f);
xfwrite(&n,sizeof(n),1,fs->f);
xfclose(fs->f);
fntmp_destruct(fs->fn);
xfree(fs);
}

/**********************************************************/

UINT4 fseno_getnm( fseno * fs )
{
return fs->nm;
}

/**********************************************************/

void fseno_setsamp( fseno * fs,
                    FLOAT4 samp )
{
INT16 isamp;

switch (fs->tdata) {
  case TD_INT2 :
		isamp = sunnorm_i16(samp);
    xfwrite(&isamp,sizeof(isamp),1,fs->f);
    break;
  case TD_FLOAT4 :
    xescribe_sfloat(fs->f,samp);
    break;
  default :
	die_beep("error: tipo de datos no soportado");
	break;
  }
(fs->nm)++;
}

/**********************************************************/

void fseno_seti16samp( fseno * fs,
											INT16 samp )
{
switch (fs->tdata) {
  case TD_INT2 :
    xfwrite(&samp,sizeof(samp),1,fs->f);
    break;
  case TD_FLOAT4 :
		xescribe_sfloat(fs->f,(FLOAT4)norm_i16(samp));
    break;
  default :
	die_beep("error: tipo de datos no soportado");
	break;
  }
(fs->nm)++;
}

/**********************************************************/

fslpco * fslpco_construct( pCHAR fname,
                           struct cgsenal * cgs,
                           struct cgp_lpc * cgpl,
                           UINT2 ftyp,
                           UCHAR1 tdata,
                           pCHAR tmpname )
{
fslpco * fs;
struct fminmax rango;

fs = (fslpco *)xmalloc(sizeof(fslpco));
fs->fn = fntmp_construct(fname,tmpname);

fs->f = xfopen(fntmp_getname(fs->fn),"wb+");
rango = cgs->rango;

if (tdata==TD_INT2) {
	cgs->rango.min = -INT16_RANGE;
  cgs->rango.max = +INT16_RANGE;
  }
else {
  cgs->rango.min = -1;
  cgs->rango.max = +1;
  }

escribecfslpc(fs->f,cgs,cgpl,ftyp,tdata);
fs->tdata = tdata;
cgs->rango = rango;

fs->nm = 0;

return fs;
}

/**********************************************************/

void fslpco_destruct( fslpco * fs )
{
UINT4 n;

n = fs->nm;
vete_cgsenal(fs->f);
xfwrite(&n,sizeof(n),1,fs->f);
xfclose(fs->f);
fntmp_destruct(fs->fn);
xfree(fs);
}

/**********************************************************/

UINT4 fslpco_getnm( fslpco * fs )
{
return fs->nm;
}

/**********************************************************/

void fslpco_setsamp( fslpco * fs,
                     FLOAT4 samp )
{
INT16 isamp;

switch (fs->tdata) {
  case TD_INT2 :
		isamp = sunnorm_i16(samp);
    xfwrite(&isamp,sizeof(isamp),1,fs->f);
    break;
  case TD_FLOAT4 :
    xescribe_sfloat(fs->f,samp);
    break;
  default :
	die_beep("error: tipo de datos no soportado");
	break;
  }
(fs->nm)++;
}

/**********************************************************/

fslpci * fslpci_construct( pCHAR fname )
{
fslpci * fs;

fs = (fslpci *)xmalloc(sizeof(fslpci));

fs->f = xfopen(fname,"rb");
fs->tdata = (UCHAR1)leecfslpc(fs->f,&fs->cgs,&fs->cgpl,-1,&fs->haycgpl);
fs->min = fs->cgs.rango.min;
fs->max = fs->cgs.rango.max;

fs->cgs.rango.min = -1;
fs->cgs.rango.max = 1;

return fs;
}

/**********************************************************/

void fslpci_destruct( fslpci * fs )
{
free_cgsenal(&fs->cgs);
xfclose(fs->f);
xfree(fs);
}

/**********************************************************/

struct cgsenal * fslpci_getcgs( fslpci * fs )
{
return &fs->cgs;
}

/**********************************************************/

BOOL fslpci_haycgpl( fslpci * fs )
{
return fs->haycgpl;
}

/**********************************************************/

struct cgp_lpc * fslpci_getcgpl( fslpci * fs )
{
return &fs->cgpl;
}

/**********************************************************/

UINT4 fslpci_getnm( fslpci * fs )
{
return fs->cgs.nm;
}

/**********************************************************/

FLOAT4 fslpci_getfm( fslpci * fs )
{
return fs->cgs.fm;
}

/**********************************************************/

FLOAT4 fslpci_getsamp( fslpci * fs )
{
INT16 samp;

switch (fs->tdata) {
  case TD_INT2 :
    xfread(&samp,sizeof(samp),1,fs->f);
    return (FLOAT4)normalize(samp,fs->min,fs->max);
  case TD_FLOAT4 :
    return (FLOAT4)normalize(xlee_sfloat(fs->f),fs->min,fs->max);
  default :
	die_beep("error: tipo de datos no soportado");
	break;
  }
return 0;
}

/**********************************************************/

flpci * flpci_construct( pCHAR fname )
{
flpci * fl;

fl = (flpci *)xmalloc(sizeof(flpci));

fl->f = xfopen(fname,"rb");
lee_cab_lpc(fl->f,&fl->cgpl);
fl->lpcvec = (pFLOAT4)xmalloc(sizeof(FLOAT4)*flpci_getdimvlpc(fl));

return fl;
}

/**********************************************************/

void flpci_destruct( flpci * fl )
{
xfclose(fl->f);
xfree(fl->lpcvec);
xfree(fl);
}

/**********************************************************/

struct cgp_lpc * flpci_getcgp_lpc( flpci * fl )
{
return &(fl->cgpl);
}

/**********************************************************/

UINT4 flpci_getnvec( flpci * fl )
{
return (fl->cgpl.cgp.vecd.nvec);
}

/**********************************************************/

UINT2 flpci_getdimvlpc( flpci * fl )
{
return (fl->cgpl.cgp.vecd.dimv) - ( (fl->cgpl.ginc) ? 1 : 0 );
}

/**********************************************************/

BOOL flpci_getginc( flpci * fl )
{
return fl->cgpl.ginc;
}

/**********************************************************/

pFLOAT4 flpci_getvlpc( flpci * fl,
                       FLOAT4 * G )
{
UINT2 i, l;

if (flpci_getginc(fl))
  *G = xlee_sfloat(fl->f);
else
  *G = 1;

l = flpci_getdimvlpc(fl);
for (i=0; i<l; i++)
  fl->lpcvec[i]=xlee_sfloat(fl->f);
return fl->lpcvec;
}

/**********************************************************/

flpco * flpco_construct( pCHAR fname,
                         struct cgsenal * cgs,
                         struct cgp_lpc * cgpl,
                         pCHAR tmpname )
{
flpco * fl;

fl = (flpco *)xmalloc(sizeof(flpco));
fl->fn = fntmp_construct(fname,tmpname);

fl->f = xfopen(fntmp_getname(fl->fn),"wb+");
escribe_cab_lpc(fl->f,cgs,cgpl);

fl->nv = 0;
fl->incG = cgpl->ginc;
fl->dimv = (cgpl->cgp.vecd.dimv) - ( (fl->incG) ? 1 : 0 );

return fl;
}

/**********************************************************/

void flpco_destruct( flpco * fl )
{
UINT4 n;

n = fl->nv;
vete_cgparam(fl->f);
xfseek(fl->f,sizeof(UINT2),SEEK_CUR);
xfwrite(&n,sizeof(n),1,fl->f);
xfclose(fl->f);
fntmp_destruct(fl->fn);
xfree(fl);
}

/**********************************************************/

UINT4 flpco_getnvec( flpco * fl )
{
return fl->nv;
}

/**********************************************************/

UINT2 flpco_getdimvlpc( flpco * fl )
{
return fl->dimv;
}

/**********************************************************/

BOOL flpco_getginc( flpco * fl )
{
return fl->incG;
}

/**********************************************************/

void flpco_setvlpc( flpco * fl,
                    pFLOAT4 lpcvec,
                    FLOAT4 G )
{
INT16 i;

if (fl->incG)
  xescribe_sfloat(fl->f,G);    /* salva G */
for (i=0; i<fl->dimv; i++)
  xescribe_sfloat(fl->f,lpcvec[i]);

(fl->nv)++;
}

/**********************************************************/

fparo * fparo_construct( pCHAR fname,
                         struct cgsenal * cgs,
                         struct cgp_parcor * cgpp,
                         pCHAR tmpname )
{
fparo * fp;

fp = (fparo *)xmalloc(sizeof(fparo));
fp->fn = fntmp_construct(fname,tmpname);

fp->f = xfopen(fntmp_getname(fp->fn),"wb+");
escribe_cab_parcor(fp->f,cgs,cgpp);

fp->nv = 0;
fp->incG = cgpp->ginc;
fp->dimv = (cgpp->cgp.vecd.dimv) - ( (fp->incG) ? 1 : 0 );

return fp;
}

/**********************************************************/

void fparo_destruct( fparo * fp )
{
UINT4 n;

n = fp->nv;
vete_cgparam(fp->f);
xfseek(fp->f,sizeof(UINT2),SEEK_CUR);
xfwrite(&n,sizeof(n),1,fp->f);
xfclose(fp->f);
fntmp_destruct(fp->fn);
xfree(fp);
}

/**********************************************************/

UINT4 fparo_getnvec( fparo * fp )
{
return fp->nv;
}

/**********************************************************/

UINT2 fparo_getdimvpar( fparo * fp )
{
return fp->dimv;
}

/**********************************************************/

BOOL fparo_getginc( fparo * fp )
{
return fp->incG;
}

/**********************************************************/

void fparo_setvpar( fparo * fp,
                    pFLOAT4 parvec,
                    FLOAT4 G )
{
INT i;

if (fp->incG)
  xescribe_sfloat(fp->f,G);    /* salva G */

for (i=0; i<fp->dimv; i++)
  xescribe_sfloat(fp->f,parvec[i]);

(fp->nv)++;
}

/**********************************************************/
