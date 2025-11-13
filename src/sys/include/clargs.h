#ifndef __CLARGS_H
#define __CLARGS_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ CLARGS.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... NO
Codigo condicional........... NO

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.1.0    20/05/97  Borja     arg {allow_unknown} en clasw_getsw()
1.0.0    11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera de CLARGS.C

Gestion de argumentos de linea.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

#define CLARGS_MAX_LEN 256    /* Maxima longitud de un argumento
                                  (incluido el 0 final) */

#define CLARGS_CFGFILE  "CfgFile"  /* switch de fichero de configuracion */
#define CLARGS_ARGFILE  "ArgFile"  /* switch de fichero de argumentos */

/**********************************************************/
/* 1 para mayusculas!=minusculas.
0 no distingue  (0 es el valor por defecto) */

extern int _clargs_case_sensitive;

/**********************************************************/

typedef struct {
    char args[CLARGS_MAX_LEN];  /* buffer para meter los argumentos */
    int argc;  /* argc-1 de main() */
    char * * argv;  /* argv+1[] de main() */

    int lastarg;  /* ultimo argumento de linea procesado */

    char * cfgfn;  /* nombre del fichero de configuracion a utilizar */
    char * exefn;  /* _cfgfn, pero en el directorio del .exe, o NULL
                       si _cfgfn no es el indicado por defecto */

    FILE * cfgf;  /* fichero de configuracion */
    FILE * argf;  /* fichero de argumentos */
    } clargs;

typedef struct {
  clargs * c;
	const char * * switches;  /* array de switches validos para getsw() */
  int nswitches;  /* numero de switches */
  } clasw;

/**********************************************************/

clargs * clargs_construct( int argc, char * argv[], const char * def_cfgfn );
void clargs_destruct( clargs * c );
void clargs_restart( clargs * c );
char * clargs_getarg( clargs * c );
char * clargs_getsarg( clargs * c );
char * clargs_getparg( clargs * c );

/**********************************************************/

clasw * clasw_construct( int argc, char * argv[], const char * sw[],
													const char * def_cfgfn );
void clasw_destruct( clasw * c );
int clasw_getsw( clasw * c, char  ** value, int allow_unknown );
void clasw_restart( clasw * c );
char * clasw_getarg( clasw * c );
char * clasw_getsarg( clasw * c );
char * clasw_getparg( clasw * c );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
