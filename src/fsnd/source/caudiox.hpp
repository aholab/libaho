#ifndef __CAUDIOX_HPP__
#define __CAUDIOX_HPP__

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
#include "caudio.hpp"

/**********************************************************/

class AFFormat {
	protected:
		CAudioFile *fa;
	public:
		AFFormat( CAudioFile *ca ) { fa = ca; };
		virtual ~AFFormat( VOID ) {};
		virtual BOOL OK( VOID ) = 0;
		virtual const CHAR* fFormat( VOID ) = 0;

		virtual LONG HdrR( FILE *f, KVStrList &def, BOOL override ) = 0;
		virtual VOID HdrG( KVStrList &def, BOOL override ) = 0;
		virtual VOID HdrW( FILE *f, LONG nSamp ) = 0;
};

/**********************************************************/

#define ADD(p,v) fa->info().add(p,v)
#define NOV(p) ((!fa->info().contains(p))|| override)
#define ADDIFNOV(p,v) if (NOV(p)) ADD(p,v)

/**********************************************************/

class AFRaw : public AFFormat {
	public:
		AFRaw( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Raw"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};

/**********************************************************/

class AFAho2 : public AFFormat {
	protected:
		LONG lcgs;  /* longitud de la ultima cabecera leida/escrita */

	public:
		AFAho2( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Aho2"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};

/**********************************************************/

class AFTei : public AFFormat {
	public:
		AFTei( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Tei"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};

/**********************************************************/

class AFWav : public AFFormat {
		long datapos;
	public:
		AFWav( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Wav"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};


/**********************************************************/

class AFAho1 : public AFFormat{
	public:
		AFAho1( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Aho1"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};

/**********************************************************/

class AFNist : public AFFormat {
	public:
		AFNist( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Nist"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};

/**********************************************************/

class AFHak : public AFFormat {
		LONG lc;  /* longitud de la ultima cabecera leida/escrita */
	public:
		AFHak( CAudioFile *ca ) : AFFormat(ca) {};
		BOOL OK( VOID ) {return TRUE;};
		static const CHAR* formatStr( VOID ) { return "Hak"; };
		const CHAR* fFormat( VOID ) { return formatStr(); };
		static BOOL testFile( FILE *f );

		LONG HdrR( FILE *f, KVStrList &def, BOOL override );
		VOID HdrG( KVStrList &def, BOOL override );
		VOID HdrW( FILE *f, LONG nSamp );
};

/**********************************************************/

#endif
