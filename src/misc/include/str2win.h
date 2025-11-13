#ifndef __STR2WIN_H__
#define __STR2WIN_H__

/**********************************************************/

#include "tdef.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

DOUBLE *str2win( const CHAR *strwin, DOUBLE *win, INT len );

INT str2xfftwin( const CHAR *strwin );

DOUBLE *xfftwin2win( INT xfftwin, DOUBLE *win, INT len );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif

