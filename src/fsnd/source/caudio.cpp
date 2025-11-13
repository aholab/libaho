/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ caudio.cpp
Nombre paquete............... caudio
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    09/04/99  Borja     uso tipos UINT
0.0.0    01/07/97  Borja     codificacion inicial.

======================== Contenido ========================
Funcioncillas sobre objetos CAudio en general
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>
#include "caudiox.hpp"

/**********************************************************/

VOID CAudio::minmax( LONG len, DOUBLE *min, DOUBLE *max )
{
	DOUBLE d, mi, ma;
	LONG i;

	mi=ma=0;

	if (len>0) {
		get(d);
		ma=mi=d;
		for (i=1; i<len; i++) {
			get(d);
			if (d>ma) ma=d;
			if (d<mi) mi=d;
		}
	}
	if (min) *min=mi;
	if (max) *max=ma;
}

/**********************************************************/

VOID CAudio::minmax( LONG len, INT16 *min, INT16 *max )
{
	INT16 d, mi, ma;
	LONG i;

	mi=ma=0;

	if (len>0) {
		get(d);
		ma=mi=d;
		for (i=1; i<len; i++) {
			get(d);
			if (d>ma) ma=d;
			if (d<mi) mi=d;
		}
	}
	if (min) *min=mi;
	if (max) *max=ma;
}

/**********************************************************/

LONG CAudio::maxfr( UINT framelen, UINT winlen, LONG nframes )
{
	LONG pos0 = getPos();
	INT ofs;
	LONG max, posmax;

	if (nframes<0) {
		LONG len=getNSamples()-pos0;
		nframes=(LONG)((len+framelen-1)/framelen);
	}

	if (framelen<=winlen)
		ofs = -((INT)(winlen-framelen+1)/2);
	else
		ofs = ((INT)(framelen-winlen)/2);

	max=0;
	posmax=0;
	for ( LONG fr=0; fr<nframes; fr++) {
		setPos(pos0+fr*framelen+ofs);
		INT16 *v = get_i16(winlen);
		LONG sum=0;
		for (UINT i=0; i<winlen; i++)
			sum += abs(v[i]);
		if (sum>max) {
			max=sum;
			posmax = fr;
		}
	}
	return posmax;
}


/**********************************************************/
