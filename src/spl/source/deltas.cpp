/*!
   \file cbuffer.cpp
   \brief Implementación de buffers circulares

	Copyright: 2010 - Grupo de Voz (DAET) ETSII/IT-Bilbao
	\author Iker Luengo
	\version 1.0.0
	\date 04/03/10

	\if SKIP
	Version  dd/mm/aa  Autor     Proposito de la edicion
	-------  --------  --------  -----------------------
	1.0.0    04/03/10  ikerl     Codificación inicial
	\endif
*/

#include <cassert>
#include "deltas.h"



/*!
	A partir de un buffer de valores, obtiene un valor de diferencia.
	La fórmula para el cálculo de esta diferencia se corresponde con la fórmula
	de la pendiente de la regresión lineal de los valores del buffer
*/
void
GetDiff	( const buffer2D& buffer,	///<Buffer de donde se tomaran los valores para calcular la diferencia.
		  DOUBLE* diff )			///<Memoria en la que se va a almacenar la diferencia calculada.
									///<Debe tener buffer.Dim() posiciones.
{
	//Fórmula de la pendiente de regresión lineal: Numerador
	INT iWindow = (buffer.Size()-1)/2;
	INT iMiddle = iWindow;
	for (INT j = 0; j < buffer.Dim(); ++j)
	{	
		diff[j] = 0;
		for (INT i = 1; i <= iWindow; ++i)
			diff[j] += i*(buffer[iMiddle+i][j] - buffer[iMiddle-i][j]);
	}
	
	//Fórmula de la pendiente de regresión lineal: Denominador común para todos los elementos
	DOUBLE acumulador = 0;
	for (INT i = 1; i <= iWindow; ++i)
		acumulador += i*i;

	for (INT j = 0; j < buffer.Dim(); ++j)
	{	
		//Para evitar indeterminaciones si iWindow == 0
		if (diff[j] != 0.0)
			diff[j] /= (2.0*acumulador);
	}
}

	

/*!
	A partir de un buffer de valores, obtiene un valor de diferencia.
	La fórmula para el cálculo de esta diferencia se corresponde con la fórmula
	de la pendiente de la regresión lineal de los valores del buffer.

	\return Diferencia obtenida a partir de las muestras del buffer.
*/
DOUBLE
GetDiff	( const buffer1D& buffer )		///<Buffer de donde se tomaran los valores para calcular la diferencia.
{
	INT iWindow = (buffer.Size()-1)/2;
	INT iMiddle = iWindow;

	//Fórmula de la pendiente de regresión lineal: Numerador y denominador
	DOUBLE sal = 0;
	DOUBLE acumulador = 0;
	for (INT i = 1; i <= iWindow; ++i)
	{
		sal += i*(buffer[iMiddle+i] - buffer[iMiddle-i]);
		acumulador += i*i;
	}
		
	//Para evitar indeterminaciones si iWindow == 0
	if (sal == 0.0)
		return 0.0; 
	return sal /= (2.0*acumulador);
}




/*!
 * El número de deltas puede ser 0, pero ninguna ventana puede ser ni cero ni negativa,
 * o se provoca un ASSERT.
 * 
 * \p Ndeltas = 0 se trata como un caso especial
 */	
void
ParamDeltas::Initialize (INT Ndeltas,		///<Número de deltas a calcular
						 INT* windowSize,	///<Tamaño de la ventana para el cálculo de cada delta
						 INT Ndim)			///<nº de dimensiones de los parámetros (sin deltas)
{
	//Comprobamos
	assert (Ndeltas >= 0);

	m_Ndim = Ndim;
	m_bInit = false;
	
	//Reservamos buffers para todas las deltas a calcular
	//El buffer [0] será para los vectores sin deltas.
	//El buffer [Ndeltas-1] será para las deltas de orden (Ndeltas-1)
	//Las últimas deltas no requieren buffer, y se calculan 'on the fly'
	//cuando se llama a Get()
	m_vDeltaBuffers.resize(Ndeltas);

	if (Ndeltas != 0)
		m_vMemBuffers.resize(Ndeltas);
	else
	{
		m_vMemBuffers.resize(1);	//La memoria de muestras SIEMPRE debe existir
		m_vMemBuffers[0].Resize (1, Ndim);
	}
	
	INT memsize = 1;
	for (INT i = Ndeltas-1; i >= 0; --i)
	{
		assert (windowSize[i] > 0);
		memsize += windowSize[i];
		m_vDeltaBuffers[i].Resize (2*windowSize[i]+1, Ndim);
		m_vMemBuffers[i].Resize (memsize, Ndim);
	}
	
	//Reservamos memoria para el vector de deltas
	if (m_pdDiff)
		xfree(m_pdDiff);
	m_pdDiff = (DOUBLE*)xmalloc(Ndim*sizeof(DOUBLE));
}


void
ParamDeltas::Push (const DOUBLE* e)	///<Vector a añadir. Debe tener Dim() elementos
{
	if (!m_bInit && Ndeltas() != 0)
		Populate(e);

	if (Ndeltas() != 0)
		//Metemos el vector en el buffer de parámetros
		m_vDeltaBuffers[0].Push(e);
	//Aquí siempre lo podemos meter
	m_vMemBuffers[0].Push(e);

	//Por cada delta, la calculamos y la metemos en el buffer correspondiente
	for (INT i = 1; i < Ndeltas(); ++i)
	{
		GetDiff (m_vDeltaBuffers[i-1], m_pdDiff);
		m_vDeltaBuffers[i].Push(m_pdDiff);
		m_vMemBuffers[i].Push(m_pdDiff);
	}
	//La úlima delta no la calculamos, como ya hemos dicho, se calculará
	//sólo cuando se llame a Get()
}
	
	
/*!
 * Devuelve un vector completo con deltas. Cuidado, ya que
 * el vector devuelto tiene un retardo de Delay() tramas con respecto
 * al último vector insertado con Push(). Dicho de otra manera, en el siguiente código:
 * 
 * \code
 * 		INT ndeltas = 2;
 * 		INT wins [] = {2, 3};
 * 		INT dim = 10;
 * 		ParamDeltas deltas (ndeltas, wins, dim);
 * 
 * 		DOUBLE* frame = new DOUBLE [dim];						//Trama sin deltas
 * 		DOUBLE* deltaframe = new DOUBLE [(ndeltas+1)*dim];		//Trama con deltas
 * 
 * 		for (int i = 0, i < Nframes(); ++i)
 * 		{
 *			//Recogemos una trama
 * 			frame = GetFrame(i);
 * 			deltas.Push(frame);
 * 			deltas.Get(deltaframe);
 * 		}
 * \endcode
 * 
 * Cuando metemos al sistema la trama \p i mediante un Push(), Get() nos devuelve la trama \p i-Delay().
 */
void
ParamDeltas::Get(DOUBLE* e) const	///<Memoria donde escribir el vector. Debe tener Ndim()*(Ndeltas()+1) elementos
{
	//Las muetras buenas siempre están en la posición 0 de las memorias
	for (INT i = 0; i < Ndeltas(); ++i)
		GetSample(m_vMemBuffers[i], e+i*Ndim(), 0);
	
	//Falta por calcular la última delta
	if (Ndeltas() != 0)
		GetDiff (m_vDeltaBuffers[Ndeltas()-1], e+Ndeltas()*Ndim());
	else
		GetSample(m_vMemBuffers[0], e, 0);
}
	
/*!
 * Cuando llega la primerísima trama, esta función se encarga de
 * llenar el buffer [0] (el de las muestras sin deltas). En realidad
 * lo llena por completo excepto la última posición, de la cual ya se encarga Push()
 */
void
ParamDeltas::Populate (const DOUBLE* e)
{
	for (INT i = 0; i < m_vDeltaBuffers[0].Size()-1; ++i)
		m_vDeltaBuffers[0].Push(e);
	m_bInit = true;
}
