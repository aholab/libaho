#ifndef __XFFT_H
#define __XFFT_H



/////////////////////////////////////////////////////////////////////
/*!
   \file xfft.h
   \brief Funciones para el calculo eficiente y reiterado de FFT, cepstrum y potencia por bandas.

	Copyright: 2010 - Grupo de Voz (DAET) ETSII/IT-Bilbao
	\author Borja Etxebarria
	\version 1.4.0
	\date 02/01/97

	\if SKIP
	Version  dd/mm/aa  Autor     Proposito de la edicion
	-------  --------  --------  -----------------------
	1.5.0    19/02/10  ikerl     Calculo de potencia por bandas y mel-cepstrum ('clase' fband).
	1.4.0    02/01/97  Borja     nuevas funcs: getwinvec(), set/get0piweight
	1.3.2    27/08/95  Borja     SPL_FLOAT32 --> FLOAT32
	1.3.1    30/07/95  Borja     TIPO SPL_PTR --> pTIPO
	1.3.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
	1.2.0    28/10/93  Borja     Soporte de diferentes tipos de enventanado.
	1.1.0    01/02/93  Borja     Calculo de cepstrum fft-modulo ('clase' cepfm).
	1.0.0    16/03/92  Borja     Codificacion inicial.
	\endif

	Hay varios bloques de funciones, cada una formando una 'clase':

	- cfft : FFT de funciones complejas
	- rfft : FFT de funciones reales
	- cepfm : Cálculo de los cepstrum-FFT-modulo
	- fband : Cálculo de la potencia por bandas

	Las funciones estan organizadas segun los bloques indicados.
	Estan programadas en C estandar utilizando 'handlers', siguiendo
	una estructura proxima a la programacion orientada a objetos C++.
	Para cada 'clase' (cfft , rfft , cepfm, fband) hay una funcion
	constructora (???_construct()) que crea y devuelve un puntero a una
	estructura (CFFT, RFFT, CEPFM, FBAND) de la que no es necesario conocer
	nada externamente. El resto de las funciones tienen siempre como
	primer parametro este puntero.

	Despues del uso, la estructura debe destruirse mediante la funcion
	???_destruct() que libera toda la memoria reservada. El sistema es
	similar al utilizado por las librerias de ficheros del C (estructura
	FILE y funciones fopen(), fread(), fclose()). Al crear la estructura
	(???_construct()) se debe reservar memoria dinamica. Si no es posible el
	constructor devuelve un puntero a NULL. En otras funciones distintas del
	constructor que tambien impliquen gestion dinamica de memoria, se
	{devuelve} un valor booleano (SPL_BOOL) de valor SPL_TRUE si el proceso
	se realiza correctamente, o SPL_FALSE si no se puede completar.

	Estos juegos de funciones son adecuados cuando se quiera efectuar
	FFT repetidamente. Primero se construye la estructura adecuada con
	la funcion ???_construct(), se realizan todas las FFT que se quiera
	y finalmente se destruye la estructura con ???_destruct(). La
	estructura almacena informacion reutilizable en todas las FFT que
	queramos efectuar (tablas de senos y cosenos, funcion de
	enventanado...) acelerando el proceso de calculo al no tener que
	calcular esta informacion en cada FFT.

	Definir NDEBUG para desconectar la validacion de parametros
	con assert(). No definir este simbolo mientras se depuren
	aplicaciones, ya que aunque las funciones son algo mas lentas,
	el chequeo de validacion de parametros resuelve muchos problemas.

	\note Para la reserva de memoria dinamica se han definido
	en _XFFTX.H dos macros: xfft_malloc() y xfft_free(), que mapean
	a las funciones _malloc() y _free(), definidas en _ALLOC.H.


*/


#include "spl.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/*! \brief Codigos para funcion de enventanado 
 * 
 * Esta enumeración define los posibles enventanados que pueden aplicarse a
 * la señal para la extracción de la FFT.
 * El tipo de enventanado puede indicarse al construir una nueva estructura con
 * ???_construct() o mediante la función ???_setwin().
 */
typedef enum {
	XFFT_WIN_USER = -1,  ///< funcion de enventanado definida por usuario
	XFFT_WIN_NONE = 0,   ///< sin enventanado (ventana rectangular)
	XFFT_WIN_RECT = XFFT_WIN_NONE, ///< sin enventanado (v. rectangular)
	XFFT_WIN_HAMM,       ///< ventana de hamming
	XFFT_WIN_HANN,       ///< ventana de hanning
	XFFT_WIN_BART,       ///< ventana de bartlett
	XFFT_WIN_BLACK,      ///< ventana de blackman
	XFFT_WIN_KAIS5,      ///< ventana de kaiser, con beta=5
	XFFT_WIN_KAIS6,      ///< ventana de kaiser, con beta=6
	XFFT_WIN_KAIS7,      ///< ventana de kaiser, con beta=7
	XFFT_WIN_KAIS8,      ///< ventana de kaiser, con beta=8
	XFFT_WIN_KAIS9,      ///< ventana de kaiser, con beta=9
	XFFT_WIN_KAIS10      ///< ventana de kaiser, con beta=10
} XFFT_WIN;


/*! \brief Definicion de la funcion de enventanado.
 * 
 * Si el usuario suministra una funcion de enventanado propia
 * (XFFT_WIN_USER), debe cumplir esta definicion.
 * 
 * \code
 * 		SPL_pFLOAT mi_funcion( SPL_pFLOAT v, SPL_INT N )
 * \endcode
 * 
 * Funcion que recibe 2 parametros: 
 * 
 * 	\param v puntero a un vector de elementos de tipo SPL_FLOAT.
 * 	\param N entero que indica la longitud del vector anterior.
 * 
 * La funcion debe rellenar el vector \p v con los \p N valores de forman
 * una ventana de longitud \p N.
 * 
 * 	\return El propio puntero \p v
 */
typedef SPL_pFLOAT (* XFFT_WIN_FUNC)( SPL_pFLOAT v, SPL_INT N );


/*!
 * \brief Códigos para la forma del filtro para FBAND
 * 
 * Esta enumeración define las posibles formas de los filtros frecuenciales
 * para la extracción de la potencia por bandas.
 * El tipo de escala puede indicarse al construir una nueva estructura con
 * ???_construct() o mediante la función ???_setfilter().
 */
typedef enum {
	XFBAND_FILTER_TRIANG, 	///<Filtro triangular
	XFBAND_FILTER_RECT		///<Filtro rectangular
} XFBAND_FILTER;


/*!
 * \brief Códigos para la escala del filtro para FBAND
 * 
 * Esta enumeración define las posibles escalas de filtrado que pueden aplicarse a
 * la señal para la extracción de la potencia por bandas.
 * El tipo de escala puede indicarse al construir una nueva estructura con
 * ???_construct() o mediante la función ???_setscale().
 */
typedef enum {
	XFBAND_SCALE_LIN,		///<Escala lineal
	XFBAND_SCALE_MEL,		///<Escala MEL
	XFBAND_SCALE_BARK,		///<Escala Bark
	XFBAND_SCALE_LOG,		///<Escala logarítmica (neperiano)
	XFBAND_SCALE_LOG10,		///<Escala logarítmica (log10)
	XFBAND_SCALE_BIN		///<Esto NO es una escala válida, pero simplifica las cosas para la función FbandScaleConvert()
} XFBAND_SCALE;


/*!
	\brief Conversión de escalas de frecuencia
*/
SPL_FLOAT FbandScaleConvert ( SPL_FLOAT f, XFBAND_SCALE from, XFBAND_SCALE to, SPL_FLOAT Fres );
	
/*!\name Funciones de conversión
	Funciones auxiliares de bajo nivel utilizadas internamente por FbandScaleConvert
	para las conversiones de frecuencia
*/
//\{	
	
	SPL_FLOAT Hz2Log ( SPL_FLOAT f );
	SPL_FLOAT Log2Hz ( SPL_FLOAT f );
	SPL_FLOAT Hz2Log10 ( SPL_FLOAT f );
	SPL_FLOAT Log102Hz ( SPL_FLOAT f );
	SPL_FLOAT Hz2Mel ( SPL_FLOAT f );
	SPL_FLOAT Mel2Hz ( SPL_FLOAT f );
	SPL_FLOAT Hz2Bark ( SPL_FLOAT f );
	SPL_FLOAT Bark2Hz ( SPL_FLOAT f );
	SPL_FLOAT Hz2Bin ( SPL_FLOAT f, SPL_FLOAT fres );
	SPL_FLOAT Bin2Hz ( SPL_FLOAT f, SPL_FLOAT fres );

//\}



/**********************************************************/
/*! \brief estructura "objeto" para FFT compleja 

	\sa xfft.h xfft1.c
*/

typedef struct _CFFT {
	SPL_INT _nbits;
	SPL_pFLOAT _revec, _imvec;
	SPL_pINT _tinv;
	SPL_pFLOAT _tsin, _tcos;
	SPL_BOOL _inv;
	SPL_FLOAT _ufactor;

	SPL_INT _np;
} CFFT, SPL_PTR pCFFT;


/*! \brief estructura "objeto" para FFT real 

	\sa xfft.h xfft2.c
*/

typedef struct _RFFT {
	SPL_INT _nbits;
	SPL_pFLOAT _revec, _imvec;
	SPL_pINT _tinv;
	SPL_pFLOAT _half_tsin, _tsin, _tcos;
	SPL_BOOL _inv, _weight0pi;
	SPL_FLOAT _ufactor, _ufactor0, _winmean;

	SPL_INT _np;
	XFFT_WIN _win, _owin;
	XFFT_WIN_FUNC _winfunc;
	SPL_pFLOAT _winvec;
} RFFT, SPL_PTR pRFFT;


/*! \brief estructura "objeto" para cepstrum FFT-modulo 

	\sa xfft.h xfft3.c
*/

typedef struct _CEPFM {
	pRFFT _r1;
	pRFFT _r2;
	SPL_pFLOAT _tmp;
} CEPFM, SPL_PTR pCEPFM;


/*! \brief estructura "objeto" para potencia por bandas 

	\sa xfft.h xfft4.c
*/

typedef struct _FBAND {
	pRFFT _fft;					/* Objeto para el cáclulo de la FFT */
	
	SPL_FLOAT _minfreqidx;		/* Índice del bin correspondiente a la mínima frecuencia considerada */
	SPL_FLOAT _maxfreqidx;		/* Índice del bin correspondiente a la máxima frecuencia considerada */
	SPL_FLOAT _srate;			/* Frecuencia de muestreo de la señal 
								   (necesario para calcular resolución frecuencial de FFT) */
	
	SPL_INT _nf;				/* Número de filtros */
	XFBAND_FILTER _filt;		/* Forma de los filtros */
	XFBAND_SCALE _scale;		/* Escala de los filtros */
	
	SPL_pINT _findex;			/* Filtro al que pertenece cada bin FFT */
	SPL_pFLOAT _fweight;		/* Peso de cada bin dentro del filtro */
	SPL_pFLOAT _pvec;			/* Potencia por cada banda */
	
} FBAND, SPL_PTR pFBAND;


/* Función pra el cálculo de coeficientes MFCC a partir de FBAND */
SPL_VOID API fband2mfcc (pFBAND r, SPL_pFLOAT mfcc, SPL_INT nceps, SPL_INT lift);





	


/*!\name metodos de fft compleja
*/
//\{	

pCFFT API cfft_construct( SPL_INT np, SPL_INT fftnp,
		SPL_BOOL inv, SPL_FLOAT ufactor );
SPL_VOID API cfft_destruct( pCFFT c );
SPL_VOID API cfft_setnp( pCFFT c, SPL_INT np );
SPL_VOID API cfft_setinv( pCFFT c, SPL_BOOL inv );
SPL_VOID API cfft_setufactor( pCFFT c, SPL_FLOAT ufactor );
SPL_INT API cfft_getnp( pCFFT c );
SPL_INT API cfft_getfftnp( pCFFT c );
SPL_INT API cfft_getvecnp( pCFFT c );
SPL_BOOL API cfft_getinv( pCFFT c );
SPL_FLOAT API cfft_getufactor( pCFFT c );
SPL_pFLOAT API cfft_getrevec( pCFFT c );
SPL_pFLOAT API cfft_getimvec( pCFFT c );
SPL_VOID API cfft_cfft( pCFFT c, SPL_pFLOAT re_invec, SPL_pFLOAT im_invec );
SPL_VOID API cfft_norm( pCFFT c );
SPL_VOID API cfft_mag( pCFFT c );
SPL_VOID API cfft_arg( pCFFT c );
SPL_VOID API cfft_normarg( pCFFT c );
SPL_VOID API cfft_magarg( pCFFT c );
SPL_VOID API cfft_trefmove_reim( pCFFT c, SPL_FLOAT nTs );
SPL_VOID API cfft_trefmove_arg( pCFFT c, SPL_FLOAT nTs );
SPL_VOID API cfft_trefmove_argm( pCFFT c, SPL_FLOAT nTs );

#define cfft_getnormvec(c)  cfft_getrevec(c)
#define cfft_getmagvec(c)  cfft_getrevec(c)
#define cfft_getargvec(c)  cfft_getimvec(c)
//\}

/*!\name metodos de fft real
*/
//\{	

pRFFT API rfft_construct( SPL_INT np, SPL_INT fftnp, SPL_BOOL inv, SPL_FLOAT ufactor,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_VOID API rfft_destruct( pRFFT r );
SPL_BOOL API rfft_setwin( pRFFT r, XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_BOOL API rfft_setnp( pRFFT r, SPL_INT np );
SPL_BOOL API rfft_setnpwin( pRFFT r, SPL_INT np, XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_VOID API rfft_setinv( pRFFT r, SPL_BOOL inv );
SPL_VOID API rfft_setufactor( pRFFT r, SPL_FLOAT ufactor );
SPL_VOID API rfft_set0piweight( pRFFT r, SPL_BOOL weight );
SPL_BOOL API rfft_get0piweight( pRFFT r );
SPL_INT API rfft_getnp( pRFFT r );
SPL_INT API rfft_getfftnp( pRFFT r );
SPL_INT API rfft_getvecnp( pRFFT r );
XFFT_WIN API rfft_getwin( pRFFT r );
SPL_pFLOAT API rfft_getwinvec( pRFFT r );
XFFT_WIN_FUNC API rfft_getwinfunc( pRFFT r );
SPL_BOOL API rfft_getinv( pRFFT r );
SPL_FLOAT API rfft_getufactor( pRFFT r );
SPL_pFLOAT API rfft_getrevec( pRFFT r );
SPL_pFLOAT API rfft_getimvec( pRFFT r );
SPL_VOID API rfft_rfft( pRFFT r, SPL_pFLOAT invec );
SPL_VOID API rfft_rfft_i( pRFFT r, SPL_pINT invec );
SPL_VOID API rfft_rfft_i16( pRFFT r, pINT16 invec );
SPL_VOID API rfft_rfft_f32( pRFFT r, pFLOAT32 invec );
SPL_VOID API rfft_rfft_i32( pRFFT r, pINT32 invec );
SPL_VOID API rfft_rfft_u32( pRFFT r, pUINT32 invec );
SPL_VOID API rfft_norm( pRFFT r );
SPL_VOID API rfft_mag( pRFFT r );
SPL_VOID API rfft_arg( pRFFT r );
SPL_VOID API rfft_normarg( pRFFT r );
SPL_VOID API rfft_magarg( pRFFT r );
SPL_VOID API rfft_trefmove_reim( pRFFT r, SPL_FLOAT nTs );
SPL_VOID API rfft_trefmove_arg( pRFFT r, SPL_FLOAT nTs );
SPL_VOID API rfft_trefmove_argm( pRFFT r, SPL_FLOAT nTs );

#define rfft_getnormvec(r)  rfft_getrevec(r)
#define rfft_getmagvec(r)  rfft_getrevec(r)
#define rfft_getargvec(r)  rfft_getimvec(r)

//\}

/*!\name metodos de cepstrum
*/
//\{	

pCEPFM API cepfm_construct( SPL_INT np, SPL_INT ifftnp, SPL_FLOAT ufactor,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_VOID API cepfm_destruct( pCEPFM c );
SPL_BOOL API cepfm_setwin( pCEPFM c, XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_BOOL API cepfm_setnp( pCEPFM c, SPL_INT np );
SPL_BOOL API cepfm_setnpwin( pCEPFM c, SPL_INT np, XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_VOID API cepfm_setufactor( pCEPFM c, SPL_FLOAT ufactor );
SPL_INT API cepfm_getnp( pCEPFM c );
SPL_INT API cepfm_getcepnp( pCEPFM c );
XFFT_WIN API cepfm_getwin( pCEPFM c );
XFFT_WIN_FUNC API cepfm_getwinfunc( pCEPFM c );
SPL_FLOAT API cepfm_getufactor( pCEPFM c );
SPL_VOID API cepfm_cepfm( pCEPFM c, SPL_pFLOAT invec );
SPL_VOID API cepfm_cepfm_i( pCEPFM c, SPL_pINT invec );
SPL_VOID API cepfm_cepfm_i16( pCEPFM c, pINT16 invec );
SPL_VOID API cepfm_cepfm_f32( pCEPFM c, pFLOAT32 invec );
SPL_pFLOAT API cepfm_getcepfm( pCEPFM c );

//\}

/*!\name metodos de potencia por bandas
*/
//\{	
pFBAND XAPI fband_construct( SPL_INT np, SPL_INT nband, XFBAND_FILTER filter,
		XFBAND_SCALE scale, SPL_FLOAT minfreq, SPL_FLOAT maxfreq, SPL_FLOAT fs,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_VOID XAPI fband_destruct( pFBAND r );
SPL_BOOL XAPI fband_setwin( pFBAND r, XFFT_WIN win, XFFT_WIN_FUNC winfunc );
SPL_BOOL XAPI fband_setnp( pFBAND r, SPL_INT np );
SPL_BOOL XAPI fband_setnband( pFBAND r, SPL_INT nband );
SPL_BOOL XAPI fband_setfilters( pFBAND r, XFBAND_FILTER filter, XFBAND_SCALE scale );
SPL_BOOL XAPI fband_setfreqlimits( pFBAND r, SPL_FLOAT minfreq, SPL_FLOAT maxfreq );
SPL_BOOL XAPI fband_setminfreq( pFBAND r, SPL_FLOAT minfreq );
SPL_BOOL XAPI fband_setmaxfreq( pFBAND r, SPL_FLOAT maxfreq );
SPL_BOOL XAPI fband_setsrate( pFBAND r, SPL_FLOAT srate );
SPL_INT XAPI fband_getnp( pFBAND r );
SPL_INT XAPI fband_getnband( pFBAND r );
XFFT_WIN XAPI fband_getwin( pFBAND r );
SPL_pFLOAT XAPI fband_getwinvec( pFBAND r );
XFFT_WIN_FUNC XAPI fband_getwinfunc( pFBAND r );
SPL_pFLOAT XAPI fband_getpband( pFBAND r );
SPL_FLOAT XAPI fband_getminfreq( pFBAND r );
SPL_FLOAT XAPI fband_getmaxfreq( pFBAND r );
SPL_FLOAT XAPI fband_getsrate( pFBAND r );
SPL_VOID XAPI fband_fband( pFBAND r, SPL_pFLOAT invec );
SPL_VOID XAPI fband_fband_i( pFBAND r, SPL_pINT invec );
SPL_VOID XAPI fband_fband_i16( pFBAND r, pINT16 invec );
SPL_VOID XAPI fband_fband_i32( pFBAND r, pINT32 invec );
SPL_VOID XAPI fband_fband_u32( pFBAND r, pUINT32 invec );
SPL_VOID XAPI fband_fband_f32( pFBAND r, pFLOAT32 invec );


//\}



/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif

