/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XFFT3.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.3.2    30/07/95  Borja     scope funciones explicito
1.3.1    30/03/95  Borja     llamada invalida en cefm_cepfm_f32()
1.3.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.2.1    14/03/94  Borja     bug: cepfm_getceps() --> cepfm_getcepfm()
1.2.0    28/10/93  Borja     Soporte de diferentes tipos de enventanado.
1.1.0    01/02/93  Borja     Calculo de cepstrum fft-modulo ('clase' cepfm).
1.0.0    16/03/92  Borja     Codificacion inicial.

======================== Contenido ========================
Funciones para el calculo eficiente y reiterado de
cepstrum-FFT-modulo.

Ver XFFT1.C para informacion general sobre los modulos XFFT?.C.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "_xfft.h"

/**********************************************************/
/* Cepstrum FFT-modulo. Funcion constructora de la estructura.

Recibe como parametros:
- SPL_INT {np} : Numero de puntos de los vectores de datos (reales)
.                de los que queremos calcular los cepstrum FFT-modulo.
.                {np}>0, y no tiene por que ser una potencia de 2 (relleno
.                automatico con ceros).
.                La fft directa utiliza la menor potencia de dos superior
.                o igual a {np}.

- SPL_INT {ifftnp} : Numero de puntos de la FFT inversa.
.                    Si este parametro es cero, se utiliza la menor potencia
.                    de dos superior o igual a {np}.
.                    Si este valor no es cero, se utiliza la menor potencia
.                    de dos superior o igual a {ifftnp}. En caso de que {np}
.                    sea menor que {ifftnp} (realmente las potencias de dos
.                    mas proximas), se introducen ceros al rededor
.                    de la muestra correspondiente a PI. Asi, la fft inversa
.                    da como resultado cepstrum con valores 'interpolados'.
.                    {ifftnp}>=0, y no tiene porque ser potencia de 2.

- SPL_FLOAT {ufactor} : factor de escalado. Multiplica al resultado
.                        de los cepstrum. Por lo general este
.                        factor es 1 (sin escalado).

- XFFT_WIN {win} : Codigo que identifica el tipo de ventana a
.                  utilizar. Son los codigos XFFT_WIN_??? definidos
.                  en xfft.h. Ver rfft_setwin() para mas informacion.
.                  Si se utilizar una ventana, sera de {np} puntos,
.                  aunque si el numero de puntos de la fft es menor,
.                  se utiliza este otro valor.

- XFFT_WIN_FUNC {winfunc} : Puntero a una funcion que sigue el
.                           prototipo indicado (xfft.h) que suministra
.                           el usuario. Esta funcion se utiliza para
.                           obtener la ventana a utilizar cuando
.                           en {win} se envia XFFT_WIN_USER. Si este
.                           puntero es NULL y se envia XFFT_WIN_USER
.                           en {win}, no se utiliza ningun enventanado.

La funcion {devuelve} un puntero a una estructura, que debera utilizarse
como primer parametro para el resto de las funciones cepfm_???().
Si no es posible crear la estructura, la funcion {devuelve} NULL. */

PUBLIC pCEPFM XAPI cepfm_construct( SPL_INT np, SPL_INT ifftnp,
		SPL_FLOAT ufactor, XFFT_WIN win, XFFT_WIN_FUNC winfunc )
{
	pCEPFM c;
	SPL_INT n, i;
	assert(np>0);
	assert(ifftnp>=0);

	c = (pCEPFM)xfft_malloc(sizeof(CEPFM));

	if (c!=NULL) {

		if (!ifftnp)
			ifftnp = np;

		c->_r1 = rfft_construct(np,0,FFT_DIRECT,1,win,winfunc);
		n = ((SPL_INT)1 << fft_n_bits(ifftnp));

		/* fft inversa: divide entre ufactor */
		/* log(XAPI^2)=2*log(XAPI), por tanto, escala
		a 1/2 --> ifft divide entre 2 */
		c->_r2 = rfft_construct(n,0,FFT_INVERSE,
				2.0/ufactor,XFFT_WIN_NONE,NULL);

		c->_tmp = (SPL_pFLOAT)xfft_malloc(sizeof(SPL_FLOAT)*n);

		if ((c->_r1!=NULL)&&(c->_r2!=NULL)&&(c->_tmp!=NULL)) {
			for (i=0; i<n; i++)
				c->_tmp[i]=0.0;
		}
		else {
			cepfm_destruct(c);
			c=NULL;
		}
	}

	return c;
}

/**********************************************************/
/* destructor para una estructura CEPFM */

PUBLIC SPL_VOID XAPI cepfm_destruct( pCEPFM c )
{
	assert(c!=NULL);

	if (c->_r1)
		rfft_destruct(c->_r1);
	if (c->_r2)
		rfft_destruct(c->_r2);
	if (c->_tmp)
		xfft_free(c->_tmp);
	xfft_free(c);
}

/**********************************************************/
/* modifica el uso de la ventana en la fft directa.
Enviar {win}==XFFT_WIN_??? para utilizar la ventana del tipo
deseado. En xfft.h se definen constantes XFFT_WIN_??? para
diversos tipos de ventanas (??? = HAMM, HANN, BART, BLACK,
KAIS5, ... KAIS10, USER, NONE). Si {win} es XFFT_WIN_NONE o
XFFT_WIN_RECT no se utiliza ninguna ventana. Si {win} es
XFFT_WIN_USER, entonces se utilizara la ventana obtenida
a traves de la funcion que el usuario suministra en el puntero
{winfunc} (siempre y cuando sea distinto de NULL, ya que si es
NULL no se utiliza ninguna ventana). En xfft.h se explica el
prototipo que debe seguir esta funcion.

Puesto que el proceso realizado puede implicar la creacion
de vectores de datos utilizando memoria dinamica, la
funcion {devuelve} SPL_TRUE si todo va bien, o SPL_FALSE si se produce
algun error. En caso de error, la ventana se desconecta (se
utiliza ventana rectangular) */

PUBLIC SPL_BOOL XAPI cepfm_setwin( pCEPFM c, XFFT_WIN win,
		XFFT_WIN_FUNC winfunc )
{
	assert(c!=NULL);

	return rfft_setwin(c->_r1,win,winfunc);
}

/**********************************************************/
/* modifica el numero de puntos de los vectores de los que se
quiere calcular los cepstrum, indicando que los vectores
tendran {np} puntos. Si {np} es mayor que el numero de puntos de la
FFT directa configurada al crear la estructura cepfm, {np} se trunca
al valor del numero de puntos de esta FFT.

Puesto que esto puede implicar la creacion de vectores de datos
utilizando memoria dinamica, la funcion {devuelve} SPL_TRUE si todo
va bien, o SPL_FALSE si se produce algun error. En caso de error,
significa que la ventana de hamming se desconecta (se utiliza
ventana rectangular) */

PUBLIC SPL_BOOL XAPI cepfm_setnp( pCEPFM c, SPL_INT np )
{
	assert(c!=NULL);

	return rfft_setnp(c->_r1,np);
}

/**********************************************************/
/* modifica simultaneamente el numero de puntos de los vectores
de datos y el uso del enventanado (ver cepfm_setwin()).
El valor SPL_BOOL que {devuelve} la funcion es similar al indicado
en cepfm_setwin() */

PUBLIC SPL_BOOL XAPI cepfm_setnpwin( pCEPFM c, SPL_INT np,
		XFFT_WIN win, XFFT_WIN_FUNC winfunc )
{
	assert(c!=NULL);

	return rfft_setnpwin(c->_r1,np,win,winfunc);
}

/**********************************************************/
/* modifica el factor de usuario que multiplica a los cepstrum */

PUBLIC SPL_VOID XAPI cepfm_setufactor( pCEPFM c, SPL_FLOAT ufactor )
{
	assert(c!=NULL);

	rfft_setufactor(c->_r2,2/ufactor);
}

/**********************************************************/
/* {devuelve} el numero de puntos que deben tener los vectores
de datos de los que se quiere calcular los cepstrum */

PUBLIC SPL_INT XAPI cepfm_getnp( pCEPFM c )
{
	assert(c!=NULL);

	return rfft_getnp(c->_r1);
}

/**********************************************************/
/* {devuelve} el numero de puntos del vector resultado que contiene
los cepstrum FFT-modulo */

PUBLIC SPL_INT XAPI cepfm_getcepnp( pCEPFM c )
{
	assert(c!=NULL);

	return rfft_getvecnp(c->_r2);
}

/**********************************************************/
/* {devuelve} el tipo de enventanado que se esta utilizando
(XFFT_WIN_???, con ??? = NONE (o RECT), HAMM, HANN, ....
consultar xfft.h ) */

PUBLIC XFFT_WIN XAPI cepfm_getwin( pCEPFM c )
{
	assert(c!=NULL);

	return rfft_getwin(c->_r1);
}

/**********************************************************/
/* {devuelve} el puntero a la funcion de enventanado que
ha suministrado el usuario */

PUBLIC XFFT_WIN_FUNC XAPI cepfm_getwinfunc( pCEPFM c )
{
	assert(c!=NULL);

	return rfft_getwinfunc(c->_r1);
}

/**********************************************************/
/* {devuelve} el factor de usuario */

PUBLIC SPL_FLOAT XAPI cepfm_getufactor( pCEPFM c )
{
	assert(c!=NULL);

	/* fft inversa, 'divide' entre 2 */
	return 2.0 / rfft_getufactor(c->_r2);
}

/**********************************************************/
/* {devuelve} un puntero al vector que contiene los cepstrum.
El usuario previamente habra tenido que calcular los cepstrum.
Este vector es interno y el usuario no debe borrarlo, aunque
si puede modificarlo.
Este vector tiene cepfm_getcepnp() puntos. */

PUBLIC SPL_pFLOAT XAPI cepfm_getcepfm( pCEPFM c )
{
	assert(c!=NULL);

	return rfft_getrevec(c->_r2);
}

/**********************************************************/
/* uso interno: 2*log(modulo(r,i)) */

PRIVATE SPL_FLOAT cepfm_logmod2( SPL_FLOAT * r, SPL_FLOAT * i )
{
	SPL_FLOAT tmp;

	tmp = __fft_cx_norm(*r,*i);

	if (tmp<(FFT_ZERO*FFT_ZERO))
		return log((FFT_ZERO*FFT_ZERO));
	else
		return log(tmp);
}

/**********************************************************/
/* uso interno */

PRIVATE SPL_VOID cepfm_docepfm( pCEPFM c )
{
	SPL_INT i, n1, n2;
	SPL_pFLOAT qr,qi, pi,pf;
	assert(c!=NULL);

	n2 = rfft_getnp(c->_r2);
	pi = (c->_tmp);
	pf = (c->_tmp)+n2-1;
	n2 >>= 1;
	n1 = rfft_getvecnp(c->_r1)-1;
	if (n1>n2)
		n1 = n2;
	qr = rfft_getrevec(c->_r1);
	qi = rfft_getimvec(c->_r1);

	/* log(XAPI**2)==2*log(XAPI), por tanto hay que dividir
	entre 2 en la fft inversa */
	(*(pi++)) = cepfm_logmod2(qr++,qi++);
	for (i=1; i<n1; i++)
		(*(pi++)) = (*(pf--)) = cepfm_logmod2(qr++,qi++);
	if (n1)
		(*pf) = cepfm_logmod2(qr,qi);

	rfft_rfft(c->_r2,c->_tmp);
}

/**********************************************************/
/* calcula los cepstrum FFT-modulo de un vector {invec} de cepfm_getnp()
elementos de tipo SPL_FLOAT */

PUBLIC SPL_VOID XAPI cepfm_cepfm( pCEPFM c, SPL_pFLOAT invec )
{
	assert(c!=NULL);

	rfft_rfft(c->_r1,invec);
	cepfm_docepfm(c);
}

/**********************************************************/
/* calcula los cepstrum FFT-modulo de un vector {invec} de cepfm_getnp()
elementos de tipo SPL_INT */

PUBLIC SPL_VOID XAPI cepfm_cepfm_i( pCEPFM c, SPL_pINT invec )
{
	assert(c!=NULL);

	rfft_rfft_i(c->_r1,invec);
	cepfm_docepfm(c);
}

/**********************************************************/
/* calcula los cepstrum FFT-modulo de un vector {invec} de cepfm_getnp()
elementos de tipo INT16 */

PUBLIC SPL_VOID XAPI cepfm_cepfm_i16( pCEPFM c, pINT16 invec )
{
	assert(c!=NULL);

	rfft_rfft_i16(c->_r1,invec);
	cepfm_docepfm(c);
}

/**********************************************************/
/* calcula los cepstrum FFT-modulo de un vector {invec} de cepfm_getnp()
elementos de tipo FLOAT32 */

PUBLIC SPL_VOID XAPI cepfm_cepfm_f32( pCEPFM c, pFLOAT32 invec )
{
	assert(c!=NULL);

	rfft_rfft_f32(c->_r1,invec);
	cepfm_docepfm(c);
}

/**********************************************************/

