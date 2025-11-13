#ifndef __SAMP_HPP__
#define __SAMP_HPP__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SAMP.HPP
Nombre paquete............... -
Lenguaje fuente.............. C++ (BC31, GNUC)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    09/04/99  Borja     uso tipos UINT
0.1.0    05/05/97  Borja     get_i16(), get_d()
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

#include "fblock.hpp"

/**********************************************************/

enum {
	SAMPTYPE_PCM8U=1,
	SAMPTYPE_PCM16, 	
	SAMPTYPE_PCM32, 	
	SAMPTYPE_FLOAT32,	
	SAMPTYPE_FLOAT64, 
};

#define SAMPTYPE_STR_PCM8U "PCM8U"
#define SAMPTYPE_STR_PCM16 "PCM16"
#define SAMPTYPE_STR_PCM32 "PCM32"
#define SAMPTYPE_STR_FLOAT32 "Float32"
#define SAMPTYPE_STR_FLOAT64 "Float64"

#define SAMP_ENDIAN_MACHINE 0
#define SAMP_ENDIAN_LITTLE 1
#define SAMP_ENDIAN_BIG -1

/**********************************************************/

class Samp: public FBlock {
	private:
		INT tSamp;  // tipo muestra
		UINT sSize;  // tamanyo en bytes de la muestra
		VOID *ptr;  // buffer interno para get_i16() y get_d()
		LONG nptr;  // tamanyo de ptr
		LONG pptr;  // posicion de ptr
		INT tptr;   // tipo get???_?() usado la ultima vez (get_i16, etc)
		INT endian; // 0=machine 1=little -1=big endian (defines SAMP_ENDIAN_???)
		UINT nchan;  // numero de canales
		UINT sSizeCh; // tamanyo en bytes de la "muestra" (varios canales)

	public:
		Samp( VOID );

		VOID attach( FILE* fBin, INT sampType, UINT nChan=1, LONG N=-1 );
		VOID setendian( INT endi ) { endian=endi; } ;
		INT getendian( VOID ) { return endian; };
		INT getSampType( VOID ) const { return tSamp; }
		static UINT getSampTypeSize( INT sampType );
		UINT getNChan( VOID ) const { return nchan; }

		VOID setPos( LONG pos ) { FBlock::setPos(pos*sSizeCh); }
		LONG getPos( VOID ) const { return FBlock::getPos()/sSizeCh; }
		LONG getN( VOID ) const { return FBlock::getN()/sSizeCh; }
		VOID setPosNoCh( LONG pos ) { FBlock::setPos(pos*sSize); }
		LONG getPosNoCh( VOID ) const { return FBlock::getPos()/sSize; }
		LONG getNNoCh( VOID ) const { return FBlock::getN()/sSize; }

		UINT getNoCh( INT16& sample );
		UINT setNoCh( INT16 sample );
		UINT insNoCh( INT16 sample );
		UINT getNoCh( DOUBLE& sample );
		UINT setNoCh( DOUBLE sample );
		UINT insNoCh( DOUBLE sample );
		UINT delNoCh( VOID ) { return FBlock::del(sSize)/sSize; }
		UINT getBlkNoCh( INT16* block, UINT len );
		UINT setBlkNoCh( const INT16* block, UINT len );
		UINT insBlkNoCh( const INT16* block, UINT len );
		UINT getBlkNoCh( DOUBLE* block, UINT len );
		UINT setBlkNoCh( const DOUBLE* block, UINT len );
		UINT insBlkNoCh( const DOUBLE* block, UINT len );
		UINT delBlkNoCh( UINT len ) { return FBlock::del(len*sSize)/sSize; }

		UINT getCh( INT16& sample, UINT ch );
		UINT setCh( INT16 sample, UINT ch );
		UINT insCh( INT16 sample, UINT ch );
		UINT getCh( DOUBLE& sample, UINT ch );
		UINT setCh( DOUBLE sample, UINT ch );
		UINT insCh( DOUBLE sample, UINT ch );
		UINT delCh( UINT ch ) { (void)ch; return delBlkNoCh(nchan); }
		UINT getBlkCh( INT16* block, UINT len, UINT ch );
		UINT setBlkCh( const INT16* block, UINT len, UINT ch );
		UINT insBlkCh( const INT16* block, UINT len, UINT ch );
		UINT getBlkCh( DOUBLE* block, UINT len, UINT ch );
		UINT setBlkCh( const DOUBLE* block, UINT len, UINT ch );
		UINT insBlkCh( const DOUBLE* block, UINT len, UINT ch );
		UINT delBlkCh( UINT len, UINT ch ) { (void)ch; return delBlkNoCh(nchan*len); }

		UINT getChAll( INT16* sampr ) { return getBlkNoCh(sampr,nchan); }
		UINT setChAll( const INT16* sampr ) { return setBlkNoCh(sampr,nchan); }
		UINT insChAll( const INT16* sampr ) { return insBlkNoCh(sampr,nchan); }
		UINT getChAll( DOUBLE* sampr ) { return getBlkNoCh(sampr,nchan); }
		UINT setChAll( const DOUBLE* sampr ) { return setBlkNoCh(sampr,nchan); }
		UINT insChAll( const DOUBLE* sampr ) { return insBlkNoCh(sampr,nchan); }
		UINT delChAll( VOID ) { return delBlkNoCh(nchan); }
		UINT getBlkChAll( INT16* block, UINT len ) { return getBlkNoCh(block,nchan*len); }
		UINT setBlkChAll( const INT16* block, UINT len ) { return setBlkNoCh(block,nchan*len); }
		UINT insBlkChAll( const INT16* block, UINT len ) { return insBlkNoCh(block,nchan*len); }
		UINT getBlkChAll( DOUBLE* block, UINT len ) { return getBlkNoCh(block,nchan*len); }
		UINT setBlkChAll( const DOUBLE* block, UINT len ) { return setBlkNoCh(block,nchan*len); }
		UINT insBlkChAll( const DOUBLE* block, UINT len ) { return insBlkNoCh(block,nchan*len); }
		UINT delBlkChAll( UINT len ) { return delBlkNoCh(nchan*len); }

		UINT getChRange( INT16* sampr, UINT ch1, UINT ch2 );
		UINT setChRange( const INT16* sampr, UINT ch1, UINT ch2 );
		UINT insChRange( const INT16* sampr, UINT ch1, UINT ch2 );
		UINT getChRange( DOUBLE* sampr, UINT ch1, UINT ch2 );
		UINT setChRange( const DOUBLE* sampr, UINT ch1, UINT ch2 );
		UINT insChRange( const DOUBLE* sampr, UINT ch1, UINT ch2 );
		UINT delChRange( UINT ch1, UINT ch2 ) { (void)ch1; (void)ch2; return delBlkNoCh(nchan); }
		UINT getBlkChRange( INT16* block, UINT len, UINT ch1, UINT ch2 );
		UINT setBlkChRange( const INT16* block, UINT len, UINT ch1, UINT ch2 );
		UINT insBlkChRange( const INT16* block, UINT len, UINT ch1, UINT ch2 );
		UINT getBlkChRange( DOUBLE* block, UINT len, UINT ch1, UINT ch2 );
		UINT setBlkChRange( const DOUBLE* block, UINT len, UINT ch1, UINT ch2 );
		UINT insBlkChRange( const DOUBLE* block, UINT len, UINT ch1, UINT ch2 );
		UINT delBlkChRange( UINT len, UINT ch1, UINT ch2 ) { (void)ch1; (void)ch2; return delBlkNoCh(nchan*len); }


		UINT get( INT16& sample ) { return getCh(sample,0); }
		UINT set( INT16 sample ) { return setCh(sample,0); }
		UINT ins( INT16 sample ) { return insCh(sample,0); }
		UINT get( DOUBLE& sample ) { return getCh(sample,0); }
		UINT set( DOUBLE sample ) { return setCh(sample,0); }
		UINT ins( DOUBLE sample ) { return insCh(sample,0); }
		UINT del( VOID ) { return delCh(0); }
		UINT getBlk( INT16* block, UINT len ) { return getBlkCh(block,len,0); }
		UINT setBlk( const INT16* block, UINT len ) { return setBlkCh(block,len,0); }
		UINT insBlk( const INT16* block, UINT len ) { return insBlkCh(block,len,0); }
		UINT getBlk( DOUBLE* block, UINT len ) { return getBlkCh(block,len,0); }
		UINT setBlk( const DOUBLE* block, UINT len ) { return setBlkCh(block,len,0); }
		UINT insBlk( const DOUBLE* block, UINT len ) { return insBlkCh(block,len,0); }
		UINT delBlk( UINT len ) { return delBlkCh(len,0); }

		INT16 *get_i16( UINT len, BOOL reloadall=FALSE );
		DOUBLE *get_d( UINT len, BOOL reloadall=FALSE );
		INT16 *getChAll_i16( UINT len, BOOL reloadall=FALSE );
		DOUBLE *getChAll_d( UINT len, BOOL reloadall=FALSE );

		VOID toggleRW( VOID ) { FBlock::toggleRW(); }

		BOOL OK( VOID ) const;

		~Samp( VOID );
};

/**********************************************************/

INT SampType_a2i( const CHAR* sampType, BOOL errexit=TRUE );
const CHAR* SampType_i2a( INT sampType, BOOL errexit=TRUE );

/**********************************************************/

#endif
