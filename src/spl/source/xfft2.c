/////////////////////////////////////////////////////////////////////
/*!
   \file xfft2.c
   \brief C�lculo de FFT de secuencias reales

	Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao
	\author Borja Etxebarria
	\version 1.4.0
	\date 02/01/97

	\if SKIP
	Version  dd/mm/aa  Autor     Proposito de la edicion
	-------  --------  --------  -----------------------
	1.4.0    02/01/97  Borja     nuevas funcs: getwinvec(), set/get0piweight
	1.3.3    27/08/95  Borja     SPL_FLOAT32 --> FLOAT32
	1.3.2    30/07/95  Borja     scope funciones explicito
	1.3.1    15/05/95  Borja     retoques en algunos comentarios.
	1.3.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
	1.2.1    14/03/94  Borja     bug: cepfm_getceps() --> cepfm_getcepfm()
	1.2.0    28/10/93  Borja     Soporte de diferentes tipos de enventanado.
	1.1.0    01/02/93  Borja     Calculo de cepstrum fft-modulo ('clase' cepfm).
	1.0.0    16/03/92  Borja     Codificacion inicial.
	\endif

	Funciones para el calculo eficiente y reiterado de FFT real.

*/

#include "_xfft.h"

/*!
   \brief Inicializaci�n de _RFFT
   
	Inicializa una estructura _RFFT para el c�lculo de FFT de secuencias reales

   \param np N�mero de puntos del vector de entrada (del que se quiere calcular la FFT).
			Ha de ser mayor que cero, pero no forzosamente potencia de dos.
			En caso de que no sea potencia de dos, se rellena autom�ticamente con ceros 

   \param fftnp N�mero de puntos de la FFT. Ha de ser mayor o igual a cero\n
			Si es cero, se utiliza la menor potencia de dos superior o igual a \p np.\n
			Si no es cero, se utiliza la menor potencia de dos superior o igual a \p fftnp.\n
			Si \p np es mayor que \p fftnp, el vector de datos se trunca a \p fftnp valores.

   \param inv Indica si se desea calcular la transformada directa o inversa.
			\li \p SPL_FALSE o \p FFT_DIRECT: FFT directa
			\li \p SPL_TRUE o \p FFT_INVERSE: FFT inversa

   \param ufactor Factor de escalado. Multiplica al resultado en la FFT directa y
			lo divide en la FFT inversa. Por lo general, se deja a 1 (sin escalado).
			Si vale 0, tiene un uso especial: la FFT directa se escala
			de forma que el modulo de la tranformada encaja con las amplitudes
			de las componentes (lo que haria un equipo analizador de espectro);
			en tal caso, 0 y pi estan realmente escaladas por dos (por interaccion 
			entre espectro positivo y negativo. Si se hace rfft_set0piweight(TRUE)
			esto se corrige y 0 y pi se deescalan. Es una simplificacion, realmente
			habria que deescalar todos los alrededores siguiendo la respuesta
			de la ventana de analisis.

   \param win Codigo que identifica el tipo de ventana a utilizar. Estos tipos est�n
			definidos en xfft.h (XFFT_WIN). Si se utiliza una ventana, sera de \p np puntos
			aunque si el numero de puntos de la fft es menor, se utiliza este otro valor.

   \param winfunc Puntero a una funcion que que suministra el usuario, y que 
			sigue el prototipo indicado (xfft.h). Esta funcion se utiliza para
			obtener la ventana cuando en \p win se envia XFFT_WIN_USER. 
			Si este puntero es NULL y se envia XFFT_WIN_USER en \p win, 
			no se utiliza ningun enventanado (ventana rectangular). 

   \return Puntero a una estructura _FFT. Este puntero deber� utilizarse como par�metro 
			para el resto de las funciones \p fft_???(). Devuelve \p NULL si la inicializaci�n falla
*/
PUBLIC pRFFT XAPI rfft_construct( SPL_INT np, SPL_INT fftnp,
		SPL_BOOL inv, SPL_FLOAT ufactor,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc )
{
/*
Aunque externamente parezca que el sistema utiliza {fftnp} puntos para
la FFT, realmente aunque a nivel interno, se utiliza la mitad por
tratarse de FFT de un vector real. Asi es mucho mas eficiente.
Externamente esto pasa COMPLETAMENTE desapercibido.
*/
	pRFFT r;
	SPL_INT n, n2, n3;
	assert(np>0);
	assert(fftnp>=0);

	r = (pRFFT)xfft_malloc(sizeof(RFFT));
	if (r!=NULL) {
		n = fft_n_bits((fftnp>0) ? fftnp : np);
		r->_nbits = ((n) ? n-1 : n);
		n2 = ( (n = (SPL_INT)1 << r->_nbits) >> 1 );
		n3 = n<<1;
		r->_np = ( (np>n3) ? n3 : np );

		r->_revec = (SPL_pFLOAT)xfft_malloc(
				sizeof(SPL_FLOAT)*(n+1));
		r->_imvec = (SPL_pFLOAT)xfft_malloc(
				sizeof(SPL_FLOAT)*(n+1));
		r->_tinv = (SPL_pINT)xfft_malloc(sizeof(SPL_INT)*n);
		r->_half_tsin = (SPL_pFLOAT)xfft_malloc(
				sizeof(SPL_FLOAT)*n2);
		r->_tsin = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n2);
		r->_tcos = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n2);
		if ((win!=XFFT_WIN_NONE)&&(r->_np))
			r->_winvec = (SPL_pFLOAT)xfft_malloc(
					sizeof(SPL_FLOAT)*r->_np);
		else
			r->_winvec = NULL;

		if ( ((r->_revec==NULL)||(r->_imvec==NULL)||
				((n)&&(r->_tinv==NULL))||
				((n2)&&((r->_half_tsin==NULL)||
				(r->_tsin==NULL)||(r->_tcos==NULL)))||
				((win!=XFFT_WIN_NONE)&&(r->_np)&&
				(r->_winvec==NULL))) == 0 ) {

			if (n2) {
				fft_fill_half_tsin(r->_half_tsin,n2);
				fft_htsin_fill_tsin_tcos(r->_tsin,
						r->_tcos,r->_half_tsin,n2);
			}
			fft_fill_tinv(r->_tinv,n);
			r->_inv = inv;
			r->_ufactor = ufactor;
			r->_weight0pi=FALSE;

			r->_owin = XFFT_WIN_NONE;
			rfft_setwin(r,win,winfunc);
		}
		else {
			rfft_destruct(r);
			r=NULL;
		}
	}

	return r;
}

/*!
   \brief Destructor para una estructura _RFFT

   Destruye una estructura _RFFT. Es necesario llamar a esta funci�n despu�s de haber
   llamado a rfft_construct(), una vez se ha terminado de calcular FFT's

   \param r puntero a la estructura que se quiere destruir

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_destruct( pRFFT r )
{
	assert(r!=NULL);

	if (r->_revec)
		xfft_free(r->_revec);
	if (r->_imvec)
		xfft_free(r->_imvec);
	if (r->_tinv)
		xfft_free(r->_tinv);
	if (r->_half_tsin)
		xfft_free(r->_half_tsin);
	if (r->_tsin)
		xfft_free(r->_tsin);
	if (r->_tcos)
		xfft_free(r->_tcos);
	if (r->_winvec)
		xfft_free(r->_winvec);
	xfft_free(r);
}

/*!
   \brief Modifica el tipo de enventanado

	Modifica el tipo de enventanado a utilizar. Los tipos de ventanas est�n definidos en
	xfft.h (XFFT_WIN). Si se indica ventana XFFT_WIN_NONE o XFFT_WIN_RECT, se usa ventana
	rectangular. Si se indica XFFT_WIN_USER, entonces se utilizara la ventana obtenida
	a traves de la funcion que el usuario suministra. En xfft.h se explica el
	prototipo que debe seguir esta funcion.

	La ventana se maneja de forma optimizada: aunque se active y
	desactive sucesivamente no se hace ni una cuenta mientras no se
	cambie su longitud o se cambie de tipo de ventana.  

   \param r Puntero a la estructura _RFFT que se quiere modificar.

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
PUBLIC SPL_BOOL XAPI rfft_setwin( pRFFT r, XFFT_WIN win,
		XFFT_WIN_FUNC winfunc )
{
	INT i;
	assert(r!=NULL);

	r->_win = win;
	r->_winfunc=winfunc;
	r->_ufactor0 = (r->_np) ? 2./r->_np : 1;

	if ((win!=XFFT_WIN_NONE)&&(r->_np)) {
		if (r->_winvec==NULL) {
			r->_owin=XFFT_WIN_NONE;
			r->_winvec = (SPL_pFLOAT)xfft_malloc(
					sizeof(SPL_FLOAT)*r->_np);
		}
		if (r->_winvec!=NULL) {
			if (r->_owin!=r->_win) {
				switch (r->_win) {
				case XFFT_WIN_HAMM :
					win_hamm(r->_winvec,r->_np);
					break;
				case XFFT_WIN_HANN :
					win_hann(r->_winvec,r->_np);
					break;
				case XFFT_WIN_BLACK :
					win_black(r->_winvec,r->_np);
					break;
				case XFFT_WIN_BART :
					win_bart(r->_winvec,r->_np);
					break;
				case XFFT_WIN_KAIS5 :
					win_kais(r->_winvec,r->_np,5.0);
					break;
				case XFFT_WIN_KAIS6 :
					win_kais(r->_winvec,r->_np,6.0);
					break;
				case XFFT_WIN_KAIS7 :
					win_kais(r->_winvec,r->_np,7.0);
					break;
				case XFFT_WIN_KAIS8 :
					win_kais(r->_winvec,r->_np,8.0);
					break;
				case XFFT_WIN_KAIS9 :
					win_kais(r->_winvec,r->_np,9.0);
					break;
				case XFFT_WIN_KAIS10 :
					win_kais(r->_winvec,r->_np,10.0);
					break;
				case XFFT_WIN_USER :
					if ((r->_winfunc)!=NULL)
						(r->_winfunc)(r->_winvec,
								r->_np);
					else
						r->_win=XFFT_WIN_NONE;
					break;
				default :
					/* error: ventana desconocida */
					assert(1==0); /* trigger */
					r->_win=XFFT_WIN_NONE;
					break;
				}
				r->_owin=r->_win;
				if (r->_win!=XFFT_WIN_NONE) {
					r->_winmean = 0;
					for (i=0; i<r->_np; i++) r->_winmean += r->_winvec[i];
					r->_winmean /= r->_np;
				}
				else
					r->_winmean = 1;
			}
			r->_ufactor0 /= r->_winmean;
		}
		else {
			r->_win=XFFT_WIN_NONE;
			return SPL_FALSE;
		}
	}

	return SPL_TRUE;
}

/*!
   \brief Fija la longitud de los vectores de entrada

   Permite modificar el n�mero de puntos de los vectores de entrada (aquellos cuya FFT
   se quiere calcular).

   \param r Puntero a la estructura _RFFT que se quiere modificar.

   \param np N�mero de puntos de los vectores de entrada. Ha de ser mayor que cero,
		pero no forzosamente potencia de dos. En caso de que \p np sea mayor que el
		n�mero de puntos de la FFT, los vectores de entrada se truncan. En caso de
		que \p np sea menor que el n�mero de puntos de la FFT, se rellena con ceros

   \return SPL_TRUE si todo ha ido bien. SPLA_FALSE si se ha producido un error, en cuyo
		caso, la ventana se desconecta y se utiliza ventana rectangular.
*/
PUBLIC SPL_BOOL XAPI rfft_setnp( pRFFT r, SPL_INT np )
{
	SPL_INT n;
	assert(r!=NULL);
	assert(np>0);

	n = ((SPL_INT)1 << (r->_nbits+1));
	if (np>n)
		np = n;
	if (np!=r->_np) {
		r->_np = np;
		if (r->_winvec!=NULL) {
			xfft_free(r->_winvec);
			r->_winvec = NULL;
		}
		return rfft_setwin(r,r->_win,r->_winfunc);
	}
	return SPL_TRUE;
}

/*!
   \brief Modifica el tipo de enventanado y la longitud de los vectores de entrada a la vez

	Modifica el tipo de enventanado y la longitud de los vectores de entrada de forma
	simult�nea. Los tipos de ventanas est�n definidos en
	xfft.h XFFT_WIN. Si se indica ventana XFFT_WIN_NONE o XFFT_WIN_RECT, se usa ventana
	rectangular. Si se indica XFFT_WIN_USER, entonces se utilizara la ventana obtenida
	a traves de la funcion que el usuario suministra. En xfft.h se explica el
	prototipo que debe seguir esta funcion.

	La ventana se maneja de forma optimizada: aunque se active y
	desactive sucesivamente no se hace ni una cuenta mientras no se
	cambie su longitud o se cambie de tipo de ventana.  

   \param r Puntero a la estructura _RFFT que se quiere modificar.

   \param np N�mero de puntos de los vectores de entrada. Ha de ser mayor que cero,
		pero no forzosamente potencia de dos. En caso de que \p np sea mayor que el
		n�mero de puntos de la FFT, los vectores de entrada se truncan. En caso de
		que \p np sea menor que el n�mero de puntos de la FFT, se rellena con ceros

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
PUBLIC SPL_BOOL XAPI rfft_setnpwin( pRFFT r, SPL_INT np,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc )
{
	assert(r!=NULL);
	assert(np>0);

	r->_win = XFFT_WIN_NONE;
	rfft_setnp(r,np);
	return rfft_setwin(r,win,winfunc);
}

/*!
   \brief Fija el c�lculo a FFT directa o inversa

   Permite indicar si se quiere calcular una FFT directa o inversa

   \param r Puntero a la estructura _RFFT que se quiere modificar.

   \param inv: Indica si se desea calcular la transformada directa o inversa.
			\li \p SPL_FALSE o \p FFT_DIRECT: FFT directa
			\li \p SPL_TRUE o \p FFT_INVERSE: FFT inversa

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_setinv( pRFFT r, SPL_BOOL inv )
{
	assert(r!=NULL);

	r->_inv = inv;
}

/*!
   \brief Fija el factor de escala

   Permite cambiar el factor de escala aplicado a la FFT. Este factor multiplica 
		al resultado en la FFT directa y lo divide en la FFT inversa

   \param r Puntero a la estructura _RFFT que se quiere modificar.

   \param ufactor: Factor de escalado.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_setufactor( pRFFT r, SPL_FLOAT ufactor )
{
	assert(r!=NULL);

	r->_ufactor = ufactor;
}

/*!
   \brief Configura el factor de ponderaci�n en 0 y PI

   escala por 1/2 en la fft directa, por 2 en la inversa. Para que
   funcione el factor de usuario \p ufactor debe ser 0.

   \param r Puntero a la estructura _RFFT que se quiere modificar.

   \param weight booleano para activar o desactivar el factor de ponderaci�n

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_set0piweight( pRFFT r, SPL_BOOL weight )
{
	assert(r!=NULL);

	r->_weight0pi=weight;
}

/*!
   \brief Devuelve el estado del factor de ponderaci�n

   Devuelve el estado del factor de ponderaci�n. Realmente este factor s�lo estar�
   activado si el factor de usuario \p ufactor es cero

   \param r Puntero a la estructura _RFFT que se quiere modificar.

   \return SPL_TRUE si el factor de ponderaci�n est� activado. SPL_FALSE si no.
*/
PUBLIC SPL_BOOL XAPI rfft_get0piweight( pRFFT r )
{
	assert(r!=NULL);

	return r->_weight0pi;
}

/*!
   \brief Devuelve el n�mero de puntos de los vectores de entrada

   Permite conocer el n�mero m�nimo de puntos que han de tener los vectores de entrada
   (aquellos cuya FFT se quiere calcular).

   \param r Puntero a la estructura _RFFT.

   \return N�mero de puntos de los vectores de entrada
*/
PUBLIC SPL_INT XAPI rfft_getnp( pRFFT r )
{
	assert(r!=NULL);

	return r->_np;
}

/*!
   \brief Devuelve el n�mero de puntos de la FFT

   Permite conocer el n�mero de puntos que tienen las FFT calculadas. Devuelve
   el verdadero n�mero de puntos utilizado (siempre potencia de dos)

   \param r Puntero a la estructura _RFFT.

   \return N�mero de puntos de la FFT
*/

/*
Realmente se van a usar la mitad de los indicados, ya que se hace uso de las
propiedades de simetr�a de una FFT real.
*/
PUBLIC SPL_INT XAPI rfft_getfftnp( pRFFT r )
{
	assert(r!=NULL);

	return ((SPL_INT)1 << (r->_nbits+1));
}

/*!
   \brief Devuelve el n�mero de puntos de la FFT

   Devuelve el n�mero de puntos de los vectores de salida. 
   Puesto que una FFT real es sim�trica en la parte real y antisim�trica
   en la parte imaginaria, los vectores de resultado s�lo tienen la mitad
   de puntos de la FFT. Es decir: rfft_getvecnp() = rfft_getfftnp()/2 + 1.

   Por tanto, los vectores resultado abarcan el rango [0, PI] (en herzios, 
   [0, fs/2]).

   \param r Puntero a la estructura _RFFT.

   \return N�mero de puntos de los vectores
*/
PUBLIC SPL_INT XAPI rfft_getvecnp( pRFFT r )
{
	assert(r!=NULL);

	return ((SPL_INT)1 << r->_nbits)+1;
}

/*!
   \brief Devuelve el tipo de enventanado utilizado

   \param r Puntero a la estructura _RFFT.

   \return Tipo de ventana (los tipos est�n definidos en xfft.h)
*/
PUBLIC XFFT_WIN XAPI rfft_getwin( pRFFT r )
{
	assert(r!=NULL);

	return r->_win;
}

/*!
   \brief Devuelve el vector que contiene la ventana

   Permite acceder al vector que contiene la ventana utilizada. La longitud
   de esta ventana viene dada por rfft_getnp().

   \param r Puntero a la estructura _RFFT.

   \return Puntero al vector que contiene la ventana. En caso de que no se utilice
		ventana, se devuelve NULL.
*/
PUBLIC SPL_pFLOAT XAPI rfft_getwinvec( pRFFT r )
{
	assert(r!=NULL);

	if (r->_win==XFFT_WIN_NONE)
		return NULL;
	return r->_winvec;
}

/*!
   \brief Devuelve el puntero a la funci�n de enventanado suministrada por el usuario

   \param r Puntero a la estructura _RFFT.

   \return Puntero a la funci�n de enventanado suministrada por el usuario
*/
PUBLIC XFFT_WIN_FUNC XAPI rfft_getwinfunc( pRFFT r )
{
	assert(r!=NULL);

	return r->_winfunc;
}

/*!
   \brief Devuelve si se est� calculando una FFT directa o inversa

   \param r Puntero a la estructura _RFFT.

   \return
			\li \p SPL_FALSE o \p FFT_DIRECT: FFT directa
			\li \p SPL_TRUE o \p FFT_INVERSE: FFT inversa
*/
PUBLIC SPL_BOOL XAPI rfft_getinv( pRFFT r )
{
	assert(r!=NULL);

	return r->_inv;
}

/*!
   \brief Devuelve el factor de escala aplicado

   \param r Puntero a la estructura _RFFT.

   \return El factor de escala aplicado
*/
PUBLIC SPL_FLOAT XAPI rfft_getufactor( pRFFT r )
{
	assert(r!=NULL);

	return r->_ufactor;
}

/*!
   \brief Acceso a la parte real de la FFT

   Permite acceder al vector de la parte real de la FFT calculada. El usuario debe haber
   calculado una FFT previamente. Este vector es de uso interno y no debe borrarse, aunque
   se puede modificar.

   En caso de que se calcule el m�dulo o la norma (m�dulo al cuadrado) de la FFT, estos
   valores se almacenan en este mismo vector.

   El vector devuelto tiene rfft_getvecnp() puntos, 
   y sus valores se corresponden al rango [0, PI] (en herzios [0, fs/2])

   \param r Puntero a la estructura _RFFT.

   \return Puntero al inicio del vector de la parte real de la FFT.
*/
PUBLIC SPL_pFLOAT XAPI rfft_getrevec( pRFFT r )
{
	assert(r!=NULL);

	return r->_revec;
}

/*!
   \brief Acceso a la parte imaginaria de la FFT

   Permite acceder al vector de la parte imaginaria de la FFT calculada. El usuario debe haber
   calculado una FFT previamente. Este vector es de uso interno y no debe borrarse, aunque
   se puede modificar.

   En caso de que se calcule la fase de la FFT, estos
   valores se almacenan en este mismo vector.

   El vector devuelto tiene rfft_getvecnp() puntos, 
   y sus valores se corresponden al rango [0, PI] (en herzios [0, fs/2])

   \param r Puntero a la estructura _RFFT.

   \return Puntero al inicio del vector de la parte imaginaria de la FFT.
*/
PUBLIC SPL_pFLOAT XAPI rfft_getimvec( pRFFT r )
{
	assert(r!=NULL);

	return r->_imvec;
}

/*!
   \brief Uso interno
*/
PRIVATE SPL_VOID rfft_dofft( pRFFT r )
{
	SPL_INT n;
	SPL_FLOAT factor;
	assert(r!=NULL);

	factor = r->_ufactor ? r->_ufactor : r->_ufactor0;
	n = ((SPL_INT)1 << r->_nbits);

	/* correccion 0-PI */
	if ((r->_weight0pi) && (r->_inv!=FALSE) && (r->_ufactor==0)) {
		r->_revec[r->_tinv[0]] *= 2;
		r->_revec[r->_tinv[(SPL_INT)1 << (r->_nbits-1)]] *= 2;
		/* la secuencia de puntos es: re[tinv[0]] im[tinv[0]]
		re[tinv[1]] im[tinv[1]].... El punto PI es re[tinv[1<<(nbits-1)] */
	}

	fft_fft( r->_nbits,r->_inv,r->_revec,r->_imvec,r->_tsin,r->_tcos,
			((r->_inv)?factor*2.0:factor));
	fft_unscramble_cx_out(r->_revec,r->_imvec,n,r->_half_tsin,r->_inv);
	r->_revec[n] = r->_imvec[0];
	r->_imvec[0] = r->_imvec[n] = 0.0;

	/* correccion 0-PI */
	if ((r->_weight0pi) && (r->_inv==FALSE) && (r->_ufactor==0)) {
		r->_revec[0] *= 0.5;
		r->_revec[n] *= 0.5;
	}
}

/*!
   \brief Macro de uso interno
*/
#define __rfft_rfft(r,invec)  \
{  \
	SPL_INT i, n2;  \
	SPL_pINT tinv;  \
	SPL_pFLOAT winv;  \
	SPL_INT n = ((SPL_INT)1 << r->_nbits);  \
	\
	/*Scrambling & bit-inversion:*/  \
	n2 = (r->_np) >> 1;  \
	tinv = r->_tinv;  \
	winv = r->_winvec;  \
	if (r->_win!=XFFT_WIN_NONE) {  \
		for (i=0; i<n2; i++) {  \
			r->_revec[*tinv] = (*(invec++))*(*(winv++));  \
			r->_imvec[*(tinv++)] = (*(invec++))*(*(winv++));  \
		}  \
		/* np impar */  \
		if ((r->_np)&1) {  \
			r->_revec[*tinv] = (*invec)*(*winv);  \
			r->_imvec[*(tinv++)] = 0.0;  \
			n2++;  \
		}  \
	}  \
	else {  \
		for (i=0; i<n2; i++) {  \
			r->_revec[*tinv] = (*(invec++));  \
			r->_imvec[*(tinv++)] = (*(invec++));  \
		}  \
		/* np impar */ \
		if ((r->_np)&1) {  \
			r->_revec[*tinv] = (*invec);  \
			r->_imvec[*(tinv++)] = 0.0;  \
			n2++;  \
		}  \
	}  \
	for (i=n2; i<n; i++) {  \
		r->_revec[*tinv] = 0.0;  \
		r->_imvec[*(tinv++)] = 0.0;  \
	}  \
	rfft_dofft(r);  \
}

/*!
   \brief Calcula la FFT de una secuencia real

   Calcula la FFT de una secuencia real de valores flotantes. La parte real de esta FFT puede
   recuperarse mediante rfft_getrvec(), y la parte imaginaria mediante rfft_getimvec().

   \param r Puntero a la estructura _RFFT.

   \param invec Vector que contiene la secuencia de valores flotantes cuya FFT
		se quiere calcular. Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_rfft( pRFFT r, SPL_pFLOAT invec )
{
	assert(r!=NULL);

	__rfft_rfft(r,invec);
}

/*!
   \brief Calcula la FFT de una secuencia real

   Calcula la FFT de una secuencia real de valores enteros. La parte real de esta FFT puede
   recuperarse mediante rfft_getrvec(), y la parte imaginaria mediante rfft_getimvec().

   \param r Puntero a la estructura _RFFT.

   \param invec Vector que contiene la secuencia de valores enteros cuya FFT
		se quiere calcular. Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_rfft_i( pRFFT r, SPL_pINT invec )
{
	assert(r!=NULL);

	__rfft_rfft(r,invec);
}

/*!
   \brief Calcula la FFT de una secuencia real

   Calcula la FFT de una secuencia real de valores INT16. La parte real de esta FFT puede
   recuperarse mediante rfft_getrvec(), y la parte imaginaria mediante rfft_getimvec().

   \param r Puntero a la estructura _RFFT.

   \param invec Vector que contiene la secuencia de valores INT16 cuya FFT
		se quiere calcular. Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_rfft_i16( pRFFT r, pINT16 invec )
{
	assert(r!=NULL);

	__rfft_rfft(r,invec);
}


/*!
   \brief Calcula la FFT de una secuencia real

   Calcula la FFT de una secuencia real de valores INT32. La parte real de esta FFT puede
   recuperarse mediante rfft_getrvec(), y la parte imaginaria mediante rfft_getimvec().

   \param r Puntero a la estructura _RFFT.

   \param invec Vector que contiene la secuencia de valores INT32 cuya FFT
		se quiere calcular. Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_rfft_i32( pRFFT r, pINT32 invec )
{
	assert(r!=NULL);

	__rfft_rfft(r,invec);
}

/*!
   \brief Calcula la FFT de una secuencia real

   Calcula la FFT de una secuencia real de valores UINT32. La parte real de esta FFT puede
   recuperarse mediante rfft_getrvec(), y la parte imaginaria mediante rfft_getimvec().

   \param r Puntero a la estructura _RFFT.

   \param invec Vector que contiene la secuencia de valores UINT32 cuya FFT
		se quiere calcular. Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_rfft_u32( pRFFT r, pUINT32 invec )
{
	assert(r!=NULL);

	__rfft_rfft(r,invec);
}

/*!
   \brief Calcula la FFT de una secuencia real

   Calcula la FFT de una secuencia real de valores FLOAT32 flotantes. La parte real de esta FFT puede
   recuperarse mediante rfft_getrvec(), y la parte imaginaria mediante rfft_getimvec().

   \param r Puntero a la estructura _RFFT.

   \param invec Vector que contiene la secuencia de valores FLOAT32 cuya FFT
		se quiere calcular. Debe tener rfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_rfft_f32( pRFFT r, pFLOAT32 invec )
{
	assert(r!=NULL);

	__rfft_rfft(r,invec);
}

/*!
   \brief Calcula el m�dulo de una FFT ya calculada

   Una vez calculada una FFT, permite extraer el m�dulo de la misma. El resultado
   se introduce en el vector de la parte real de la FFT, de forma que posteriormente es
   accesible mediante rfft_getrevec(). 
   
   \note Una vez llamada esta funci�n, se pierde la parte real. Por lo tanto, 
		ya no se puede recuperar esta parte real ni se puede calcular la fase.
   
   \param r Puntero a la estructura _RFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_mag( pRFFT r )
{
	SPL_INT i, n;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++)
		r->_revec[i] = __fft_cx_mag(r->_revec[i],r->_imvec[i]);
}

/*!
   \brief Calcula la norma de una FFT ya calculada

   Una vez calculada una FFT, permite extraer la norma (m�dulo al cuadrado) de la misma.
   El resultado se introduce en el vector de la parte real de la FFT, de forma que
   posteriormente es accesible mediante rfft_getrevec(). 
   
   \note Una vez llamada esta funci�n, se pierde la parte real. Por lo tanto, 
		ya no se puede recuperar esta parte real ni se puede calcular la fase.
   
   \param r Puntero a la estructura _RFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_norm( pRFFT r )
{
	SPL_INT i, n;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++)
		r->_revec[i] = __fft_cx_norm(r->_revec[i],r->_imvec[i]);
}

/*!
   \brief Calcula la fase de una FFT ya calculada

   Una vez calculada una FFT, permite extraer la fase de la misma.
   El resultado se introduce en el vector de la parte imaginaria de la FFT, de forma que
   posteriormente es accesible mediante rfft_getimvec(). 
   
   \note Una vez llamada esta funci�n, se pierde la parte imaginaria. Por lo tanto, 
		ya no se puede recuperar esta parte imaginaria ni se puede calcular el m�dulo.
   
   \param r Puntero a la estructura _RFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_arg( pRFFT r )
{
	SPL_INT i, n;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++)
		r->_imvec[i] = fft_zcx_arg(r->_revec[i],r->_imvec[i]);
}

/*!
   \brief Calcula el m�dulo y la fase de una FFT ya calculada

   Una vez calculada una FFT, permite extraer el m�dulo y la fase de la misma.
   El m�dulo se introduce en el vector de la parte real de la FFT, y la fase en
   el vector de la parte imaginaria, de forma que posteriormente son accesibles
   mediante rfft_getrevec() y rfft_getimvec() respectivamente. 
   
   \param r Puntero a la estructura _RFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_magarg( pRFFT r )
{
	SPL_INT i, n;
	SPL_FLOAT temp;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++) {
		temp = r->_revec[i];
		r->_revec[i] = __fft_cx_mag(temp,r->_imvec[i]);
		r->_imvec[i] = fft_zcx_arg(temp,r->_imvec[i]);
	}
}

/*!
   \brief Calcula la norma y la fase de una FFT ya calculada

   Una vez calculada una FFT, permite extraer la norma (m�dulo al cuadrado) y la fase
   de la misma. La norma se introduce en el vector de la parte real de la FFT,
   y la fase en el vector de la parte imaginaria, de forma que posteriormente son
   accesibles mediante rfft_getrevec() y rfft_getimvec() respectivamente. 
   
   \param r Puntero a la estructura _RFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_normarg( pRFFT r )
{
	SPL_INT i, n;
	SPL_FLOAT temp;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++) {
		temp = r->_revec[i];
		r->_revec[i] = __fft_cx_norm(temp,r->_imvec[i]);
		r->_imvec[i] = fft_zcx_arg(temp,r->_imvec[i]);
	}
}

/*!
   \brief Incorpora un retardo temporal a los valores de la FFT

	Una vez efectuada una FFT corrige los vectores
	real e imaginario para incorporar un retardo
	al origen de fases (p.ej. un retardo de N/2 muestras avanzaria el origen
	de fases al centro de la venana de analisis de N puntos).
	No es mas que una fase lineal positiva.

	Esta funcion, solo debe utilizarse mientras se mantengan
	los vectores real e imaginario. Una vez calculado el modulo, norma o fase,
	en vez de esta hay que usar rfft_trefmove_arg().

	\note Esta funcion es poco eficiente. El retardo de fase es mejor
		incorporarlo directamente sobre la fase con rfft_trefmove_arg().

   \param r Puntero a la estructura _RFFT.

   \param nTs N�mero de muestras a retardar. Puede ser un valor fraccionario.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_trefmove_reim( pRFFT r, SPL_FLOAT nTs )
{
	SPL_INT i, n;
	SPL_FLOAT d, a, b;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++) {
		d = (M_PI*i*nTs)/n;
		a = cos(d);
		b = sin(d);
		d = r->_revec[i]*a - r->_imvec[i]*b;
		r->_imvec[i] = r->_imvec[i]*a+r->_revec[i]*b;
		r->_revec[i] = d;
	}
}

/*!
   \brief Incorpora un retardo temporal a los valores de la FFT

	Una vez efectuada una FFT y calculada la fase, corrige
	el vector de fase para incorporar un retardo
	al origen de fases (p.ej. retardar N/2 muestras avanzaria el origen
	de fases al centro de la ventana de analisis de N puntos).
	No es mas que una fase lineal positiva.

	Esta funcion, solo debe utilizarse una vez calculada la fase.
	Si se quiere incorporar el retardo antes, sobre los vectores real e
	imaginario se debe utilizar rfft_trefmove_reim().

	La fase resultante ya NO sera el valor principal de fase (entre
	-PI y PI). Si quieres el valor principal, usa rfft_trefmove_argm(),

   \param r Puntero a la estructura _RFFT.

   \param nTs N�mero de muestras a retardar. Puede ser un valor fraccionario.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_trefmove_arg( pRFFT r, SPL_FLOAT nTs )
{
	SPL_INT i, n;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++)
		r->_imvec[i] += ((M_PI*i*nTs)/n);
}

/*!
   \brief Incorpora un retardo temporal a los valores de la FFT

	Una vez efectuada una FFT y calculada la fase, corrige
	el vector de fase para incorporar un retardo
	al origen de fases (p.ej. retardar N/2 muestras avanzaria el origen
	de fases al centro de la ventana de analisis de N puntos).
	No es mas que una fase lineal positiva.

	Esta funcion, solo debe utilizarse una vez calculada la fase.
	Si se quiere incorporar el retardo antes, sobre los vectores real e
	imaginario se debe utilizar rfft_trefmove_reim().

	La fase resultante sigue siendo el valor principal de fase, y
	se mantiene entre -PI y PI.

   \param r Puntero a la estructura _RFFT.

   \param nTs N�mero de muestras a retardar. Puede ser un valor fraccionario.

   \return -
*/
PUBLIC SPL_VOID XAPI rfft_trefmove_argm( pRFFT r, SPL_FLOAT nTs )
{
	SPL_INT i, n;
	assert(r!=NULL);

	n = ( (SPL_INT)1 << r->_nbits );
	for (i = 0; i<=n; i++) {
		SPL_FLOAT d=fmod(r->_imvec[i] + (M_PI*i*nTs)/n, (2*M_PI));
		if (d>M_PI) d-= (2*M_PI);
		else if (d<-M_PI) d+= (2*M_PI);
		r->_imvec[i] = d;
	}
}

/**********************************************************/


