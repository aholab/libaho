/////////////////////////////////////////////////////////////////////
/*!
   \file xfft1.c
   \brief C�lculo de FFT de secuencias complejas

	Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao
	\author Borja Etxebarria
	\date 10/01/97
	\version 
\code
	Version  dd/mm/aa  Autor     Proposito de la edicion
	-------  --------  --------  -----------------------
	1.3.2    10/01/97  Borja     algunos comentarios
	1.3.1    30/07/95  Borja     scope funciones explicito
	1.3.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
	1.2.1    14/03/94  Borja     bug: cepfm_getceps() --> cepfm_getcepfm()
	1.2.0    28/10/93  Borja     Soporte de diferentes tipos de enventanado.
	1.1.0    01/02/93  Borja     Calculo de cepstrum fft-modulo ('clase' cepfm).
	1.0.0    16/03/92  Borja     Codificacion inicial.
\endcode

	Funciones para el calculo eficiente y reiterado de FFT compleja.
	Hay varios bloques de funciones, cada una formando una 'clase':

	- cfft : FFT de funciones complejas
	- rfft : FFT de funciones reales
	- cepfm : Calculo de los cepstrum-FFT-modulo

	Las funciones estan organizadas segun los bloques indicados.
	Estan programadas en C estandar utilizando 'handlers', siguiendo
	una estructura proxima a la programacion orientada a objetos C++.
	Para cada 'clase' (cfft , rfft , cepfm) hay una funcion
	constructora (???_construct()) que crea y devuelve un puntero a una
	estructura (CFFT, RFFT, CEPFM) de la que no es necesario conocer
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


#include "_xfft.h"

/*!
   \brief Inicializaci�n de _CFFT
   
	Inicializa una estructura _CFFT para el c�lculo de FFT de secuencias complejas

   \param np N�mero de puntos del vector de entrada (del que se quiere calcular la FFT).
			Ha de ser mayor que cero, pero no forzosamente potencia de dos.
			En caso de que no sea potencia de dos, se rellena autom�ticamente con ceros 
			(pero ojo, no en PI sino en 2PI!!! tal vez habria que corregir esto....)

   \param fftnp N�mero de puntos de la FFT. Ha de ser mayor o igual a cero\n
			Si es cero, se utiliza la menor potencia de dos superior o igual a \p np.\n
			Si no es cero, se utiliza la menor potencia de dos superior o igual a \p fftnp.\n
			Si \p np es mayor que \p fftnp, el vector de datos se trunca a \p fftnp valores.

   \param inv Indica si se desea calcular la transformada directa o inversa.
			\li \p SPL_FALSE o \p FFT_DIRECT: FFT directa
			\li \p SPL_TRUE o \p FFT_INVERSE: FFT inversa

   \param ufactor Factor de escalado. Multiplica al resultado en la FFT directa y
			lo divide en la FFT inversa. Por lo general, se deja a 1 (sin escalado).

   \return Puntero a una estructura _CFFT. Este puntero deber� utilizarse como par�metro 
			para el resto de las funciones \p cfft_???(). Devuelve \p NULL si la inicializaci�n falla
*/
PUBLIC pCFFT XAPI cfft_construct( SPL_INT np, SPL_INT fftnp,
		SPL_BOOL inv, SPL_FLOAT ufactor )
{
	pCFFT c;
	SPL_INT n, n2;
	assert(np>0);
	assert(fftnp>=0);

	c = (pCFFT)xfft_malloc(sizeof(CFFT));
	if (c!=NULL) {
		c->_nbits = fft_n_bits((fftnp>0) ? fftnp : np);
		n2 = ( (n = (SPL_INT)1 << c->_nbits) >> 1 );

		c->_revec = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n);
		c->_imvec = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n);
		c->_tinv = (SPL_pINT)xfft_malloc(sizeof(SPL_INT)*n);
		c->_tsin = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n2);
		c->_tcos = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n2);

		if ((((n!=0)&&((c->_revec==NULL)||(c->_imvec==NULL)||
				(c->_tinv==NULL)))|| ((n2!=0)&&
				((c->_tsin==NULL)||(c->_tcos==NULL))))==0) {
			fft_fill_tinv(c->_tinv,n);
			if (n2)
				fft_fill_tsin_tcos(c->_tsin,c->_tcos,n2);
			c->_inv = inv;
			c->_ufactor = ufactor;
			c->_np = ( (np>n) ? n : np );
		}
		else {
			cfft_destruct(c);
			c=NULL;
		}
	}

	return c;
}

/*!
   \brief Destructor para una estructura _CFFT

   Destruye una estructura _CFFT. Es necesario llamar a esta funci�n despu�s de haber
   llamado a cfft_construct(), una vez se ha terminado de calcular FFT's

   \param c puntero a la estructura que se quiere destruir

   \return -
*/

PUBLIC SPL_VOID XAPI cfft_destruct( pCFFT c )
{
	assert(c!=NULL);

	if (c->_revec!=NULL)
		xfft_free(c->_revec);
	if (c->_imvec!=NULL)
		xfft_free(c->_imvec);
	if (c->_tinv!=NULL)
		xfft_free(c->_tinv);
	if (c->_tsin!=NULL)
		xfft_free(c->_tsin);
	if (c->_tcos!=NULL)
		xfft_free(c->_tcos);
	xfft_free(c);
}

/*!
   \brief Fija la longitud de los vectores de entrada

   Permite modificar el n�mero de puntos de los vectores de entrada (aquellos cuya FFT
   se quiere calcular).

   \param c Puntero a la estructura _CFFT que se quiere modificar.

   \param np N�mero de puntos de los vectores de entrada. Ha de ser mayor que cero,
		pero no forzosamente potencia de dos. En caso de que \p np sea mayor que el
		n�mero de puntos de la FFT, los vectores de entrada se truncan. En caso de
		que \p np sea menor que el n�mero de puntos de la FFT, se rellena con ceros

   \return -
*/

PUBLIC SPL_VOID XAPI cfft_setnp( pCFFT c, SPL_INT np )
{
	SPL_INT n;
	assert(c!=NULL);
	assert(np>0);

	n = ((SPL_INT)1 << (c->_nbits));

	c->_np = ((np>n) ? n : np );
}

/*!
   \brief Fija el c�lculo a FFT directa o inversa

   Permite indicar si se quiere calcular una FFT directa o inversa

   \param c Puntero a la estructura _CFFT que se quiere modificar.

   \param inv: Indica si se desea calcular la transformada directa o inversa.
			\li \p SPL_FALSE o \p FFT_DIRECT: FFT directa
			\li \p SPL_TRUE o \p FFT_INVERSE: FFT inversa

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_setinv( pCFFT c, SPL_BOOL inv )
{
	assert(c!=NULL);

	c->_inv = inv;
}

/*!
   \brief Fija el factor de escala

   Permite cambiar el factor de escala aplicado a la FFT. Este factor multiplica 
		al resultado en la FFT directa y lo divide en la FFT inversa

   \param c Puntero a la estructura _CFFT que se quiere modificar.

   \param ufactor: Factor de escalado.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_setufactor( pCFFT c, SPL_FLOAT ufactor )
{
	assert(c!=NULL);

	c->_ufactor = ufactor;
}

/*!
   \brief Devuelve el n�mero de puntos de los vectores de entrada

   Permite conocer el n�mero m�nimo de puntos que han de tener los vectores de entrada
   (aquellos cuya FFT se quiere calcular).

   \param c Puntero a la estructura _CFFT.

   \return N�mero de puntos de los vectores de entrada
*/
PUBLIC SPL_INT XAPI cfft_getnp( pCFFT c )
{
	assert(c!=NULL);

	return c->_np;
}

/*!
   \brief Devuelve el n�mero de puntos de la FFT

   Permite conocer el n�mero de puntos que tienen las FFT calculadas. Devuelve
   el verdadero n�mero de puntos utilizado (siempre potencia de dos)

   \param c Puntero a la estructura _CFFT.

   \return N�mero de puntos de la FFT
*/
PUBLIC SPL_INT XAPI cfft_getfftnp( pCFFT c )
{
	assert(c!=NULL);

	return ((SPL_INT)1 << (c->_nbits));
}

/*!
   \brief Devuelve el n�mero de puntos de la FFT

   Devuelve el n�mero de puntos de los vectores de salida. 
   El valor devuelto coincide con cfft_getfftnp() (siempre potencia de dos).

   \param c Puntero a la estructura _CFFT.

   \return N�mero de puntos de los vectores
*/
PUBLIC SPL_INT XAPI cfft_getvecnp( pCFFT c )
{
	assert(c!=NULL);

	return ((SPL_INT)1 << (c->_nbits));
}

/*!
   \brief Devuelve si se est� calculando una FFT directa o inversa

   \param c Puntero a la estructura _CFFT.

   \return
			\li \p SPL_FALSE o \p FFT_DIRECT: FFT directa
			\li \p SPL_TRUE o \p FFT_INVERSE: FFT inversa
*/
PUBLIC SPL_BOOL XAPI cfft_getinv( pCFFT c )
{
	assert(c!=NULL);

	return c->_inv;
}

/*!
   \brief Devuelve el factor de escala aplicado

   \param c Puntero a la estructura _CFFT.

   \return El factor de escala aplicado
*/
PUBLIC SPL_FLOAT XAPI cfft_getufactor( pCFFT c )
{
	assert(c!=NULL);

	return c->_ufactor;
}

/*!
   \brief Acceso a la parte real de la FFT

   Permite acceder al vector de la parte real de la FFT calculada. El usuario debe haber
   calculado una FFT previamente. Este vector es de uso interno y no debe borrarse, aunque
   se puede modificar.

   En caso de que se calcule el m�dulo o la norma (m�dulo al cuadrado) de la FFT, estos
   valores se almacenan en este mismo vector.

   El vector devuelto tiene cfft_getvecnp() puntos (igual al n�mero de puntos dela FFT), 
   y sus valores se corresponden al rango [0, 2*PI] (en herzios [0, fs])

   \param c Puntero a la estructura _CFFT.

   \return Puntero al inicio del vector de la parte real de la FFT.
*/
PUBLIC SPL_pFLOAT XAPI cfft_getrevec( pCFFT c )
{
	assert(c!=NULL);

	return c->_revec;
}

/*!
   \brief Acceso a la parte imaginaria de la FFT

   Permite acceder al vector de la parte imaginaria de la FFT calculada. El usuario debe haber
   calculado una FFT previamente. Este vector es de uso interno y no debe borrarse, aunque
   se puede modificar.

   En caso de que se calcule la fase de la FFT, estos
   valores se almacenan en este mismo vector.

   El vector devuelto tiene cfft_getvecnp() puntos (igual al n�mero de puntos dela FFT), 
   y sus valores se corresponden al rango [0, 2*PI] (en herzios [0, fs])

   \param c Puntero a la estructura _CFFT.

   \return Puntero al inicio del vector de la parte imaginaria de la FFT.
*/
PUBLIC SPL_pFLOAT XAPI cfft_getimvec( pCFFT c )
{
	assert(c!=NULL);

	return c->_imvec;
}

/*!
   \brief Macro de uso interno
*/
#define __cfft_cfft(c,re_invec,im_invec)  \
{  \
	SPL_INT i, n;  \
	SPL_pINT tinv;  \
	\
	n = ((SPL_INT)1 << c->_nbits);  \
	tinv = c->_tinv; \
	/* inversion de bits */  \
	for (i=0; i<c->_np; i++) {	\
		c->_revec[*tinv] = (*(re_invec++));  \
		c->_imvec[*(tinv++)] = (*(im_invec++));  \
	} \
	/* cero padding */	\
	for (i=c->_np; i<n; i++) {	\
		c->_revec[*tinv] = 0.0;  \
		c->_imvec[*(tinv++)] = 0.0;  \
	}  \
	fft_fft(c->_nbits,c->_inv,c->_revec,c->_imvec,c->_tsin,  \
			c->_tcos,c->_ufactor); \
}

/*!
   \brief Calcula la FFT de una secuencia compleja

   Calcula la FFT de una secuencia compleja. La parte real de esta FFT puede
   recuperarse mediante cfft_getrvec(), y la parte imaginaria mediante cfft_getimvec().

   \param c Puntero a la estructura _CFFT.

   \param re_invec Vector que contiene la parte real de la secuencia cuya FFT
		se quiere calcular. Debe tener cfft_getnp() puntos.

   \param im_invec Vector que contiene la parte imaginaria de la secuencia cuya FFT
		se quiere calcular. Debe tener cfft_getnp() puntos.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_cfft( pCFFT c,
		SPL_pFLOAT re_invec, SPL_pFLOAT im_invec )
{
	assert(c!=NULL);

	__cfft_cfft(c,re_invec,im_invec);
}

/*!
   \brief Calcula el m�dulo de una FFT ya calculada

   Una vez calculada una FFT, permite extraer el m�dulo de la misma. El resultado
   se introduce en el vector de la parte real de la FFT, de forma que posteriormente es
   accesible mediante cfft_getrevec(). 
   
   \note Una vez llamada esta funci�n, se pierde la parte real. Por lo tanto, 
		ya no se puede recuperar esta parte real ni se puede calcular la fase.
   
   \param c Puntero a la estructura _CFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_mag( pCFFT c )
{
	SPL_INT i, n;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++)
		c->_revec[i] = __fft_cx_mag(c->_revec[i],c->_imvec[i]);
}

/*!
   \brief Calcula la norma de una FFT ya calculada

   Una vez calculada una FFT, permite extraer la norma (m�dulo al cuadrado) de la misma.
   El resultado se introduce en el vector de la parte real de la FFT, de forma que
   posteriormente es accesible mediante cfft_getrevec(). 
   
   \note Una vez llamada esta funci�n, se pierde la parte real. Por lo tanto, 
		ya no se puede recuperar esta parte real ni se puede calcular la fase.
   
   \param c Puntero a la estructura _CFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_norm( pCFFT c )
{
	SPL_INT i, n;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++)
		c->_revec[i] = __fft_cx_norm(c->_revec[i],c->_imvec[i]);
}

/*!
   \brief Calcula la fase de una FFT ya calculada

   Una vez calculada una FFT, permite extraer la fase de la misma.
   El resultado se introduce en el vector de la parte imaginaria de la FFT, de forma que
   posteriormente es accesible mediante cfft_getimvec(). 
   
   \note Una vez llamada esta funci�n, se pierde la parte imaginaria. Por lo tanto, 
		ya no se puede recuperar esta parte imaginaria ni se puede calcular el m�dulo.
   
   \param c Puntero a la estructura _CFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_arg( pCFFT c )
{
	SPL_INT i, n;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++)
		c->_imvec[i] = fft_zcx_arg(c->_revec[i],c->_imvec[i]);
}

/*!
   \brief Calcula el m�dulo y la fase de una FFT ya calculada

   Una vez calculada una FFT, permite extraer el m�dulo y la fase de la misma.
   El m�dulo se introduce en el vector de la parte real de la FFT, y la fase en
   el vector de la parte imaginaria, de forma que posteriormente son accesibles
   mediante cfft_getrevec() y cfft_getimvec() respectivamente. 
   
   \param c Puntero a la estructura _CFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_magarg( pCFFT c )
{
	SPL_INT i, n;
	SPL_FLOAT temp;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++) {
		temp = c->_revec[i];
		c->_revec[i] = __fft_cx_mag(temp,c->_imvec[i]);
		c->_imvec[i] = fft_zcx_arg(temp,c->_imvec[i]);
	}
}

/*!
   \brief Calcula la norma y la fase de una FFT ya calculada

   Una vez calculada una FFT, permite extraer la norma (m�dulo al cuadrado) y la fase
   de la misma. La norma se introduce en el vector de la parte real de la FFT,
   y la fase en el vector de la parte imaginaria, de forma que posteriormente son
   accesibles mediante cfft_getrevec() y cfft_getimvec() respectivamente. 
   
   \param c Puntero a la estructura _CFFT.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_normarg( pCFFT c )
{
	SPL_INT i, n;
	SPL_FLOAT temp;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++) {
		temp = c->_revec[i];
		c->_revec[i] = __fft_cx_norm(temp,c->_imvec[i]);
		c->_imvec[i] = fft_zcx_arg(temp,c->_imvec[i]);
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
	en vez de esta hay que usar cfft_trefmove_arg().

	\note Esta funcion es poco eficiente. El retardo de fase es mejor
		incorporarlo directamente sobre la fase con cfft_trefmove_arg().

   \param c Puntero a la estructura _CFFT.

   \param nTs N�mero de muestras a retardar. Puede ser un valor fraccionario.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_trefmove_reim( pCFFT c, SPL_FLOAT nTs )
{
	SPL_INT i, n;
	SPL_FLOAT d, a, b;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++) {
		d = (2*M_PI*i*nTs)/n;
		a = cos(d);
		b = sin(d);
		d = c->_revec[i]*a - c->_imvec[i]*b;
		c->_imvec[i] = c->_imvec[i]*a+c->_revec[i]*b;
		c->_revec[i] = d;
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
	imaginario se debe utilizar cfft_trefmove_reim().

	La fase resultante ya NO sera el valor principal de fase (entre
	-PI y PI). Si quieres el valor principal, usa cfft_trefmove_argm(),

   \param c Puntero a la estructura _CFFT.

   \param nTs N�mero de muestras a retardar. Puede ser un valor fraccionario.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_trefmove_arg( pCFFT c, SPL_FLOAT nTs )
{
	SPL_INT i, n;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++)
		c->_imvec[i] += ((2*M_PI*i*nTs)/n);
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
	imaginario se debe utilizar cfft_trefmove_reim().

	La fase resultante sigue siendo el valor principal de fase, y
	se mantiene entre -PI y PI.

   \param c Puntero a la estructura _CFFT.

   \param nTs N�mero de muestras a retardar. Puede ser un valor fraccionario.

   \return -
*/
PUBLIC SPL_VOID XAPI cfft_trefmove_argm( pCFFT c, SPL_FLOAT nTs )
{
	SPL_INT i, n;
	assert(c!=NULL);

	n = ( (SPL_INT)1 << c->_nbits );
	for (i = 0; i<n; i++) {
		SPL_FLOAT d=fmod(c->_imvec[i] + ((2*M_PI)*i*nTs)/n, (2*M_PI));
		if (d>M_PI) d-= (2*M_PI);
		else if (d<-M_PI) d+= (2*M_PI);
		c->_imvec[i] = d;
	}
}

/**********************************************************/

