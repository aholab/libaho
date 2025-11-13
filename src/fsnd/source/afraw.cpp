/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ -
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    09/04/99  Borja     uso tipos UINT
0.0.0    30/07/96  Borja     codificacion inicial.

======================== Contenido ========================
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "uti.h"
#include "caudiox.hpp"

/**********************************************************/
/* Lee la cabecera. La posicion en el fichero es indeterminada,
al terminar de leer la cabecera debe dejarse el fichero en la
posicion de la primera muestra. {devuelve} el n. de muestras */

LONG AFRaw::HdrR( FILE* f, KVStrList &def, BOOL override )
{
	/* el tipo de muestra y la frecuencia de muestreo, se dejan
	por defecto (previamente definidas, o variables de entorno, o
	valor por defecto compilado). */
	ADDIFNOV(CAUDIO_SRATE,def.dval(CAUDIO_SRATE,8000));
	ADDIFNOV(CAUDIO_SAMPTYPE,def.val(CAUDIO_SAMPTYPE,SAMPTYPE_STR_PCM16));
	ADDIFNOV(CAUDIO_NCHAN,def.lval(CAUDIO_NCHAN,1));
	// endian por defecto
	ADDIFNOV(CAUDIO_BIGENDIAN,def.val(CAUDIO_BIGENDIAN,CAUDIO_BIGENDIAN_DEFAULT?"yes":"no"));

	xfseek(f,0,SEEK_SET);  // posiciona en la primera muestra
	return -1;  // auto
}

/**********************************************************/
/* Se inventa y rellena los campos que quiera de una cabecera
por defecto para un fichero nuevo. */

VOID AFRaw::HdrG( KVStrList &def, BOOL override )
{
	ADD(CAUDIO_NSAMPLES,0);
	ADDIFNOV(CAUDIO_SRATE,def.dval(CAUDIO_SRATE,8000));
	ADDIFNOV(CAUDIO_SAMPTYPE,def.val(CAUDIO_SAMPTYPE,SAMPTYPE_STR_PCM16));
	ADDIFNOV(CAUDIO_NCHAN,def.lval(CAUDIO_NCHAN,1));

  // endian por defecto
	ADDIFNOV(CAUDIO_BIGENDIAN,def.val(CAUDIO_BIGENDIAN,CAUDIO_BIGENDIAN_DEFAULT?"yes":"no"));
}

/**********************************************************/
/* Escribe la cabecera. La posicion inicial en el fichero esta
indefinida.
Una vez escrita la cabecera, debe ir a la primera muestra
de audio. */

VOID AFRaw::HdrW( FILE* f, LONG )
{
	xfseek(f,0,SEEK_SET);  // vamos a la primera muestra
}

/**********************************************************/

BOOL AFRaw::testFile( FILE * )
{
	return FALSE;  // un raw no es detectable
}

/**********************************************************/
