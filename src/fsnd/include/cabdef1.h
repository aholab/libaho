#ifndef __CABDEF1_H
#define __CABDEF1_H


/************************************/
/*   1992 (c) Grupo de Voz - DAET   */
/*         ETSII/IT Bilbao          */
/************************************/


/**********************************************************/

#include "cabdef0.h"

/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(push,1)
#elif __CC_BORLANDC__
typedef struct {CHAR8 x; INT2 y; } ____tmp_cabdef1_align_test___;
#if sizeof(____tmp_cabdef1_align_test___)>3
#error You must disable Word Alignment
#endif
#endif

/**********************************************************/
/*cabecera general de parametros de ficheros de espectrogramas */

struct cgp_espect {
       struct cgparam  cgp;
       FLOAT4  preenf;
       };

/**********************************************************/
/*cabecera general de parametros de ficheros lpc */

struct cgp_lpc {
	   struct cgparam  cgp MINALIGN;
	   FLOAT4  preenf MINALIGN;
	   UINT2  orden MINALIGN;
	   UCHAR1  ginc MINALIGN;              /* g incluida */
	   UCHAR1  mlpc MINALIGN;              /* metodo calculo */
	   };

/**********************************************************/
/*cabecera general de parametros de ficheros parcor */

struct cgp_parcor {
	   struct cgparam  cgp MINALIGN;
	   FLOAT4  preenf MINALIGN;
	   UINT2  orden MINALIGN;
	   UCHAR1  ginc MINALIGN;              /* g incluida */
};

/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(pop)
#endif

/**********************************************************/

#endif
