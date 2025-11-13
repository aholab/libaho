#ifndef __CABDEF0_H
#define __CABDEF0_H


/************************************/
/*   1992 (c) Grupo de Voz - DAET   */
/*         ETSII/IT Bilbao          */
/************************************/


/**********************************************************/

#include "tdef.h"

/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(push,1)
#elif __CC_BORLANDC__
typedef struct {CHAR8 x; INT2 y; } ____tmp_cabdef0_align_test___;
#if sizeof(____tmp_cabdef0_align_test___)>3
#error You must disable Word Alignment
#endif
#endif

/**********************************************************/
/* Definici¢n de las estructuras b sicas de las cabeceras */
/**********************************************************/

/* rango de variacion en coma flotante */
struct fminmax {
	   FLOAT4 min MINALIGN;
	   FLOAT4 max MINALIGN;
	   };

/* rango de variacion en enteros de 2 UCHAR1s */
struct iminmax {
	   INT2 min MINALIGN;
	   INT2 max MINALIGN;
	   };

/* definicion de marcas */
struct marcdef {
	   UINT4  unimarc MINALIGN;     /* unidad de las marcas (en muestras) */
	   UINT4  nmarc MINALIGN;       /* n£mero de marcas */
	   pUINT4  marcas MINALIGN;  /* marcas */
	   };

/* definicion de texto */
struct txtdef {
	   UINT2  ltxt MINALIGN;      /* longitud */
		 pCHAR1  txt MINALIGN;   /* texto */
	   };

/* definicion de vectores multi-dimensionales */
struct vecdef {
	   UINT2  dimv MINALIGN;         /* dimension */
	   UINT4  nvec MINALIGN;     /* n£mero de vectores */
	   };

/* definicion de ventana de analisis */
struct ventdef {
	   UINT2  lvent MINALIGN;     /* longitud de la ventana */
	   UINT2  despv MINALIGN;     /* desplazamiento */
	   UCHAR1  tipov MINALIGN;     /* tipo de ventana */
	   };

/* estructura de la primera parte de la cabecera de cualquier fichero */
/* esta estructura existe siempre en todos los ficheros */
struct cfich {
	   UINT4  lcab MINALIGN;      /* longitud en UCHAR11s lcab NO inclu¡da */
	   UINT2  tfich MINALIGN;         /* tipo de fichero (definido en cabcod.h) */
	   UCHAR1  tdato MINALIGN;         /* tipo de dato. Definido en CABCOD */
	   UINT4  mcab MINALIGN;      /* mascara de cabeceras incluidas */
	   };

/* estructura de la cabecera general de se¤al */
struct cgsenal {
	   UINT4  nm MINALIGN;             /* n£mero de muestras */
	   FLOAT4  fm MINALIGN;           /* frecuencia de muestreo en Hz */
	   struct fminmax  rango MINALIGN;    /* de la se¤al */
	   struct txtdef  locu MINALIGN;      /* informante */
	   struct txtdef  texto MINALIGN;     /* de la se¤al */
	   struct txtdef  version MINALIGN;   /* de la se¤al */
	   struct txtdef  obs MINALIGN;       /* observaciones */
	   struct marcdef  marc MINALIGN;     /* marcas */
	   };
/* nota sobre rango: indica los valores que correspondan a los
niveles -1 (rango.min) y +1 (rango.max) de la senal.
Ej. En convertidor de 12 bits entre 0 y 4095, si tenemos
rango.min=0  y rango.max=4096, indica que el -1 esta como 0,
el 1 como 4096 (nunca se da porque solo llega a 4095) y el cero
se codifica como 2048 ((min+max)/2). En cambio, si min=0 y
max=4095, -1-->0, +1-->4095, y el cero no tiene representacion
en el convertidor (0-->2047.5 es un numero REAL) */

/* estructura de la cabecera general de parametros */
/* esta estructura puede existir en todos los ficheros */
/* esta estructura existe siempre en los ficheros de par metros */
/*                      (tfich>=100)                            */
struct cgparam {
	   struct vecdef  vecd MINALIGN;
	   struct ventdef  pventa MINALIGN;
	   };

/**********************************************************/
/* la cabecera completa de un fichero se compone de:

   - cfich                  <--- datos de la cabecera al completo   (siempre)
   - long+ cgsenal + ext    <--- datos de la se¤al original         (op)
   - long+ cgparam + ext    <--- datos de la parametrizaci¢n        (op)
   - long+ cg..... + ext
   - .....

ext significa que podr  haber mas valores, dependientes del tipo del fichero.
long es un UINT4 que indica la longitud de cada una de las subcabeceras sin
incluirse a si mismo.

IMPORTANTE: el alineamiento debe estar a byte (Alignment=Byte) y no a UINT2*/
/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(pop)
#endif

/**********************************************************/

#endif

