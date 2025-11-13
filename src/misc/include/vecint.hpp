#ifndef __VECINT_HPP__
#define __VECINT_HPP__

/**********************************************************/

#include "tdef.h"

/**********************************************************/

class VecInt {
public:
  VecInt( INT len, BOOL linint=FALSE, DOUBLE tend=1. );
  ~VecInt();
  
  VOID set( DOUBLE *v, DOUBLE tend=-1 );
  DOUBLE *get( DOUBLE pos );
  
protected:
  INT len;
  BOOL lin;
  DOUBLE tend;
  
  DOUBLE *v1, *v2;
  DOUBLE *vwrk;
};

/**********************************************************/

#endif
