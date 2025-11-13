#ifndef __ETA_HPP__
#define __ETA_HPP__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1997 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ ETA.HPP
Nombre paquete............... -
Lenguaje fuente.............. C
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.1    16/05/97  Borja     chrono -> Chrono
1.0.0    20/03/97  Borja     inicial.

======================== Contenido ========================
Gestion de "Tiempo Estimado de LLegada" (ETA).
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"
#include "chrono.hpp"

/**********************************************************/

class ETA {
	private:
		Chrono c;
		DOUBLE nf, ni, rt, ti;
	public:
		ETA( DOUBLE n=100, DOUBLE refreshTimeSec=1 ) { init(n,refreshTimeSec); };
		VOID init( DOUBLE n, DOUBLE refreshTimeSec=1 );
		BOOL getRefresh( VOID );
		DOUBLE getPercent( DOUBLE pos );
		DOUBLE getETASec( DOUBLE pos );
		CHAR *getETATxt( DOUBLE pos );
};

/**********************************************************/

#endif
