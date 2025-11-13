/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL8B.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.0    01/07/97  Borja     spl8.c -> spl8?.c

======================== Contenido ========================
Funciones varias relacionadas con la DFT
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/**********************************************************/

#define DEG60  (60*M_PI/180)
#define DEG120  (120*M_PI/180)

/**********************************************************/
/* uso interno */

PRIVATE VOID specpal_basic( SPL_FLOAT H, SPL_FLOAT S,
	SPL_FLOAT *R, SPL_FLOAT *G, SPL_FLOAT *B )
{
	H = H*M_PI/180.;
	*B = 1-S;
	*R = (1 + cos(H)/cos(DEG60-H))/3;
	*G = (1 + cos(DEG120-H)/cos(-DEG60+H))/3;
	*R += (1-S)*(1-*R);
	*G += (1-S)*(1-*G);
}

/**********************************************************/
/* uso interno */

PRIVATE VOID specpal_hsi2rgb( SPL_FLOAT H, SPL_FLOAT S, SPL_FLOAT I,
	SPL_FLOAT *r, SPL_FLOAT *g, SPL_FLOAT *b )
{
	SPL_FLOAT R, G, B;
	SPL_FLOAT II;

	if (H<0) H+=360;

	if ((H>0)&&(H<=120)) specpal_basic(H,S,&R,&G,&B);
	else if ((H>120)&&(H<=240)) specpal_basic(H-120,S,&G,&B,&R);
	else specpal_basic(H-240,S,&B,&R,&G);

	II=R;
	if (G>II) II=G;
	if (B>II) II=B;

	R*=I/II;
	G*=I/II;
	B*=I/II;

	if (R>1) R=1;
	if (G>1) G=1;
	if (B>1) B=1;

	if (R<0) R=0;
	if (G<0) G=0;
	if (B<0) B=0;

	*r=R;
	*g=G;
	*b=B;
}

/**********************************************************/
/* genera codigos de colores para paleta espectral.
Si queremos una paleta espectral (negro, azul, rojo, amarillo,
blanco) con un total de {ncolors} colores, en {r} {g} {b} nos
devuelve las componentes RGB normalizadas (valores float
entre 0 y 1) que debemos utilizar para el color numero {colornum}
de la paleta ( 0 <= {colornum} < {ncolors} */

PUBLIC VOID XAPI spectral_color( SPL_INT colornum,
	SPL_FLOAT ncolors, SPL_FLOAT *r, SPL_FLOAT *g, SPL_FLOAT *b )
{
#define ZLEVEL 0.25
	SPL_FLOAT h, s, i;

	if (colornum<=0) { *r = *g = *b = 0; return; }
	if (colornum>=ncolors-1) { *r = *g = *b = 1; return; }

	colornum--;
	ncolors-=3;

	if (!ncolors) { *r = 1; *g = *b = 0.2; return; }

	h=(180.*colornum)/ncolors-120;
	s=1;
	i=ZLEVEL+((1-ZLEVEL)*colornum)/ncolors;

	specpal_hsi2rgb(h,s,i,r,g,b);
}

/**********************************************************/
