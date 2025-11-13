#ifndef __AFWAV_I_HPP__
#define __AFWAV_I_HPP__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao 
                  & CSTR - Univ. of Edinburgh

Nombre fuente................ -
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
0.0.2    30/08/98  Borja     split en varios modulos afwav_?.cpp
0.0.0    15/07/96  Borja     codificacion inicial.

======================== Contenido ========================
ver afwav.cpp
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"
#include "caudiox.hpp"
#include "uti.h"

/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(push,1)
#elif __CC_BORLANDC__
typedef struct {CHAR8 x; INT2 y; } ____tmp_tdef_align_test___;
#if sizeof(____tmp_tdef_align_test___)>3
#error You must disable Word Alignment
#endif
#endif

/**********************************************************/
/* Definir IGNORE_CUE_LABEL para ignorar los labels de los
cue points. Se usan solo los cue-notes como etiquetas para
las marcas */

#define IGNORE_CUE_LABEL

/**********************************************************/

typedef struct {
	CHAR8 chunkName [4] MINALIGN;
	UINT32 len MINALIGN;
} WAVChunk;

typedef struct {
	CHAR8 Literal_RIFF[4] MINALIGN;
	UINT32 len MINALIGN;
	CHAR8 Literal_WAVE[4] MINALIGN;
} WAVHdr ;

#define WFORMAT_PCM 0x0001

typedef struct {
	UINT16 wFormatTag MINALIGN;       // Format category
	UINT16 wChannels MINALIGN;        // Number of channels
	UINT32 dwSamplesPerSec MINALIGN;  // Sampling rate
	UINT32 dwAvgBytesPerSec MINALIGN; // For buffer estimation
	UINT16 wBlockAlign MINALIGN;      // Data block size
} FormatInfo;

typedef struct {
	UINT32 dwName MINALIGN;
	UINT32 dwPosition MINALIGN;
	char fccChunk[4] MINALIGN;
	UINT32 dwChunkStart MINALIGN;
	UINT32 dwBlockStart MINALIGN;
	UINT32 dwSampleOffset MINALIGN;
} CuePoint;


class UInt32 {
	public: UINT32 i;
	public:
		UInt32( ) {};
		UInt32( UINT32 n ): i(n) {};
		UInt32( const UInt32 &n ): i(n.i) {};
		UInt32& operator = (const UInt32 & a) { i=a.i; return *this; };
};

int operator < (const UInt32& a, const UInt32& b);
int operator == (const UInt32& a, const UInt32& b);
int operator > (const UInt32& a, const UInt32& b);

typedef SetListT<UInt32> CueNames;

/**********************************************************/

char *wstr( FILE *f );
char *wstr( FILE *f, UINT32 l );
VOID mrk_labl(Mark1DList &m, CueNames &cn, CHAR8 *buf);
VOID mrk_note(Mark1DList &m, CueNames &cn, CHAR8 *buf);
VOID mrk_ltxt(Mark1DList &m, CueNames &cn, CHAR8 *buf);

size_t strlen_zeven( const char *s );
void puts_zeven( const char * s, FILE *f );

/**********************************************************/

//#ifdef __CC_MSVC__
//#pragma pack(pop)
//#endif

/**********************************************************/

#endif
