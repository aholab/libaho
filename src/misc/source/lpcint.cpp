#include <math.h>

#include "xalloc.h"
#include "uti.h"
#include "spl.h"
#include "lpcint.hpp"

/**********************************************************/

LPCInt::LPCInt( INT ord, INT meth, DOUBLE te ) :
  VecInt(ord,(meth!=LPCINT_CONST)?TRUE:FALSE,te)
{
	INT tmplen=0, t;
	INT i;

	method=meth;

	vtmp=vwrk2=NULL;

	switch (method) {
	case LPCINT_CONST:
		tmplen=0;
		break;
	case LPCINT_LIN_KI:
		tmplen=tnel_lp_k2a(ord);
		t=tnel_lp_a2k(ord);
		if (tmplen<t) tmplen=t;
		break;
	case LPCINT_LIN_LSF:
		// LSFs needs some initial value
		for (i=0; i<ord; i++) v2[i]=linterpol(i,0,ord,0.1,M_PI-0.1);
		tmplen=tnel_lp_a2lx(ord);
    t=tnel_lp_lx2a(ord);
    if (tmplen<t) tmplen=t;
    break;
  default:
    die_beep("LPCInt: invalid LPC interpolation method (%d)",method);
  }
	
  if (tmplen) {
    vtmp = (DOUBLE*)xmalloc(sizeof(DOUBLE)*tmplen);
    vwrk2 = (DOUBLE*)xmalloc(sizeof(DOUBLE)*ord);
  }
}

/**********************************************************/

LPCInt::~LPCInt()
{
#define FREE(v)  if (v) xfree(v)
  FREE(vtmp);
  FREE(vwrk2);
}
  
/**********************************************************/

VOID LPCInt::set( DOUBLE *vai, DOUBLE te )
{
  switch (method) {
  case LPCINT_CONST:
    VecInt::set(vai,te);
    break;
  case LPCINT_LIN_KI:
    lp_a2k(vai,vwrk2,len,vtmp);
    VecInt::set(vwrk2,te);
    break;
  case LPCINT_LIN_LSF:
    if (lp_a2lx(vai,vwrk2,len,vtmp)) {
      lp_lx2lw(vwrk2,vwrk2,len);
      VecInt::set(vwrk2,te);
    }
    else VecInt::set(v2,te);
    break;
  default:
    die_beep("LPCInt: invalid LPC interpolation method (%d)",method);
  }
}

/**********************************************************/

DOUBLE *LPCInt::get( DOUBLE pos )
{
  DOUBLE *v;
  switch (method) {
  case LPCINT_CONST: 
    return VecInt::get(pos);
	case LPCINT_LIN_KI:
		v=VecInt::get(pos);
		lp_k2a(v,vwrk2,len,vtmp);
		return vwrk2;
	case LPCINT_LIN_LSF:
		v=VecInt::get(pos);
		lp_lw2lx(v,v,len);
		lp_lx2a(v,vwrk2,len,vtmp);
		return vwrk2;
	default:
    die_beep("LPCInt: invalid LPC interpolation method (%d)",method);
  }
  show_srcpos();
  die_beep("LPCInt: should never arrive here!");
  return NULL;
}

/**********************************************************/

