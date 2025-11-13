#ifndef __LPCINT_HPP__
#define __LPCINT_HPP__
/**********************************************************/

#include "tdef.h"
#include "vecint.hpp"

/**********************************************************/

typedef enum {
  LPCINT_CONST = 0,
  LPCINT_LIN_KI,
  LPCINT_LIN_LSF
};

/**********************************************************/

class LPCInt : public VecInt {
public:
  LPCInt( INT order, INT method=LPCINT_CONST, DOUBLE tend=1. );
  ~LPCInt();
  
  VOID set( DOUBLE *vai, DOUBLE tend=-1 );
  DOUBLE *get( DOUBLE pos );
  
private:
  INT method;
  
  DOUBLE *vtmp, *vwrk2;
};

/**********************************************************/

#endif
