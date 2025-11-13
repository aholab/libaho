#ifndef __DELTAS_H__
#define __DELTAS_H__

/////////////////////////////////////////////////////////////////////
/*!
   \file deltas.h
   \brief Cáclulo de derivadas de parámetros

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




#include <stdio.h>
#include <vector>
#include "uti.h"
#include "xalloc.h"
#include "cbuffer.h"


///Clase para el cálculo sistemático y eficiente de derivadas de parámetros
/*!
 * Supongamos que tenemos un archivo de audio que queremos parametrizar. Supongamos
 * asímismo que tenemos una función que, dada una trama de muestras, genera un
 * vector de parámetros. Y por último, supongamos que queremos añadir a esa
 * parametrización información dinámica: derivadas.
 * 
 * El esqueleto de la parametrización sería algo así (por supuesto, el código es muy
 * mejorable, pero sirve para el ejemplo):
 * 
 * \code
 * 		INT frameLen = 80:			//Tamaño de la ventana de trama en muestras
 * 		INT npars = 10;				//Número de parámetros
 * 		INT ndeltas = 2;			//Número de derivadas a calcular (deriv. y aceleraciones)
 * 
 *		//array para recoger una trama de muestras
 * 		DOUBLE* frame = new DOUBLE [frameLen];
 * 
 *		//array para recoger un vector de parámetros (sin deltas)
 * 		DOUBLE* params = new DOUBLE [npars];
 * 		
 *		//array para recoger las derivadas de los parámetros
 * 		DOUBLE* deltaparams = new DOUBLE [npars];
 * 		
 *		//array para recoger un vector de parámetros junto con las deltas (concatenadas)
 * 		DOUBLE* finalparams = new DOUBLE [npars*(ndeltas+1)];
 * 		
 * 		for (INT i = 0; i < Nframes(); ++i)
 * 		{
 * 			frame = GetFrame(i);
 * 			params = GetParams(frame);
 * 
 *			//Calculamos las primeras derivadas y las añadimos a los parámetros
 * 			deltaparams = GetDeltas(params);
 * 			finalparams = Catenate(params, deltaparams);
 * 
 *			//Calculamos las segundas derivadas y las añadimos a los parámetros
 * 			deltaparams = GetDeltas(deltaparams);
 * 			finalparams = Catenate(finalparams, deltaparams);
 * 
 *			//Hacemos algo con los parámetros
 * 			DoSomething(finalparams);
 * 		}
 * \endcode
 * 
 * Salvo por un pequeño detalle. Si se quiere calcular la delta de una trama de
 * forma precisa, es necesario disponer de una ventana de tramas \e por \e delante y
 * \e por \e detrás de la trama actual. En modo gráfico:
 * 
 * \code
 * 		trama ->      i-3    i-2    i-1     i     i+1    i+2    i+3
 * 		               *      *      *      *      *      *      *
 * 		ventana ->         \----------------------------------/
 * 		                                    |
 * 		                                    *
 * 		delta ->                            i
 * \endcode
 * 
 * Dicho de otra forma, para calcular la delta de la trama i-ésima, es necesario que el puntero
 * del archivo esté leyendo la trama i+2 (en este ejemplo). Esto tiene varias implicaciones:
 * 
 * 		- Existe un retardo entre la trama que se lee del archivo y la trama para la que se está
 * 			calculando la delta
 * 		- Para calcular la delta de la última trama, debemos leer más allá del final del fichero.
 * 			Igualmente, para calcular la delta de la primera debemos tener tramas anteriores.
 * 			Esto se suele solucionar repitiendo la última y primera tramas respectivamente,
 * 			o no calculando las deltas al inicio y al final del fichero
 * 		- Es necesario mantener una memoria de tramas pasadas
 * 
 * Todo esto se complica de sobremanera si estamos calculando deltas de deltas: El retardo del cálculo
 * se acumula, cada delta puede tener ventanas de tamaño diferente, necesitas varias memorias (una
 * para cada delta y otra para las muestras sin deltas), etc.
 * 
 * Bien, pues esta clase te permite abstraerte de (casi) todo esto. Lo único que debes tener en cuenta es que
 * existe un retardo entre la trama que le das y la delta que te devuelve. Pero incluso este retrardo te lo
 * calcula él de forma automática.
 * 
 * El ejemplo anterior, utilizando esta clase, sería algo así como:
 * 
 * \code
 * 		INT frameLen = 80:			//Tamaño de la ventana de trama en muestras
 * 		INT npars = 10;				//Número de parámetros
 * 		INT ndeltas = 2;			//Número de derivadas a calcular (deriv. y aceleraciones)
 * 		INT wins [] = {2, 3};		//Tamaño de las ventanas para cálculo de deltas (en tramas)
 * 
 *		//Este objeto se encarga de todo el cálculo de deltas
 *		ParamDeltas deltas (ndeltas, wins, npars);
 * 
 *		//array para recoger una trama de muestras
 * 		DOUBLE* frame = new DOUBLE [frameLen];
 * 
 *		//array para recoger un vector de parámetros (sin deltas)
 * 		DOUBLE* params = new DOUBLE [npars];
 * 		
 *		//array para recoger un vector de parámetros junto con las deltas (concatenadas)
 * 		DOUBLE* finalparams = new DOUBLE [npars*(ndeltas+1)];
 * 		
 * 		for (INT i = 0; i < Nframes()+deltas.Delay(); ++i)
 * 		{
 *			//Estamos leyendo más allá del fichero. No debemos hacerlo
 *			//Si i >= NFrames(), símplemente se repite la última muestra
 *			if (i < NFrames())
 * 				frame = GetFrame(i);
 * 			params = GetParams(frame);
 * 
 *			//Metemos los parámetros en el sistema
 * 			deltas.Push(params);
 * 
 *			//El valor devuelto por el sistema no tiene sentido hasta que no hayamos
 * 			//metido deltas.Delay() tramas
 * 			if (i < deltas.Delay())
 * 				continue;
 * 
 *			//Recuperamos la trama con deltas
 *			//Será la trama i-deltas.Delay()
 * 			deltas.Get(finalparams)
 * 
 *			//Hacemos algo con los parámetros
 * 			DoSomething(finalparams);
 * 		}
 * \endcode
 * 
 * La clase automáticamente repite la primera trama al principio del fichero, con lo que es capaz de
 * calcular la delta de la primera trama. Con respecto a la última trama, como no hay forma de que la clase sepa
 * que es la última, pues ya nos encargamos nosotros, repitiendo la última trama Delay() veces.
 * 
 * Hay que tener en cuenta que Get() devuelve el vector con deltas para la trama i-Delay(). Si i\<Delay(), entonces
 * estamos recuperando una trama que en realidad no existe. El objeto está sin inicializar del todo, y sólo
 * devuelve mierda.
 * 
 * Por último, si no se quiere que se repitan la primera y última trama (es decir, si podemos soportar no
 * calcular deltas al principio y final del archivo), se puede hacer así:
 * 
 * \code
 * 		INT frameLen = 80:			//Tamaño de la ventana de trama en muestras
 * 		INT npars = 10;				//Número de parámetros
 * 		INT ndeltas = 2;			//Número de derivadas a calcular (deriv. y aceleraciones)
 * 		INT wins [] = {2, 3};		//Tamaño de las ventanas para cálculo de deltas (en tramas)
 * 
 *		//Este objeto se encarga de todo el cálculo de deltas
 *		ParamDeltas deltas (ndeltas, wins, npars);
 * 
 *		//array para recoger una trama de muestras
 * 		DOUBLE* frame = new DOUBLE [frameLen];
 * 
 *		//array para recoger un vector de parámetros (sin deltas)
 * 		DOUBLE* params = new DOUBLE [npars];
 * 		
 *		//array para recoger un vector de parámetros junto con las deltas (concatenadas)
 * 		DOUBLE* finalparams = new DOUBLE [npars*(ndeltas+1)];
 * 		
 * 		for (INT i = 0; i < Nframes(); ++i)
 * 		{
 *			//Ahora ya no leemos más allá del final del archivo
 * 			frame = GetFrame(i);
 * 			params = GetParams(frame);
 * 
 *			//Metemos los parámetros en el sistema
 * 			deltas.Push(params);
 * 
 *			//Descartamos 2*deltas.Delay(), ya que así nos aseguramos que las deltas
 * 			//obtenidas se hayan calculado sin repetir la primera trama
 * 			if (i < 2*deltas.Delay())
 * 				continue;
 * 
 *			//Recuperamos la trama con deltas
 *			//Será la trama i-deltas.Delay()
 *			//Sólo que ahora i={2*deltas.Delay(),...,Nframes()}, lo que significa
 *			//que sólo tenemos tramas completas en el rango i={deltas.Delay(),...,Nframes()-deltas.Delay()}
 * 			deltas.Get(finalparams)
 * 
 *			//Hacemos algo con los parámetros
 * 			DoSomething(finalparams);
 * 		}
 * \endcode
 * 
 * \section delta_algorithm Algoritmo
 * 
 * Esta clase calcula las deltas como la pendiente de la recta de regresión de una
 * ventana centrada en la trama \p i. Digamos que estas ventanas tienen semi-anchura
 * N. Es decir, la anchura total de la ventana es de 2N+1.
 * 
 * Esto significa que para calcular la derivada de la trama \p i es necesario conover
 * las tramas \p i-N hasta \p i+N. 
 * 
 * Bien, supongamos ahora que no sólo calculamos las diferencias, sino también aceleraciones.
 * Las aceleraciones se calculan como la pendiente de la recta de regresión de las derivadas,
 * en otra ventana de tamaño M (posiblemente diferente a N)
 * 
 * Y podríamos calcular terceras, cuartas... diferencias. Digamos que la semi-anchura de cada
 * una de las ventanas es N[j] para la derivada j-ésima. Entonces se puede demostrar que
 * el retardo total en el cálculo (o lo que es lo mismo, cuánto por delante de la trama \p i 
 * debemos estar leyendo el archivo para poder calcular TODAS las derivadas de \p i) es igual
 * a la suma de todas las semi-ventanas: delay = sum(N[j]).
 * 
 */
class ParamDeltas
{
	
/*	El algoritmo hace algo como esto: Mantiene dos listas de buffers, uno para el cálculo
	de derivadas (con tamaño size[j] = 2*N[j]+1) y otro para recordar muestras pasadas, con
	tamaño size[j] = \sum_j^{Numdeltas)(N[j])+1. Este tamaño se corresponde con el retardo de cálculo
	de todas las derivadas superiores a ese nivel más uno. De esta forma, en la posición [0]
	de ese buffer siempre está el valor de la delta actual. Dicho de otra forma, si concatenamos
	todas las posiciones [0], logramos el vector completo i-Delay(). De hecho, esto es lo que
	hace Get().
	
	No hay buffer para la última delta. No le hace falta, pues no hay que calcular nada por encima
	de ella y no requiere memoria, ya que es la de mayor retardo.
*/

public:
	
	inline
	ParamDeltas ()
	: m_Ndim(0), m_bInit(false), m_pdDiff(NULL)
	{}


	/*!
	 * El número de deltas puede ser 0, pero ninguna ventana puede ser ni cero ni negativa,
	 * o se provoca un ASSERT.
	 *
	 * \p Ndeltas = 0 se trata como un caso especial
	 */
	inline
	ParamDeltas (INT Ndeltas, INT* windowSize, INT Ndim)
	{ Initialize(Ndeltas, windowSize, Ndim); }
	
	inline
	~ParamDeltas()
	{ if (m_pdDiff) xfree(m_pdDiff); }
	
	///Cambia el tamaño del buffer
	void
	Initialize (INT Ndeltas, INT* windowSize, INT Ndim);
			
	///Añade un nuevo vector de parámetros a los buffers de cálculo
	void
	Push (const DOUBLE* e);
	
	///Devuelve el retardo de cálculo
	INT
	Delay() const
	{ return m_vMemBuffers[0].Size()-1; }
	
	///Devuelve la dimensión de los parámetros
	inline
	INT
	Ndim() const
	{ return m_Ndim; }
	
	///Devuelve el número de deltas calculadas
	inline
	INT
	Ndeltas() const
	{ return m_vDeltaBuffers.size(); }
	
	///Devuelve un vector de parámetros con sus deltas
	void
	Get(DOUBLE* e) const;
	
	
protected:
	
	///Popula el primer buffer
	void
	Populate (const DOUBLE* e);

protected:
	std::vector<buffer2D> m_vDeltaBuffers;	///<Buffers usados para el cálculo de derivadas
	std::vector<buffer2D> m_vMemBuffers;	///<Buffers usados para recordar muestras pasadas
	INT m_Ndim;								///<Número de dimensiones de los parámetros (sin deltas)
	bool m_bInit;							///<Variable interna para saber si el objeto está inicializado
	DOUBLE* m_pdDiff;						///<Buffer temporal para cálculo de derivadas

};





///Calcula una diferencia en un buffer 2D
void
GetDiff	( const buffer2D& buffer, DOUBLE* diff );

///Calcula una diferencia en un buffer 1D
DOUBLE
GetDiff	( const buffer1D& buffer );

///Devuelve una muestra de un buffer 2D
/*!
	Sólo sirve para evitar un memcpy.

	\param buffer Buffer cuya muestra se quiere sacar.
	\param mid Memoria en la que se va a dejar la muestra. Debe tener buffer.Ndim() elementos
	\param n Índice de la muestra a extraer
*/
inline
void
GetSample ( const buffer2D& buffer, DOUBLE* mid, INT n)
{ memcpy(mid, buffer[n], buffer.Dim()*sizeof(DOUBLE)); }

///Devuelve la muestra central de un buffer 2D
/*!
	Permite devolver el valor de la muestra central de un buffer. Es necesario
	que le buffer tenga un valor impar de muestras, o provocará un ASSERT

	\param buffer Buffer cuya muestra central se quiere sacar.
	\return La muestra central del buffer
*/


///Devuelve una muestra de un buffer 1D
/*!
	No hace náda más que llamar a buffer[n]. Sólo está por simetría
 	con los buffer 2D

	\param buffer Buffer cuya muestra se quiere sacar.
	\param n Índice de la muestra a extraer
*/
inline
DOUBLE
GetSample ( const buffer1D& buffer, INT n)
{ return buffer[n]; }

	
#endif



