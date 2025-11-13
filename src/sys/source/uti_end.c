/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1997 - CSTR University of Edinburg

Nombre fuente................ UTI_END.C
Nombre paquete............... UTI.H
Lenguaje fuente.............. C (BC31,GCC)
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.0    22/07/97  Borja     initial

======================== Contenido ========================
Big endian / little endian conversion
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "uti.h"

/**********************************************************/

VOID endian_swap16( VOID * word16 )
{
#define P ((BYTE*)word16)
   BYTE tmp;
   tmp =P[0];
   P[0]=P[1];
   P[1]=tmp;
#undef P
}

/**********************************************************/

VOID endian_swap32( VOID * word32 )
{
#define P ((BYTE*)word32)
   BYTE tmp;
   tmp =P[0];
   P[0]=P[3];
   P[3]=tmp;
   tmp =P[1];
   P[1]=P[2];
   P[2]=tmp;
#undef P
}

/**********************************************************/

VOID endian_swap64( VOID * word64 )
{
#define P ((BYTE*)word64)
   BYTE tmp;
   tmp =P[0];
   P[0]=P[7];
   P[7]=tmp;
   tmp =P[1];
   P[1]=P[6];
   P[6]=tmp;
   tmp =P[2];
   P[2]=P[5];
   P[5]=tmp;
	 tmp =P[3];
   P[3]=P[4];
   P[4]=tmp;
#undef P
}

/**********************************************************/

VOID endian_swap16_n( VOID * word16, int n )
{
	 BYTE tmp;
	 BYTE *p=(BYTE*)word16;

	 while (n) {
			n--;
			tmp =p[0];
			p[0]=p[1];
			p[1]=tmp;
			p+=2;
	 }
}

/**********************************************************/

VOID endian_swap32_n( VOID * word32, int n )
{
	 BYTE tmp;
	 BYTE *p=(BYTE*)word32;

	 while (n) {
			n--;
			tmp =p[0];
			p[0]=p[3];
			p[3]=tmp;
			tmp =p[1];
			p[1]=p[2];
			p[2]=tmp;
			p+=4;
	 }
}

/**********************************************************/

VOID endian_swap64_n( VOID * word64, int n )
{
	 BYTE tmp;
	 BYTE *p=(BYTE*)word64;

	 while (n) {
			n--;
			tmp =p[0];
			p[0]=p[7];
			p[7]=tmp;
			tmp =p[1];
			p[1]=p[6];
			p[6]=tmp;
			tmp =p[2];
			p[2]=p[5];
			p[5]=tmp;
			tmp =p[3];
			p[3]=p[4];
			p[4]=tmp;
			p+=8;
   }
}

/**********************************************************/


