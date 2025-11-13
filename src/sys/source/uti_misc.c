/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ uti_misc.c
Nombre paquete............... uti.h
Lenguaje fuente.............. C (BC31,GCC)
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
2.1.0    22/07/97  Borja     show_srcpos
2.0.1    01/07/97  Borja     fround() pasa a otro modulo
2.0.0    01/05/97  Borja     mix de xfiles, syst, xsyst, fname, vstr...
1.2.2    28/04/97  Borja     bug en fgetlns()
1.2.1    08/04/97  Borja     retoques en documentacion
1.2.0    31/07/96  Borja     die
1.1.0    30/07/96  Borja     ftrunc, fmoven, finsn, fdeln.
1.0.0    11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
Funciones diversas.
Gestion y busqueda de etiquetas en arrays de cadenas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "uti.h"

/**********************************************************/
/* busca {str} en el array de {nstrs} cadenas {strarr}. Si lo encuentra,
{devuelve} el indice en el array. Si no lo encuentra, {devuelve} -1.
Si se encuentra con una etiqueta ambigua, {devuelve} -2.
Codigo para las longitudes, {lcode} =
	 -2 como -1, pero si hay un match exacto se admite aunque sea ambiguo
	 -1 si se permite strlen(strarr{})<strlen(str)
	  0 si strlen(strarr{})==strlen(str)
	  1 si se permite strlen(strarr{})>strlen(str)
	  2 como 1, pero si hay un match exacto se admite aunque sea ambiguo
Si {case_sensitive}==0 no distingue mayusculas de minusculas */

int str_match( const char * const *strarr, int nstrs,
		const char * str, int lcode, int case_sensitive )
{
	int i, mtch, l1l2match, code;
	size_t l, l1, l2;

	mtch = -1;
	l1l2match = -1;
	l=l1=l2=0;

	if (lcode>0)
		l= l1 = strlen(str);
	for ( i=0; i<nstrs; i++ ) {
		if (lcode!=0) {
			l2=strlen(strarr[i]);
			if (lcode<0) l=l2;
			code = ( case_sensitive ? strncmp(str,strarr[i],l) :
					strnicmp(str,strarr[i],l) );
		}
		else
			code = ( case_sensitive ? strcmp(str,strarr[i]) :
					stricmp(str,strarr[i]) );

		if (!code) {
			if ((lcode!=0) && (l1==l2)) l1l2match=i;
			if (mtch!=-1)
				mtch = -2;
			else
				mtch = i;
		}
	}

	if ((mtch==-2)&&(l1l2match!=-1)&&((lcode>1)||(lcode<-1)))
		return l1l2match;

	return(mtch);
}

/**********************************************************/
/* Como str_match(), pero si no encuentra {str} en {strarr} o
hay ambiguedad, imprime error y termina el programa. */

int xstr_match( const char * const *strarr, int nstrs, const char * str,
				int lcode, int case_sensitive )
{
	int i;
	i = str_match(strarr,nstrs,str,lcode,case_sensitive);
	if (i<0) {
		cdie_beep(i==-2,"error: ambiguous label (%s)",str);
		die_beep("error: unknown label (%s)",str);
	}
	return(i);
}

/**********************************************************/
/* busca {label} en el array de etiquetas {labelarr} de {nlabels}
elementos.
{devuelve} un indice a la etiqueta que encaje dentro del array.
Si no encaja ninguna {devuelve} -1, y si hay ambiguedad -2.
Si {allow_partial} es distinto de cero, {label} puede ser mas corta que
las etiquetas de {labelarr} (abreviada).
Ademas se elimina el '=valor' de {label} (label=valor) para comparar.
Si {case_sensitive}==0 no distingue mayusculas de minusculas */

int label_match( const char * const *labelarr, int nlabels,
				 const char * label, int case_sensitive,
				 int allow_partial )
{
	int i;
	char c, *cp;

	c = (*(cp=str_assignptr(label)));
	*cp = 0;
	i = str_match(labelarr,nlabels,label,(allow_partial?2:0),case_sensitive);
	*cp = c;

	return(i);
}

/**********************************************************/
/* Como label_match(), pero si no encuentra {label} en {labelarr} o
hay ambiguedad, imprime error y termina el programa. */

int xlabel_match( const char * const *labelarr, int nlabels,
					const char * label, int case_sensitive, int allow_partial )
{
	int i;
	i = label_match(labelarr,nlabels,label,case_sensitive,allow_partial);
	if (i<0) {
		cdie_beep(i==-2,"error: ambiguous label (%s)",label);
		die_beep("error: unknown label (%s)",label);
	}
	return(i);
}

/**********************************************************/
/* Similar a xlabel_match(), pero si str=="" {devuelve} {deval} */

int str2k( const char * str, const char * const *labels,
		int defval, int case_sensitive, int allow_partial )
{
	if ((strlen(str)==0)&&(defval>=0))
		return(defval);
	return xlabel_match( labels,nptrs((const void * const *)labels),str,
			case_sensitive,allow_partial);
}

/**********************************************************/
/* {devuelve} el numero de punteros de un array de punteros, terminado
con puntero a NULL. Si {ptrarray}==NULL {devuelve} 0 */

int nptrs( const void * const *ptrarray )
{
	int i = 0;

	if (ptrarray!=NULL) {
		while (ptrarray[i]!=NULL)
			i++;
	}

	return(i);
}

/**********************************************************/
/* imprime nombre fichero y posicion. Usado para mensajes
de error, utilizando normalmente los macros del preprocesador 
__FILE__ y __LINE__ (usar show_srcpos()). */

void __show_srcpos( const char *file, int line )
{
	fprintf(stderr,"[%s:%d] ",file,line);
}

/**********************************************************/
