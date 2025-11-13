/////////////////////////////////////////////////////////////////////
/*!
   \file xfft4.c
   \brief Cálculo de potencia por bandas

	Copyright: 2010 - Grupo de Voz (DAET) ETSII/IT-Bilbao
	\author Iker Luengo
	\version 1.0.0
	\date 19/02/10

	\if SKIP
	Version  dd/mm/aa  Autor     Proposito de la edicion
	-------  --------  --------  -----------------------
	1.0.0    19/02/10  ikerl     Codificación inicial
	\endif

	Funciones para el calculo eficiente y reiterado de
 	potencia por bandas.

	Los coeficientes por bandas se definen como la potencia de señal que cae dentro
	de un banco de filtros. La forma y escala de estos filtros viene determinada por
	valores tipo XFBAND_FILTER y XFBAND_SCALE respectivamente.

	La potencia de cada banda se estima mediante el cálculo de la FFT de la secuencia
	y un posterior filtrado en frecuencia.
*/

#include "_xfft.h"




SPL_FLOAT Hz2Log ( SPL_FLOAT f )
{ return log(f); }

SPL_FLOAT Log2Hz ( SPL_FLOAT f )
{ return exp(f); }

SPL_FLOAT Hz2Log10 ( SPL_FLOAT f )
{ return log10(f); }

SPL_FLOAT Log102Hz ( SPL_FLOAT f )
{ return pow(10, f); }

SPL_FLOAT Hz2Mel ( SPL_FLOAT f )
{ return 1127.0*log(1.0+f/700.0); }

SPL_FLOAT Mel2Hz ( SPL_FLOAT f )
{ return 700.0*(exp(f/1127.0)-1.0); }

SPL_FLOAT Hz2Bark ( SPL_FLOAT f )
{
	SPL_FLOAT b = (26.81*f) / (1960+f) - 0.53;
	if (b < 2) b = 0.85*b + 0.3;
	else if (b > 20.1) b = 1.22*b - 4.422;
	return b;
}

SPL_FLOAT Bark2Hz ( SPL_FLOAT f )
{
	if (f < 2) f = (f-0.3)/0.85;
	else if (f > 20.1) f = (f+4.422)/1.22;
	return 1960*(f+0.53)/(26.28-f);
}
	
SPL_FLOAT Hz2Bin ( SPL_FLOAT f, SPL_FLOAT fres )
{ return floor(f/fres); }

SPL_FLOAT Bin2Hz ( SPL_FLOAT f, SPL_FLOAT fres )
{ return f*fres; }

/*!
	Permite convertir de forma sencilla entre las diferentes escalas de frecuencia
	definidas por XFBAND_SCALE:
	Frecuencias lineales, en escala logarítmica, escala Mel, bins de un análisis FFT...
 
	\param f Valor a convertir
	\param from Escala a la que está el valor \p f
	\param to Escala a la que se quiere convertir
	\param Fres Resolución frecuencial del análisis FFT (sólo para los casos en los que
			\p from o \p to sean XFBAND_SCALE_BIN. Para cualquier otro caso, poner a 0)
 
	\return El valor \p f convertido a la escala \p to
*/
SPL_FLOAT FbandScaleConvert ( SPL_FLOAT f, XFBAND_SCALE from, XFBAND_SCALE to, SPL_FLOAT Fres )
{
	/*Primero convertimos lo que sea a lineal*/
	switch (from)
	{
		case XFBAND_SCALE_MEL:
			f = Mel2Hz(f);
			break;
		case XFBAND_SCALE_BARK:
			f = Bark2Hz(f);
			break;
		case XFBAND_SCALE_BIN:
			f = Bin2Hz(f, Fres);
			break;
		case XFBAND_SCALE_LOG:
			f = Log2Hz(f);
			break;
		case XFBAND_SCALE_LOG10:
			f = Log102Hz(f);
			break;
		default:
			break;
	}
	
	/*Ahora convertimos de lineal a la salida*/
	switch (to)
	{
		case XFBAND_SCALE_MEL:
			f = Hz2Mel(f);
			break;
		case XFBAND_SCALE_BARK:
			f = Hz2Bark(f);
			break;
		case XFBAND_SCALE_BIN:
			f = Hz2Bin(f, Fres);
			break;
		case XFBAND_SCALE_LOG:
			f = Hz2Log(f);
			break;
		case XFBAND_SCALE_LOG10:
			f = Hz2Log10(f);
			break;
		default:
			break;
	}
	
	return f;

}


/*!
	\brief uso interno
*/
/* Permite recalcular los índices de banda y los pesos para cada bin.
   Devuelve SPL_TRUE si todo va bien y SPL_FALSE si ha habido errores.
   En caso de error, libera toda la memoria que se haya reservado ya.
*/
PRIVATE SPL_BOOL fband_calcfilters( pFBAND r )
{
	SPL_INT fftnsamples;
	SPL_FLOAT fres, outFreqMin, outFreqMax;
	SPL_FLOAT filtSize, outBin;
	SPL_INT i;
	SPL_FLOAT tmp, tmp2;


	assert(r!=NULL);
	assert(r->_filt == XFBAND_FILTER_RECT || r->_filt == XFBAND_FILTER_TRIANG);
	
	if (r->_fft == NULL)
		return SPL_FALSE;

	fftnsamples = rfft_getfftnp(r->_fft);
	
	/*Primero liberamos la memoria*/
	if (r->_findex) xfft_free(r->_findex);
	if (r->_fweight) xfft_free(r->_fweight);
	if (r->_pvec) xfft_free(r->_pvec);		//Podría sustituirse en una función externa setnband
	
	r->_findex = (SPL_pINT)xfft_malloc(sizeof(SPL_INT)*fftnsamples);
	r->_fweight = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*fftnsamples);
	r->_pvec = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*r->_nf);
	
	if (r->_findex == NULL || r->_fweight == NULL || r->_pvec == NULL) {
		if (r->_findex) xfft_free(r->_findex);
		if (r->_fweight) xfft_free(r->_fweight);
		if (r->_pvec) xfft_free(r->_pvec);
		return SPL_FALSE;
	}
	
	/* Necesitamos minfreq y maxfreq convertidos a la escala solicitada
	   para crear los filtros */
	fres = r->_srate/rfft_getfftnp(r->_fft);
	outFreqMin = FbandScaleConvert(r->_minfreqidx, XFBAND_SCALE_BIN, r->_scale, fres);
	outFreqMax = FbandScaleConvert(r->_maxfreqidx, XFBAND_SCALE_BIN, r->_scale, fres);
	
	switch (r->_filt)
	{
		case XFBAND_FILTER_RECT:
		{
			filtSize = (outFreqMax-outFreqMin)/r->_nf;	/*Anchura de cada filtro*/
			for (i = r->_minfreqidx; i <= r->_maxfreqidx; ++i)
			{
				/*Calculamos la frecuencia escalada a la que corresponde ese bin*/
				outBin = FbandScaleConvert(i, XFBAND_SCALE_BIN, r->_scale, fres);
		
				/*El peso y el filtro al que corresponde*/
				tmp = (outBin-outFreqMin)/filtSize;
				tmp2;
				modf (tmp, &tmp2);
				r->_fweight[i] = 1.0;	/* Peso unidad por ser filtros rectangulares */
				r->_findex[i] = (SPL_INT)tmp2;
		
				/*Para Fs nos queda que le corresponde el filtro r->_nf, el cual no existe.
					El problema es que está en la frontera entre dos filtros. Podemos ponerselo al anterior*/
				if (r->_findex[i] == r->_nf)
					r->_findex[i] = r->_nf-1;
		
				/*Escalamos los pesos por el número de bins para terminar teniendo potencia independiente
					de la NFFT (que cambiará con la Fs)*/
				r->_fweight[i] /= fftnsamples;
			}
			break;
		}

		case XFBAND_FILTER_TRIANG:
		{
			filtSize = (outFreqMax-outFreqMin)/(r->_nf+1)*2;	/*Anchura de cada filtro*/
			for (i = r->_minfreqidx; i <= r->_maxfreqidx; ++i)
			{
				/*Calculamos la frecuencia escalada a la que corresponde ese bin*/
				SPL_FLOAT outBin = FbandScaleConvert(i, XFBAND_SCALE_BIN, r->_scale, fres);
		
				/*El peso y el filtro al que corresponde*/
				tmp = (outBin-outFreqMin)*2/filtSize;
				tmp2;
				r->_fweight[i] = modf (tmp, &tmp2);
				r->_findex[i] = (SPL_INT)tmp2;
		
				/*Para Fs nos queda que le corresponde el filtro r->_nf+1, el cual no existe.
					El problema es que está en la frontera entre dos filtros. Podemos ponerselo al anterior
					con pesos modificados*/
				if (r->_findex[i] == r->_nf+1) {
					r->_findex[i] = r->_nf;
					r->_fweight[i] = 1-r->_fweight[i];
					/*peso 0 para el filtro r->_nf+1, peso 1 para r->_nf*/
				}
		
				/*Escalamos los pesos por el número de bins para terminar teniendo potencia independiente
					de la NFFT (que cambiará con la Fs)*/
				r->_fweight[i] /= fftnsamples;
			}
		
			break;
		}
	}
	
	return SPL_TRUE;
}


/*!
   \brief Inicialización de _FBAND
   
	Inicializa una estructura _FBAND para el cálculo de potencia por bandas

   \param np Número de puntos del vector de entrada (del que se quiere calcular la potencia por bandas).
			Ha de ser mayor que cero, pero no forzosamente potencia de dos.
			En caso de que no sea potencia de dos, se rellena automáticamente con ceros 

   \param nband Número de bandas. Ha de ser mayor acero

   \param filter Forma del filtro de frecuencia a aplicar

   \param scale Escala en la que se deben distribuir las bandas. En escala lineal (XFBAND_SCALE_LIN)
			todos los filtros tienen el mismo ancho de banda y están equiespaciados.
			En otras escalas los filtros tienen el mismo ancho de banda y están 
			equiespaciados si las frecuencias se miden en la escala indicada.
			Esto significa que, vistos en Herzios, su ancho de banda
			y su posición varía con la frecuencia.

   \param minfreq Frecuencia mínima a considerar (eh Hz). Los \p nband filtros se distribuyen
			entre \p minfreq y \p maxfreq.

   \param maxfreq Frecuencia máxima a considerar (eh Hz). Los \p nband filtros se distribuyen
			entre \p minfreq y \p maxfreq. Si se deja a 0 se considera la frecuencia de Nyquist=fs/2

   \param fs Frecuencia de muestreo de la señal de entrada. Necesario para poder calcular a qué frecuencia
			corresponde cada bin FFT.

   \param win Codigo que identifica el tipo de ventana a utilizar. Estos tipos est�n
			definidos en xfft.h (XFFT_WIN). Si se utiliza una ventana, sera de \p np puntos
			aunque si el numero de puntos de la fft es menor, se utiliza este otro valor.

   \param winfunc Puntero a una funcion que que suministra el usuario, y que 
			sigue el prototipo indicado (xfft.h). Esta funcion se utiliza para
			obtener la ventana cuando en \p win se envia XFFT_WIN_USER. 
			Si este puntero es NULL y se envia XFFT_WIN_USER en \p win, 
			no se utiliza ningun enventanado (ventana rectangular). 

   \return Puntero a una estructura _FBAND. Este puntero deberá utilizarse como parámetro 
			para el resto de las funciones \p fband_???(). Devuelve \p NULL si la inicialización falla
*/
PUBLIC pFBAND XAPI fband_construct( SPL_INT np, SPL_INT nband, XFBAND_FILTER filter,
		XFBAND_SCALE scale, SPL_FLOAT minfreq, SPL_FLOAT maxfreq, SPL_FLOAT fs,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc )
{
	pFBAND r = NULL;
	SPL_FLOAT fres;

	assert(np>0);
	assert(nband>0);
	assert(minfreq>=0);
	assert(maxfreq>=0);
	assert(scale!=XFBAND_SCALE_BIN);	//No es una escala válida
	
	r = (pFBAND)xfft_malloc(sizeof(FBAND));
	if (r!=NULL){
		
		/* Primero creamos el objeto FFT */
		/* El número de puntos de la FFT será el de defecto: Siguiente potencia de 2 de np */
		r->_fft = rfft_construct( np, 0, SPL_FALSE, 32768, win, winfunc );
		if (r->_fft == NULL) {
			xfft_free(r);
			r=NULL;
		} else {
			
			r->_nf = nband;
			r->_filt = filter;
			r->_scale = scale;
			r->_srate = fs;
			
			/* Preparamos los punteros para fband_calcfilters */
			r->_findex = NULL;
			r->_fweight = NULL;
			r->_pvec = NULL;
			
			fres = fs/rfft_getfftnp(r->_fft);
			maxfreq = maxfreq == 0 ? fs/2 : maxfreq;	/* Si no hay límite superior, se toma Nyquist */
			
			/* Índice del bin FFT de la minfreq */
			r->_minfreqidx = (SPL_INT) FbandScaleConvert(minfreq, XFBAND_SCALE_LIN, XFBAND_SCALE_BIN, fres);
			//	if (r->_minfreqidx < 1) r->_minfreqidx = 1;
	
			/* Índice del bin FFT de la maxfreq */
			r->_maxfreqidx = (SPL_INT) FbandScaleConvert(maxfreq, XFBAND_SCALE_LIN, XFBAND_SCALE_BIN, fres);
			//	if (r->_maxfreqidx >=  fftnsamples/2) r->_minfreqidx = fftnsamples/2;
			
			/*Calculamos los filtros*/
			if (fband_calcfilters( r ) == SPL_FALSE){
				fband_destruct(r);
				r = NULL;
			}
		}
	}
	
	return r;
}

/*!
   \brief Destructor para una estructura _FBAND

   Destruye una estructura _FBAND. Es necesario llamar a esta función después de haber
   llamado a fband_construct(), una vez se ha terminado de calcular las potencias por banda

   \param r puntero a la estructura que se quiere destruir

   \return -
*/
PUBLIC SPL_VOID XAPI fband_destruct( pFBAND r )
{
	assert(r!=NULL);

	if (r->_fft)
		rfft_destruct(r->_fft);
	if (r->_findex)
		xfft_free(r->_findex);
	if (r->_fweight)
		xfft_free(r->_fweight);
	if (r->_pvec)
		xfft_free(r->_pvec);
	xfft_free(r);
}

/*!
   \brief Modifica el tipo de enventanado

	Modifica el tipo de enventanado a utilizar. Los tipos de ventanas están definidos en
	xfft.h (XFFT_WIN). Si se indica ventana XFFT_WIN_NONE o XFFT_WIN_RECT, se usa ventana
	rectangular. Si se indica XFFT_WIN_USER, entonces se utilizara la ventana obtenida
	a traves de la funcion que el usuario suministra. En xfft.h se explica el
	prototipo que debe seguir esta funcion.

	La ventana se maneja de forma optimizada: aunque se active y
	desactive sucesivamente no se hace ni una cuenta mientras no se
	cambie su longitud o se cambie de tipo de ventana.  

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param win Codigo que identifica el tipo de ventana a utilizar. Estos tipos est�n
			definidos en xfft.h.

   \param winfunc Puntero a una funcion que que suministra el usuario, y que 
			sigue el prototipo indicado (xfft.h). Esta funcion se utiliza para
			obtener la ventana cuando en \p win se envia XFFT_WIN_USER. 
			Si este puntero es NULL y se envia XFFT_WIN_USER en \p win, 
			no se utiliza ningun enventanado (ventana rectangular). 

   \return SPL_TRUE si todo ha ido bien. SPLA_FALSE si se ha producido un error, en cuyo
		caso, la ventana se desconecta y se utiliza ventana rectangular.
*/
PUBLIC SPL_BOOL XAPI fband_setwin( pFBAND r, XFFT_WIN win,
		XFFT_WIN_FUNC winfunc )
{
	assert(r!=NULL);
	
	return rfft_setwin(r->_fft, win, winfunc);
}

/*!
   \brief Fija la longitud de los vectores de entrada

   Permite modificar el número de puntos de los vectores de entrada (aquellos cuya 
   potencia por bandas se quiere calcular).

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param np Número de puntos de los vectores de entrada. Ha de ser mayor que cero,
		pero no forzosamente potencia de dos.

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setnp( pFBAND r, SPL_INT np )
{
	SPL_INT n;

	assert(r!=NULL);
	assert(np>0);
	
	n = r->_fft->_np;
	if (np!=n) {
		if (rfft_setnp(r->_fft, np) == SPL_FALSE)
			return SPL_FALSE;
		if (r->_fft->_np != n) {	/*Tal vez no haya cambiado después de todo*/
			if (fband_calcfilters (r) == SPL_FALSE)
			return SPL_FALSE;
		}
	}
	return SPL_TRUE;
}


/*!
   \brief Fija el número de bandas

   Permite modificar el número de bandas de frecuencia

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param nband Número de bandas. Ha de ser mayor que cero.

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setnband( pFBAND r, SPL_INT nband )
{
	assert(r!=NULL);
	assert(nband>0);
	
	if (nband!=r->_nf) {
		r->_nf = nband;
		return fband_calcfilters (r);
	}
	return SPL_TRUE;
}

/*!
   \brief Cambia el tipo de filtrado

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param filter Forma del filtro que se quiere aplicar

   \param scale Escala en la que calcular los filtros

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setfilters( pFBAND r, XFBAND_FILTER filter, XFBAND_SCALE scale )
{
	assert(r!=NULL);
	assert(scale!=XFBAND_SCALE_BIN);	//No es una escala válida
	
	r->_filt = filter;
	r->_scale = scale;
	return fband_calcfilters (r);
}

/*!
   \brief Cambia los límites de frecuencia

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param minfreq Frecuencia mínima a considerar (eh Hz).

   \param maxfreq Frecuencia máxima a considerar (eh Hz).

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setfreqlimits( pFBAND r, SPL_FLOAT minfreq, SPL_FLOAT maxfreq )
{
	SPL_FLOAT fres;
	SPL_INT oldmin, oldmax;

	assert(r!=NULL);
	assert(minfreq>=0);
	assert(maxfreq>=0);
	
	fres = r->_srate/rfft_getfftnp(r->_fft);
	maxfreq = maxfreq == 0 ? r->_srate/2 : maxfreq;	/* Si no hay límite superior, se toma Nyquist */

	/* Por si acaso los bins no cambian, en cuyo caso nada cambia*/
	oldmin = r->_minfreqidx;
	oldmax = r->_maxfreqidx;
	
	/* Índice del bin FFT de la minfreq */
	r->_minfreqidx = (SPL_INT) FbandScaleConvert(minfreq, XFBAND_SCALE_LIN, XFBAND_SCALE_BIN, fres);
	//	if (r->_minfreqidx < 1) r->_minfreqidx = 1;

	/* Índice del bin FFT de la maxfreq */
	r->_maxfreqidx = (SPL_INT) FbandScaleConvert(maxfreq, XFBAND_SCALE_LIN, XFBAND_SCALE_BIN, fres);
	//	if (r->_maxfreqidx >=  fftnsamples/2) r->_minfreqidx = fftnsamples/2;

	/*Calculamos los filtros sólo si algo cambia*/
	if (oldmin != r->_minfreqidx || oldmax != r->_maxfreqidx)
		return fband_calcfilters (r);
	else
		return SPL_TRUE;
}

/*!
   \brief Cambia el límite inferior de frecuencia

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param minfreq Frecuencia mínima a considerar (eh Hz).

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setminfreq( pFBAND r, SPL_FLOAT minfreq )
{
	assert(r!=NULL);
	assert(minfreq>=0);
	
	return fband_setfreqlimits (r, minfreq, FbandScaleConvert(r->_maxfreqidx, XFBAND_SCALE_BIN, XFBAND_SCALE_LIN, r->_srate/rfft_getfftnp(r->_fft)));
}

/*!
   \brief Cambia el límite superior de frecuencia

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param maxfreq Frecuencia máxima a considerar (eh Hz).

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setmaxfreq( pFBAND r, SPL_FLOAT maxfreq )
{
	assert(r!=NULL);
	assert(maxfreq>=0);
	
	return fband_setfreqlimits (r, FbandScaleConvert(r->_minfreqidx, XFBAND_SCALE_BIN, XFBAND_SCALE_LIN, r->_srate/rfft_getfftnp(r->_fft)), maxfreq);
}


/*!
   \brief Cambia la frecuencia de muestreo de las tramas

   \param r Puntero a la estructura _FBAND que se quiere modificar.

   \param maxfreq Frecuencia de muestreo (eh Hz).

   \return SPL_TRUE si todo ha ido bien. SPL_FALSE si se ha producido un error, en cuyo
		caso posiblemente la clase haya quedado inservible.
*/
PUBLIC SPL_BOOL XAPI fband_setsrate( pFBAND r, SPL_FLOAT srate )
{
	assert(r!=NULL);
	assert(srate>0);

	/*Calculamos los filtros sólo si algo cambia*/
	if (srate == r->_srate) 
		return SPL_TRUE;
	
	r->_srate = srate;
	return fband_calcfilters (r);
}


/*!
   \brief Devuelve el número de puntos de los vectores de entrada

   Permite conocer el número mínimo de puntos que han de tener los vectores de entrada
   (aquellos cuya potencia por bandas se quiere calcular).

   \param r Puntero a la estructura _FBAND.

   \return Número de puntos de los vectores de entrada
*/
PUBLIC SPL_INT XAPI fband_getnp( pFBAND r )
{
	assert(r!=NULL);

	return rfft_getnp(r->_fft);
}

/*!
   \brief Devuelve el número de bandas

   Permite conocer el número de bandas que se están utilizando. Esto coincide
   con el número de puntos del vector de salida

   \param r Puntero a la estructura _FBAND.

   \return Número de bandas
*/
PUBLIC SPL_INT XAPI fband_getnband( pFBAND r )
{
	assert(r!=NULL);

	return (r->_nf);
}


/*!
   \brief Devuelve el tipo de enventanado utilizado

   \param r Puntero a la estructura _FBAND.

   \return Tipo de ventana (los tipos están definidos en xfft.h)
*/
PUBLIC XFFT_WIN XAPI fband_getwin( pFBAND r )
{
	assert(r!=NULL);

	return rfft_getwin(r->_fft);
}

/*!
   \brief Devuelve el vector que contiene la ventana

   Permite acceder al vector que contiene la ventana utilizada. La longitud
   de esta ventana viene dada por fband_getnp().

   \param r Puntero a la estructura _FBAND.

   \return Puntero al vector que contiene la ventana. En caso de que no se utilice
		ventana, se devuelve NULL.
*/
PUBLIC SPL_pFLOAT XAPI fband_getwinvec( pFBAND r )
{
	assert(r!=NULL);

	return rfft_getwinvec(r->_fft);
}

/*!
   \brief Devuelve el puntero a la función de enventanado suministrada por el usuario

   \param r Puntero a la estructura _FBAND.

   \return Puntero a la función de enventanado suministrada por el usuario
*/
PUBLIC XFFT_WIN_FUNC XAPI fband_getwinfunc( pFBAND r )
{
	assert(r!=NULL);

	return rfft_getwinfunc(r->_fft);
}


/*!
   \brief Acceso al vector de salida

   Permite acceder al vector de la potencia por bandas calculada. El usuario debe haber
   calculado una potencia por bandas previamente. Este vector es de uso interno y no debe borrarse, aunque
   se puede modificar.

   El vector devuelto tiene fband_getvecnp() puntos

   \param r Puntero a la estructura _FBAND.

   \return Puntero al vector de potencias por banda
*/
PUBLIC SPL_pFLOAT XAPI fband_getpband( pFBAND r )
{
	assert(r!=NULL);

	return r->_pvec;
}

/*!
   \brief Acceso al límite inferior de frecuencia

   Permite conocer el límite inferior de frecuencia. La potencia por bandas
   se está calculando con fband_getvecnp() bandas situadas entre fband_getminfreq() y 
   fband_getmaxfreq().

   Devuelve la verdadera frecuencia inferior, que puede verse modificada con respecto a la
   indicada en fband_construct() o fband_setfreqlimits() debido a la resolución
   frecuencial de la FFT.

   \param r Puntero a la estructura _FBAND.

   \return Límite inferior de la frecuencia
*/
PUBLIC SPL_FLOAT XAPI fband_getminfreq( pFBAND r )
{
	assert(r!=NULL);

	return FbandScaleConvert(r->_minfreqidx, XFBAND_SCALE_BIN, XFBAND_SCALE_LIN, r->_srate/rfft_getfftnp(r->_fft));
}

/*!
   \brief Acceso al límite superior de frecuencia

   Permite conocer el límite superior de frecuencia. La potencia por bandas
   se está calculando con fband_getvecnp() bandas situadas entre fband_getminfreq() y 
   fband_getmaxfreq().

   Devuelve la verdadera frecuencia superior, que puede verse modificada con respecto a la
   indicada en fband_construct() o fband_setfreqlimits() debido a la resolución
   frecuencial de la FFT.

   \param r Puntero a la estructura _FBAND.

   \return Límite superior de la frecuencia
*/
PUBLIC SPL_FLOAT XAPI fband_getmaxfreq( pFBAND r )
{
	assert(r!=NULL);

	return FbandScaleConvert(r->_maxfreqidx, XFBAND_SCALE_BIN, XFBAND_SCALE_LIN, r->_srate/rfft_getfftnp(r->_fft));
}


/*!
   \brief Acceso a la frecuencia de muestreo de las secuencias

   \param r Puntero a la estructura _FBAND.

   \return Frecuencia de muestreo en Hz
*/
PUBLIC SPL_FLOAT XAPI fband_getsrate( pFBAND r )
{
	assert(r!=NULL);

	return r->_srate;
}


/*!
   \brief uso interno
*/
/* Realiza el filtrado por bandas. Supone que la FFT ya está calculada
   y en formato módulo/argumento
*/
PUBLIC SPL_VOID XAPI fband_dofband( pFBAND r )
{
	SPL_INT i;
	SPL_FLOAT* fftMod;

	assert(r!=NULL);
	
	/*Recuperamos el módulo de la FFT*/
	fftMod = rfft_getrevec(r->_fft);
	
	/*Borramos todo el contenido del vector de salida*/
	for (i = 0; i < r->_nf; ++i)
		r->_pvec[i] = 0;

	/*Recorremos la FFT, metiendo su aportación dentro de cada filtro
	  Escalamos al número de bins (para tener potencia independientemente de la Fs)*/
	for (i = r->_minfreqidx; i <= r->_maxfreqidx; ++i)
	{
		/*El primer y último filtro son especiales*/
		if (r->_findex[i] != r->_nf)
			/*Para filtros rectangulares ya nos hemos asegurado de que no sea mayor que r->nf-1
			  luego siempre entraremos aquí.*/
			r->_pvec[r->_findex[i]] += r->_fweight[i]*fftMod[i];

		if (r->_findex[i] != 0)
			/*Gracias a que en los filtros rectangulares el peso siempre es 1/fftNsamples,
			  el método de calcular la aportación al filtro anterior es consistente para
			  los filtros triangulares y rectangulares.*/
			r->_pvec[r->_findex[i]-1] += (1.0/rfft_getfftnp(r->_fft) - r->_fweight[i])*fftMod[i];
	}

	/*Aplicamos el log*/
	for (i = 0; i < r->_nf; ++i)
		r->_pvec[i] = 20*log10(r->_pvec[i]);	/*20 para que sea potencia*/
}


/*!
   \brief Calcula la potencia por bandas de una secuencia

   Calcula la potencia por bandas de una secuencia real de valores flotantes.
   Esta potencia por bandas puede recuperarse mediante fband_getpband()

	Previamente se ha tenido que construir la estructura FBAND mediante fband_construct().

   \param r Puntero a la estructura _FBAND.

   \param invec Vector que contiene la secuencia de valores flotantes cuya potencia
		por bandas se quiere calcular. Debe tener fband_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI fband_fband( pFBAND r, SPL_pFLOAT invec )
{
	assert(r!=NULL);
	
	/*Calculamos su FFT en formato mod-arg*/
	rfft_rfft(r->_fft, invec);
	rfft_magarg(r->_fft);
	
	fband_dofband(r);
}


/*!
   \brief Calcula la potencia por bandas de una secuencia real de valores enteros

   \param r Puntero a la estructura _FBAND

   \param invec Vector que contiene la secuencia de valores enteros.
		Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI fband_fband_i( pFBAND r, SPL_pINT invec )
{
	assert(r!=NULL);

	/*Calculamos su FFT en formato mod-arg*/
	rfft_rfft_i(r->_fft, invec);
	rfft_magarg(r->_fft);
	
	fband_dofband(r);
}

/*!
   \brief Calcula la potencia por bandas de una secuencia real de valores INT16

   \param r Puntero a la estructura _FBAND

   \param invec Vector que contiene la secuencia de valores enteros.
		Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI fband_fband_i16( pFBAND r, pINT16 invec )
{
	assert(r!=NULL);

	/*Calculamos su FFT en formato mod-arg*/
	rfft_rfft_i16(r->_fft, invec);
	rfft_magarg(r->_fft);
	
	fband_dofband(r);
}


/*!
   \brief Calcula la potencia por bandas de una secuencia real de valores INT32

   \param r Puntero a la estructura _FBAND

   \param invec Vector que contiene la secuencia de valores enteros.
		Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI fband_fband_i32( pFBAND r, pINT32 invec )
{
	assert(r!=NULL);

	/*Calculamos su FFT en formato mod-arg*/
	rfft_rfft_i32(r->_fft, invec);
	rfft_magarg(r->_fft);
	
	fband_dofband(r);
}

/*!
   \brief Calcula la potencia por bandas de una secuencia real de valores UINT32

   \param r Puntero a la estructura _FBAND

   \param invec Vector que contiene la secuencia de valores enteros.
		Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI fband_fband_u32( pFBAND r, pUINT32 invec )
{
	assert(r!=NULL);

	/*Calculamos su FFT en formato mod-arg*/
	rfft_rfft_u32(r->_fft, invec);
	rfft_magarg(r->_fft);
	
	fband_dofband(r);
}

/*!
   \brief Calcula la potencia por bandas de una secuencia real de valores FLOAT32

   \param r Puntero a la estructura _FBAND

   \param invec Vector que contiene la secuencia de valores enteros.
		Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI fband_fband_f32( pFBAND r, pFLOAT32 invec )
{
	assert(r!=NULL);

	/*Calculamos su FFT en formato mod-arg*/
	rfft_rfft_f32(r->_fft, invec);
	rfft_magarg(r->_fft);
	
	fband_dofband(r);
}


/*!
	\brief Función para el cálculo de coeficientes MFCC a partir de FBAND

	Esta función aplica una DCT discreta de \p nceps puntos al vector de
	potencia por bandas contenido en \p r, lo que proporciona coeficientes
	mel cepstrum.

	\param r Puntero a la estructura _FBAND con la potencia por bandas.
		La potencia por bandas ya debe estar calculada (se debe haber llamado
		a fband_fband).

	\param mfcc Puntero al buffer que va a recoger los coeficientes MFCC. Debe
		tener \p nceps puntos

	\param nceps Número de coeficientes a calcular. Es igual al orden de la DCT
		a aplicar.

	\param lift Coeficiente de liftering. 0 para no aplicar liftering
*/
PUBLIC SPL_VOID API fband2mfcc (pFBAND r, SPL_pFLOAT mfcc, SPL_INT nceps, SPL_INT lift)
{
	SPL_FLOAT c, cc;
	SPL_pFLOAT fb;
	SPL_INT j, k;

	assert(r!=NULL);
	assert(nceps>0);

	c = sqrt(2.0/fband_getnband(r));
	for (j = 0; j < nceps; ++j)
	{
		mfcc[j] = 0;
		cc = M_PI*(j+1)/fband_getnband(r);
		fb = fband_getpband(r);
		for (k = 0; k < fband_getnband(r); ++k)
			mfcc[j] += fb[k]*cos(cc*(k+0.5));
	
			//Aplicamos el liftering, si se pide
		if (lift != 0)
			mfcc[j] *= c*(1+lift/2*sin(M_PI*(j+1)/lift));
		else
			mfcc[j] *= c;
	}
}


/**********************************************************/


