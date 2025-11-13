/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ FVBLK.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... NO
Codigo condicional........... SI (NDEBUG)

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    20/05/97  Borja     eliminar warnings para compilar en g++
1.0.0    19/12/93  Borja     Codificacion inicial.
======================== Contenido ========================
Lectura por bloques (entramado) de un fichero de voz.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <assert.h>

#include "tdef.h"
#include "xalloc.h"
#include "fvoz.h"
#include "fvblk.h"

/**********************************************************/

fvblk * fvblk_construct( pCHAR fname, INT fl,
                         INT prefr, INT posfr )
{
fvblk * fvb;
INT i,n;
assert(fl>0);
assert(prefr>=0);
assert(posfr>=0);

fvb = (fvblk *)xmalloc(sizeof(fvblk));

if (fvb!=NULL) {
  fvb->_fs = fseni_construct(fname);
  fvb->_fl=fl;
  fvb->_prefr=prefr;
  fvb->_posfr=posfr;
  fvb->_blkn=0;
  fvb->_nm=fseni_getnm(fvb->_fs);
  fvb->_nread=0;

  n = fl+prefr+posfr;
  fvb->_vblk = (pDOUBLE)xmalloc(sizeof(DOUBLE)*n);

  if ((fvb->_fs!=NULL)&&(fvb->_vblk!=NULL)) {
    for (i=0; i<n; i++)
      fvb->_vblk[i]=0.0;
    }
  else {
    fvblk_destruct(fvb);
    fvb=NULL;
    }
  }

return fvb;
}

/**********************************************************/

void fvblk_destruct( fvblk * fvb )
{
assert(fvb!=NULL);

if (fvb->_vblk!=NULL)
  xfree(fvb->_vblk);
if (fvb->_fs!=NULL)
  fseni_destruct(fvb->_fs);
xfree(fvb);
}

/**********************************************************/

UINT4 fvblk_getnm( fvblk * fvb )
{
assert(fvb!=NULL);

return fseni_getnm(fvb->_fs);
}

/**********************************************************/

UINT4 fvblk_getfm( fvblk * fvb )
{
assert(fvb!=NULL);

return (UINT4)(fseni_getfm(fvb->_fs)+0.5);
}

/**********************************************************/

UINT4 fvblk_getnblks( fvblk * fvb )
{
assert(fvb!=NULL);

return (fseni_getnm(fvb->_fs)+fvb->_fl-1)/fvb->_fl;
}

/**********************************************************/

BOOL fvblk_forward( fvblk * fvb )
{
INT i, n;
UINT4 nb, onm;
assert(fvb!=NULL);

nb = fvblk_getnblks(fvb);
if (fvb->_blkn<nb) {
  fvb->_blkn++;
  n = fvb->_fl+fvb->_prefr+fvb->_posfr;
  onm = fvb->_nm;

  if (fvb->_blkn==1) {
    for (i=n-fvb->_posfr; i<n; i++) {
      if (fvb->_nm) {
        (fvb->_nm)--;
        fvb->_vblk[i]=fseni_getsamp(fvb->_fs);
        }
      }
    }
  for (i=fvb->_fl; i<n; i++)
    fvb->_vblk[i-fvb->_fl]=fvb->_vblk[i];
  for (i=n-fvb->_fl; i<n; i++) {
    if (fvb->_nm) {
      fvb->_nm--;
      fvb->_vblk[i]=fseni_getsamp(fvb->_fs);
      }
    else
      fvb->_vblk[i]=0.0;
    }
  fvb->_nread=(INT)(onm-fvb->_nm);
  return TRUE;
  }
else
  return FALSE;
}

/**********************************************************/

INT fvblk_getnread( fvblk * fvb )
{
assert(fvb!=NULL);

return fvb->_nread;
}

/**********************************************************/

INT fvblk_getblknsamp( fvblk * fvb )
{
UINT4 n;
assert(fvb!=NULL);

n = fseni_getnm(fvb->_fs);

if ((UINT4)((INT4)(fvb->_blkn)*(INT4)(fvb->_fl))<=n)
  return fvb->_fl;
else
  return (INT)(n%fvb->_fl);
}

/**********************************************************/

INT fvblk_getfl( fvblk * fvb )
{
assert(fvb!=NULL);

return fvb->_fl;
}

/**********************************************************/

pDOUBLE fvblk_getblk( fvblk * fvb )
{
assert(fvb!=NULL);

return fvb->_vblk;
}

/**********************************************************/

UINT4 fvblk_getblknum( fvblk * fvb )
{
assert(fvb!=NULL);

return fvb->_blkn;
}

/**********************************************************/

struct cgsenal * fvblk_getcgs( fvblk * fvb )
{
assert(fvb!=NULL);

return fseni_getcgs(fvb->_fs);
}

/**********************************************************/
