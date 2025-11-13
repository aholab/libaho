#include <math.h>

#include "xalloc.h"
#include "uti.h"
#include "vecint.hpp"

/**********************************************************/

VecInt::VecInt( INT n, BOOL linint, DOUBLE te )
{
  len=n;
  lin=linint;
  tend=te?te:1;
  
  v1=v2=vwrk=NULL;
  
  v1 = (DOUBLE*)xmalloc(sizeof(DOUBLE)*len);
  v2 = (DOUBLE*)xmalloc(sizeof(DOUBLE)*len);  
  if (lin) vwrk = (DOUBLE*)xmalloc(sizeof(DOUBLE)*len);  
}

/**********************************************************/

VecInt::~VecInt()
{
#define FREE(v)  if (v) xfree(v)
  FREE(v1);
  FREE(v2);
  FREE(vwrk);
#undef FREE
}
  
/**********************************************************/

VOID VecInt::set( DOUBLE *v, DOUBLE te )
{
  if (te==0) memcpy(v1,v,sizeof(DOUBLE)*len);
  else memcpy(v1,v2,sizeof(DOUBLE)*len);
  memcpy(v2,v,sizeof(DOUBLE)*len);
  if (te>0) tend=te;
}

/**********************************************************/

PRIVATE void interp( DOUBLE *v1, DOUBLE *v2, DOUBLE *out, INT len, DOUBLE pos )
{
  for (INT i=0; i<len; i++) out[i]=v1[i]*(1-pos)+v2[i]*pos;
}

/**********************************************************/

DOUBLE *VecInt::get( DOUBLE pos )
{
  pos /= tend;

  if (pos<=0) return v1;
  if (pos>=1) return v2;
    
  if (lin) {
    interp(v1,v2,vwrk,len,pos);
    return vwrk;
  }

  if (pos<0.5) return v1;
  return v2;
}

/**********************************************************/

