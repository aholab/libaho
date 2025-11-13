#ifndef __FBLOCK_HPP__
#define __FBLOCK_HPP__

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
0.0.0    15/07/96  Borja     codificacion inicial.

======================== Contenido ========================
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#ifndef __cplusplus
#error Must use C++ compiler
#endif

/**********************************************************/

#include <stdio.h>
#include "tdef.h"

/**********************************************************/

#define FBLOCK_BP_IGNORE 0
#define FBLOCK_BP_BEFORE 1
#define FBLOCK_BP_AFTER 2
#define FBLOCK_BP_BOTH 4

/**********************************************************/

class FBlock {
	private:
		FILE* f;

		LONG bPos0;  // bytes delante del bloque
		LONG bTail;  // bytes detras del bloque
		LONG bPos;  // posicion actual
		LONG bTotSize;  // numero de bytes
		BOOL writeMode;  // TRUE si en modo Write
		UINT borderpolicy;  // b0->begin  b1->end
		BOOL stdinout; // TRUE si se esta en stdin o stdout;

		VOID setRW( BOOL write );

	public:
		FBlock( VOID );

		VOID attach( FILE* fBin, LONG N=-1, UINT borderpolicy=4 );

		VOID setPos( LONG pos );
		LONG getPos( VOID ) const { return (LONG)bPos; }
		LONG getN( VOID ) const {	return (LONG)bTotSize; }

		UINT get( BYTE& byte );
		UINT set( BYTE byte ) { return set(&byte,1); }
		UINT ins( BYTE byte ) { return ins(&byte,1); }
		UINT del( VOID ) { return del(1); }

		UINT get( BYTE* block, UINT len );
		UINT set( const BYTE* block, UINT len );
		UINT ins( const BYTE* block, UINT len );
		UINT del( UINT len );

		VOID toggleRW( VOID );

		BOOL OK ( VOID ) const;

		~FBlock( VOID ) { f=0; }
};

/**********************************************************/

#endif
