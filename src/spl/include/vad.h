#ifndef __VAD2_H__
#define __VAD2_H__

/////////////////////////////////////////////////////////////////////
/*!
   \file vad.h
   \brief Estimación de actividad vocal

	Copyright: 2010 - Grupo de Voz (DAET) ETSII/IT-Bilbao
	\author Iker Luengo
 	\version
	\date 04/03/10

	\if SKIP
	Version  dd/mm/aa  Autor     Proposito de la edicion
	-------  --------  --------  -----------------------
	1.0.0    04/03/10  ikerl     Codificación inicial
 	1.1.0    02/03/11  ikerl     Simplificación de la interfaz
	\endif
*/




#include <stdio.h>
#include <cassert>
#include "xfft.h"
#include "uti.h"
#include "xalloc.h"
#include "cbuffer.h"
#include "strl.hpp"



/*!
 * \brief Estimación de actividad vocal
 * 
 * \version
 * \code
 * 		Version  dd/mm/aa  Autor     Proposito de la edicion
 * 		-------  --------  --------  -----------------------
 * 		1.0.0    04/03/10  ikerl     Codificación inicial
 * 		1.1.0    02/03/11  ikerl     Simplificación de la interfaz
 * \endcode

 * 
 * 
 * Esta clase se encarga de gestionar un algoritmo de detección de actividad vocal
 * (VAD por sus siglas en inglés) basado en envolvente espectral a largo plazo (LTSE).
 * El algoritmo utilizado es una modificación del presentado en:
 * 
 * Ramirez, Javier and Segura, Jose C. and Benitez, Carmen and de la Torre, Angel and Rubio, Antonio,
 * "Efficient Voice Activity Detection Algorithms Using Long Term Speech Information",
 * Speech Communication, vol. 42, pp. 271-287, 2004.
 * 
 * El algoritmo modificado que está implementado puede encontrarse en:
 * 
 * Luengo, Iker and Navas, Eva and Odriozola, Igor and Saratxaga, Ibon and Hernaez, Inmaculada
 * and Sainz, Iñaki and Erro, Daniel
 * "Modiﬁed LTSE VAD algorithm for applications requiring reduced silence frame misclassiﬁcation",
 * Proc. of LREC 2010
 * 
 * Entre los dos artículos se debería tener una idea bastante maja de cómo va el algoritmo, por lo
 * que aquí sólo voy a tratar de aclarar cuestiones sobre su implementación y configuración. Sin embargo,
 * para entender qué representa cada parámetro de configuración, 
 * es necesario dar una visión general de cómo funciona este algoritmo. Ojo, es una descripción
 * muy poco rigurosa, no hay que tomarse todo al pié de la letra. Sólo se explica lo estrictamente
 * necesario.
 * 
 * La LTSE de una trama es una aproximación de la envolvente espectral de la trama, obtenida a partir
 * de una ventana de 2N+1 tramas, donde la trama central de la ventana es la trama actual. Esta ventana
 * es lo que vamos a llamar 'ventana LTSE'. Calculada la LTSE de la trama actual,
 * se compara esta LTSE con el espectro estimado del ruido. Si la diferencia es superior a un
 * umbral \f$ \gamma \f$, se considera que la trama contiene voz. Si no, se considera que sólo es ruido.
 * 
 * El umbral \f$ \gamma \f$ depende de la SNR estimada de la señal. Además, por cada trama que se
 * clasifica como silencio, se actualiza el espectro estimado de ruido y la potencia estimada de ruido.
 * Igualmente, por cada trama clasificada como voz se actualiza la potencia estimada de señal. Con esto,
 * la SNR también se va actualizando.
 * 
 * Por tanto, los elementos clave de este algoritmo son:
 * 
 * 	- La ventana de análisis LTSE, de 2N+1 muestras
 * 	- El espectro estimado del ruido
 * 	- La potencia estimada de ruido y de señal
 * 	- La SNR estimada (que se calcula a partir de las anteriores)
 * 	- El cálculo del umbral.
 * 
 * 
 * 
 * \section vad_use Utilización del VAD
 * 
 * El algoritmo consta de de unas tres partes: modelado de ruido, inicializacion del buffer LTSE
 * y el procesado VAD propiamente dicho. Además, antes de todo esto, es necesario configurar el
 * objeto con los parámetros adecuados.
 * 
 * \par Inicialización y configuración
 * 
 * Prácticamente toda la configuración que necesita la clase se da mediante una lista de parámetros
 * KVStrList. Esta lista puede generarse a partir de la línea de comandos o a través de un
 * archivo de configuración mediante la función clargs2props. La siguiente lista describe brevemente
 * cada uno de los parámetros definidos, junto con su valor por defecto. Cada un ode los parámetros se
 * describe con mayor detalle según va apareciendo en esta documentación.
 * 
 * \code
 * 		#Tipo de venta a utilizar: Bart | Hanning | Hamming | Black | Rect | None
 * 		#(string)
 * 		-WindowType=Hamming
 * 
 * 		#Número de tramas utilizadas para la inicialización
 * 		#del nivel de ruido.
 * 		#(int)
 * 		-Ninit=15
 * 
 * 		#Tamaño de la ventana de análisis Long Term (en tramas). Debe ser impar
 * 		#(int)
 * 		-LTWindowLen=13
 * 
 * 		#Mínima SNR considerada (dB).
 * 		#(float)
 * 		-SNR0=5
 * 
 * 		#Máxima SNR considerada (dB).
 * 		#(float)
 * 		-SNR1=20
 * 
 * 		#Umbral asociado a SNR0.
 * 		#(float)
 * 		-gamma0=8
 * 
 * 		#Umbral asociado a SNR1.
 * 		#(float)
 * 		-gamma1=15
 * 
 * 		#Desplazamiento del umbral para decisión voz/silencio
 * 		#(float)
 * 		-offset=0
 * 
 * 		#Coeficiente de recuerdo en al actualización del espectro de ruido.
 * 		#(float)
 * 		-Nalfa=0.99
 * 
 * 		#Coeficiente de recuerdo en al actualización de las potencias.
 * 		#(float)
 * 		-SNRalfa=0.99
 * 
 * 		#SNR aproximada de la señal.
 * 		#(float)
 * 		-SNRstart=(SNR0+SNR1)/2
 * 
 * 		#Número de tramas usadas en HandOver.
 * 		#(int)
 * 		-Handover=0
 * 
 * 		#Tamaño de la ventana de actualización de potencias 
 * 		#y espectro (en tramas).
 * 		#(int)
 * 		-ActWindowLen=7
 * \endcode
 * 
 * 
 * \note Las tramas que se dan al sistema no tienen que estar enventanadas, el propio VAD ya aplica la ventan
 * correcta indicada por \p WindowType. En caso de que las tramas ya estén enventanadas, será necesario
 * indicar \p WindowType=None, para evitar que se enventane dos veces. Por defecto, se utiliza ventana de Hamming.
 * 
 * El objeto se inicializa mediante una llamada a la función Reset(), que toma como parámetros la lista KVStrList
 * y la frecuencia de muestreo de la señal que se está procesando:
 * 
 * \code
 * 		vad vad_object;			//Constructor por defecto
 * 
 *		//De alguna forma, creamos los parámetros de configuración
 * 		KVStrList params;
 * 		double Srate;
 *		//...
 * 
 *		//Inicializamos el objeto
 * 		vad_object.Reset(params, Srate);
 * 
 *		//...
 * \endcode
 * 
 * En cualquier momento del programa se puede volver a llamar a Reset(), lo que hace que el objeto se
 * resetee y quede como si nunca se hubiera usado. Esto implica borrar las memorias internas y el modelo
 * de ruido. Por ejemplo, es necesario resetear el objeto si pasamos a procesar otro archivo,
 * o queremos procesar el mismo otra vez.
 * 
 * \code
 * 		vad vad_object;			//Constructor por defecto
 * 
 *		//De alguna forma, creamos los parámetros de configuración
 * 		KVStrList params;
 *		//...
 * 
 * 		for (int i = 0; i < NumFicheros; ++i)
 * 		{
 *			//Hay que resetear el VAD para cada fichero
 * 			double Srate = getSrate(i);
 * 			vad_object.Reset(params, Srate);
 * 
 *			//...
 * 		}
 * \endcode
 * 
 * \note Es importante es recordar que un objeto construido por el constructor por defecto
 * NO ESTÁ INICIALIZADO, y por tanto, es inutil. Es necesario realizar una llamada a Reset() o utilizar
 * el constructor inicializador, el cual llama a Reset() indirectamente.
 * 
 * \par Modelado de ruido
 * 
 * Todo el sistema se basa en tener un modelo de ruido contra el que comparar la LTSE de cada trama para
 * decidir si es ruido o no. El modelo de ruido utilizado por el sistema es relativamente sencillo.
 * Se toman \p Ninit tramas que SABEMOS que no contiene voz, sólo ruido. El sistema calcula el espectro medio
 * y la potencia de estas tramas, y eso es lo que forma el modelo.
 * 
 * Para ello el objeto debe saber qué tramas son las que debe utilizar para el modelado. Tenemos dos opciones:
 * Dejar que el sistema se las arregle y despreocuparnos del asunto, o indicarle expresamente cuáles son las trmas
 * que debe utilizar para el modelado.
 * 
 * La opción más sencilla es dejar que el sistema se las arregle. En esta implementación esto significa que
 * el sistema toma las primeras \p Ninit tramas que se le pasan mediante
 * doVAD() después de un Reset() (lo que en general corresponderá con las primeras \p Ninit tramas de la señal),
 * supone que son ruido, y las utiliza para inicializar el modelo. Durante estas \p Ninit primeras tramas, la decisión
 * será invariablemente 'silencio', y la verdadera detección empezará en la \p Ninit + 1.
 * 
 * \code
 * 		vad vad_object;
 * 
 * 		KVStrList params;
 * 		double Srate;
 *		//...
 * 		vad_object.Reset(params, Srate);
 * 
 *		//Empezamos a procesar normalmente, pero las primeras Ninit tramas serán 'silencio'
 *		//y esperemos que realmente no tuvieran voz
 * 		for (int i = 0; i < Nframes; ++i)
 * 		{
 * 			double* frame = GetFrame(i);
 * 			bool isVoice = vad_object.doVAD(frame);
 *			//...
 * 		}
 * \endcode
 * 
 * Esta solución sencilla es suficiente para la mayoría de los casos. Sin embargo, si no estamos seguros de que el
 * inicio del archivo contiene un silencio, o si disponemos de algún trozo de señal que por lo que sea sabemos que
 * sólo contiene ruido de fondo, puede ser conveniente indicar expresamente cuáles son las tramas a utilizar para el
 * modelo. Para ello se utiliza la función NoiseInit().
 * 
 * En caso de llamar a NoiseInit(), debe hacerse justo después de Reset(), y siempre antes de iniciar
 * el procesado normal del archivo. 
 * NoiseInit() toma como entrada una trama, y esa trama se utilizará para el modelado del ruido. Se puede llamar
 * a NoiseInit() tantas veces como se quiera, aunque lo lógico es que se llame al menos \p Ninit veces. 
 * Todas las tramas que se le pasen reforzarán el modelo de ruido, y cuantas más tramas haya para el modelo,
 * más robusto será.
 * 
 * Si por lo que sea el número de veces que se ha llamado a NoiseInit() es inferior a \p Ninit, las tramas que
 * faltan para la inicialización del ruido las cogerá del inicio del archivo, y si estas tramas no son de silencio,
 * el sistema estará mal inicializado. Para facilitar las cosas, la clase proporciona la función
 * MissingNoiseInitFrames(), que devuelve el número de tramas que faltan para completar \p Ninit.
 * 
 * \code
 * 		vad vad_object;
 * 
 * 		KVStrList params;
 * 		double Srate;
 *		//...
 * 		vad_object.Reset(params, Srate);
 * 
 *		//GetNoiseFrameNumber y GetNoiseFrame son funciones del usuario
 * 		int noise_frame_number = GetNoiseFrameNumber()
 * 		for (int i = 0; i = noise_frame_number; ++i)
 * 		{
 *			//Tomamos la trama y se la damos al sistema
 * 			double* noise_frame = GetNoiseFrame(i);
 * 			vad_object.NoiseInit(noise_frame);
 * 		}
 *		//Empezamos a procesar normalmente ahora que tenemos el ruido inicializado
 * \endcode
 * 
 * \note Aunque no hay ninguna condición, se recomienda que la inicialización del ruido se haga con
 * unas 10 a 20 tramas, para tener una estimación bastante robusta. Como regla general se
 * considera que 15 están bien. Este es el valor por defecto de \p Ninit.
 * 
 * \note No se debe llamar a NoiseInit() una vez que se ha empezado a procesar el archivo normalmente, ya que
 * 		dejaría el objeto en un estado incoherente.
 * 
 * 
 * \par Inicializacion del buffer LTSE
 * 
 * Ya se ha indicado que el algoritmo se basa en una LTSE calculada a lo largo de una ventana de 2N+1 tramas
 * (N tramas por delante y por detrás de la actual). Internamente esta ventana se implementa como un buffer
 * de 2N+1 tramas, lo que tiene dos implicaciones directas:
 * 
 * 	- La trama para la cual se está dando la decisión NO ES LA TRAMA QUE SE ACABA DE PASAR AL VAD.
 * 	  La decisión se da siempre para N tramas anteriores.
 * 	- Es necesario llenar el buffer LTSE al menos hasta la mitad para empezar a recuperar resultados.
 * 
 * Dicho de otra forma, si tomamos un archivo de audio y en un momento dado estamos leyendo la trama
 * i-ésima (i=1,...,NFrames), y se la pasamos al VAD (mediante doVAD()), el resultado devuelto es la decisión para
 * la trama i-N.
 * 
 * Si ahora suponemos que i < N, entonces estará devolviendo el resultado para una trama i-N < 0, lo cual
 * no existe. Por lo tanto, debemos llenar el buffer con N tramas antes de poder recuperar la primera decisión.
 * Las primeras N veces que se llame a doVAD(), el sistema no hace nada más que poblar el buffer,
 * y la función siempre devuelve 'silencio'. A partir de ahí empieza a devolver resultados.
 * 
 * El tamaño de la ventana de análisis LTSE se indica mediante \p LTWindowLen, y debe ser un valor impar. 
 * El retardo de la decisión puede recuperarse mediante la función Delay(), la cual devuelve
 * (\p LTWindowLen - 1)/2:
 * 
 * \code
 * 		KVStrList params;
 * 		double Srate;
 *		//...
 * 		vad_object.Reset(params, Srate);
 * 		vad_results[Nframes];
 * 
 *		//... -> Inicialización del ruido
 * 
 * 		for (int i = 0; i < Nframes; ++i)
 * 		{
 * 			double* frame = GetFrame(i);
 * 			if (i < vad_object.Delay()
 * 				//El resultado no sirve, pero hay que pasarle la trama al objeto
 * 				vad_object.doVAD(frame);
 * 			else
 *				//El resultado es válido, pero con un retardo
 * 				vad_results[i-vad.Delay()] = vad_object.doVAD(frame);
 *			//...
 * 		}
 * \endcode
 * 
 * \note Alternativamente, para saber si el objeto ya tiene los buffers llenos y el resultado devuelto es
 * 		válido, se puede usar IsBufferFull(), que devuelve true cuando se ha completado la inicialización
 * 		de los buffers. Dicho de otra forma, IsBufferFull() devuelve true si ya se ha llamado a doVAD()
 * 		Delay() veces.
 * 
 * \section vad_threshold Umbral adaptativo
 * 
 * El umbral de LTSD varía según la SNR estimada. Esta variación es lineal: a mayor SNR, mayor es el umbral.
 * Pero existen unos límites de saturación:
 * 
 * 	- Si SNR < \p SNR0 entonces \f$ \gamma \f$ = \p gamma0
 * 	- Si SNR > \p SNR1 entonces \f$ \gamma \f$ = \p gamma1
 * 	- En cualquier otro caso, \f$ \gamma \f$ toma un valor lineal entre esos puntos.
 * 
 * Además, para permitir personalizar el punto de trabajo, se permite aplicar un offset a la LTSD
 * (configurado mediante la opción \p Offset).
 * Valores positivos del offset hacen que se favorezca la decisión 'voz'. Valores negativos hacen
 * que se tienda más a la decisión 'silencio'. Con esto es posible modificar el comportamiento del sistema
 * según si es más importante que no haya tramas de silencio clasificadas como voz o tramas de voz clasificadas
 * como silencio.
 * 
 * Al comienzo del algoritmo, y después de la inicialización del ruido, el sistema tiene una estimación de la
 * potencia de ruido, pero no así de la potencia de la señal, por lo que no puede estimar la SNR inicial.
 * Es muy conveniente proporcionar una estimación inicial de la SNR de la señal mediante \p SNRinit, para ajustar
 * el punto de trabajo del algoritmo. Si este valor no se proporciona, se toma como defecto (\p SNR1 + \p SNR0)/2,
 * lo cual en muchos casos puede no ser una buena estimación.
 * 
 * Esta SNR inicial puede calcularse mediante un pre-procesado de la señal, o en el peor de los casos, estimarse
 * a ojo. En señales muy limpias o muy ruidosas, esta estimación a ojo será mejor que el valor por defecto.

 * 
 * \section vad_adaptative Adaptación del ruido
 * 
 * El espectro del ruido, potencia de ruido y potencia de la señal se van adaptando a medida que las
 * tramas se van clasificando como 'voz' o 'silencio'. Esta adaptación viene controlada por varias opciones de
 * configuración.
 * 
 * Cuando una trama se clasifica como 'voz', se toman \p ActWindowLen tramas anteriores a ella y se calcula
 * la potencia media de señal en estas tramas. La potencia de señal se actualiza como:
 * 
 * \f[
 * 		P_S = \alpha_{SNR} \cdot P_S + (1-\alpha_{SNR}) \cdot \hat{P_S}
 * \f]
 * 
 * donde \f$ P_S \f$ es la potencia de señal, \f$ \alpha_{SNR} \f$ es el coeficiente de recuerdo indicado
 * por la opción \p SNRalfa, y \f$ \hat{P_S} \f$ es la potencia media estimada de las \p ActWindowLen
 * tramas anteriores.
 * 
 * Igualmente, por cada trama que se clasifica como 'silencio', se hace algo parecido con la potencia de ruido.
 * 
 * \f[
 * 		P_N = \alpha_{N} \cdot P_N + (1-\alpha_{N}) \cdot \hat{P_N}
 * \f]
 * 
 * donde \f$ P_N \f$ es la potencia de ruido, \f$ \alpha_{N} \f$ es el coeficiente de recuerdo indicado
 * por la opción \p Nalfa, y \f$ \hat{P_N} \f$ es la potencia media estimada de las \p ActWindowLen
 * tramas anteriores.
 * 
 * En el caso del ruido se actualiza además el espectro del modelo:
 * \f[
 * 		F_N[n] = \alpha_{N} \cdot F_N[n] + (1-\alpha_{N}) \cdot \hat{F_N[n]}
 * \f]
 * 
 * donde \f$ F_N[n] \f$ es el valor del espectro de ruido en la frecuencia n. En este caso se utiliza el mismo
 * coeficiente de olvido \p Nalfa.
 * 
 * 
 * \section vad_handover Handover
 * 
 * El handover es una técnica para retrasar el paso de la decisión 'voz' a la decisión 'silencio'. Su origen
 * está en los sistemas de reconocimiento automático del habla, donde es muy importante que no haya tramas
 * de voz clasificadas como silencio, para no perder información. Para ello, cuando hay una serie de tramas
 * clasificadas como 'voz', y de pronto llega una que se clasifica como 'silencio', la decisión de 'voz' se
 * mantiene durante \p Handover tramas de forma artificial. Si todas esas tramas se clasifican como 'silencio',
 * entonces se realiza la transición y el elgoritmo sigue.
 * 
 * 
 */
class vad
{

public:
	
/*!\name Interfaz básica
 * 
 * Las funciones que se usarán habitualmente
 */
//\{

	///Constructor
	/*!
	 * Antes de utilizar el objeto será necesario llamar a Reset() para
	 * inicializar los valores de configuración
	 */
	inline
	vad()
	: m_fftObject(NULL), m_NoiseFFT(NULL), m_NoiseFFTAct(NULL), m_LTSE(NULL)
	{ /*Deben iniciarse los punteros a NULL para que el Reset() no tenga problemas*/ }
	///Constructor
	/*!
	 * Inicializa los valores de configuración llamando a Reset() automáticamente
	 */
	inline
	vad (const KVStrList& cfg, DOUBLE SRate)
	: m_fftObject(NULL), m_NoiseFFT(NULL), m_NoiseFFTAct(NULL), m_LTSE(NULL)
	/*Deben iniciarse los punteros a NULL para que el Reset() no tenga problemas*/
	{ Reset(cfg, SRate); }

	///Destructor
	inline
	~vad()
	{ Destruct(); }
	
	///Inicializa los valores de configuración y la memoria reservada
	bool
	Reset (const KVStrList& cfg, DOUBLE Srate);
	
	///Utiliza una trama para la inicialización de ruido
	void
	NoiseInit (pDOUBLE frame);

	///Realiza la decisión VAD sobre la trama
	bool
	doVAD (pDOUBLE frame);

	///Devuelve el reatardo de la decisión (en tramas)
	inline
	INT
	Delay() const
	{ return (m_LTWindowLen-1)/2; }
	
//\}
	
/*!\name get-ters y set-ters
 * 
 * Es seguro llamar a estas funciones incluso después de haber inicializado el objeto.
 * 
 * Seguro en el sentido de que no dejará el objeto en un estado incoherente. Sin embargo,
 * es mejor no utilizarlas una vez completada la inicialización si no se está seguro de lo que se está haciendo.
 */
//\{
	
	///Número de muestras que deben tener las tramas
	inline
	LONG
	getFrameLen () const
	{ return m_FrameLen; }
	
	///Número de puntos FFT que se están usando.
	/*!
	 * será la siguiente potencia de dos de getFrameLen().
	 * 
	 * Bueno, en realidad, la mitad de la siguiente potencia de dos, ya que
	 * al ser simétrica, sólo devuelve medio espectro.
	 */
	inline
	INT
	getNfft () const
	{ return m_Nfft; }

	
	///Número de tramas usadas para análisis LongTerm
	inline
	INT
	getLTWindowLen () const
	{ return m_LTWindowLen; }

	///El estado del buffer
	/*!
	 * Devuelve verdadero si el buffer ya está suficientemente lleno como para 
	 * empezar a calcular cosas
	 */
	inline
	bool
	IsBufferFull () const
	{ return (m_bufferedFrames >= (m_LTWindowLen-1)/2); }
	
	///Potencia estimada del ruido en este momento
	inline
	DOUBLE
	getNoisePow () const
	{ return m_NoisePow; }
	
	inline
	void
	setNoisePow (DOUBLE np)
	{ m_NoisePow = np; }
	
	///Potencia estimada de la señal en este momento
	inline
	DOUBLE
	getSignalPow () const
	{ return m_SignalPow; }

	inline
	void
	setSignalPow (DOUBLE sp)
	{ m_SignalPow = sp; }
	
	///SNR estimada en este momento
	inline
	DOUBLE
	getSNR () const
	{ return m_SNR; }

	/*!
	 * Puede servir para inicializar la SNR inicial si todavía no se
	 * conocía cuando se ha hecho el Reset()
	 */
	inline
	void
	setSNR (DOUBLE snr)
	{ m_SNR = snr; }

	///Límite inferior de SNR
	inline
	DOUBLE
	getSNR0 () const
	{ return m_SNR0; }

	inline
	void
	setSNR0 (DOUBLE snr0)
	{ m_SNR0 = snr0; }
	
	///Límite superior de SNR
	inline
	DOUBLE
	getSNR1 () const
	{ return m_SNR1; }

	inline
	void
	setSNR1 (DOUBLE snr1)
	{ m_SNR1 = snr1; }
	
	///Umbral de SNR0
	inline
	DOUBLE
	getGamma0 () const
	{ return m_gamma0; }

	inline
	void
	setGamma0 (DOUBLE gamma0)
	{ m_gamma0 = gamma0; }
	
	///Umbral de SNR1
	inline
	DOUBLE
	getGamma1 () const
	{ return m_gamma1; }

	inline
	void
	setGamma1 (DOUBLE gamma1)
	{ m_gamma1 = gamma1; }
	
	///Número de tramas usadas para actualización de ruido y señal
	inline
	INT
	getActWindowLen () const
	{ return m_ActWindowLen; }

	inline
	void
	setActWindowLen (INT ActWindowLen)
	{ assert ((m_LTWindowLen-1/2) >= ActWindowLen);
	  m_ActWindowLen = ActWindowLen; }

	///Coeficiente de memoria para la actualización del ruido
	inline
	FLOAT
	getNalfa () const
	{ return m_Nalfa; }

	inline
	void
	setNalfa (FLOAT Nalfa)
	{ m_Nalfa = Nalfa; }
	
	///Coeficiente de memoria para la actualización del la señal
	inline
	FLOAT
	getSNRalfa () const
	{ return m_SNRalfa; }

	inline
	void
	setSNRalfa (FLOAT SNRalfa)
	{ m_SNRalfa = SNRalfa; }
	
	///offsest en la decisión
	inline
	DOUBLE
	getOffset () const
	{ return m_offset; }

	inline
	void
	setOffset (DOUBLE offset)
	{ m_offset = offset; }
	
	///tramas de handover
	inline
	INT
	getHandover () const
	{ return m_Handover; }

	inline
	void
	setHandover (INT Handover)
	{ m_Handover = Handover; }
	
	///Número de tramas utilizadas hasta ahora para la inicialización del ruido
	inline
	INT
	getNoiseInitCounter () const
	{ return m_NoiseInitCounter; }
	
	///Número de tramas a utilizar en total para la inicialización del ruido
	/*!
	 * Devuelve el valor del parámetro \p Ninit
	 */
	inline
	INT
	getNoiseInitFrames () const
	{ return m_NoiseInitFrames; }

	///Número de tramas que faltan para la inicialización del ruido.
	/*!
	 * Literalmente devuelve getNoiseInitFrames() - getNoiseInitCounter().
	 * 
	 * Puede devolver valores negativos, lo que significa que no sólo se le han pasado suficientes
	 * tramas para inicializar el ruido, sino que se han dado de más. Eso no es grave, más robusto
	 * será el modelo.
	 */
	inline
	INT
	MissingNoiseInitFrames () const
	{ return m_NoiseInitFrames - m_NoiseInitCounter; }

//\}

		
		
		
protected:
		
	///Destruye el objeto
	void
	Destruct();
	
	///Calcula el umbral de LTSD en función de la SNR estimada
	DOUBLE
	Gamma() const;
	
	///Actualiza el modelo de señal
	void
	ActualizeSignal();
	
	///Actualiza el modelo de ruido
	void
	ActualizeNoise();
	
	///Añade una trama nueva al buffer de la LTSE
	void
	AddToBuffer (pDOUBLE frame);
	
	///Calcula la LTSE del contenido del buffer
	void
	LTSE ();
	
	///Calcula la LTSD a partir de la LTSE y el espectro de ruido
	DOUBLE
	LTSD () const;


	
protected:
		
/*!\name Variables asociadas con el enventantado y el análisis FFT
 */
//\{
	LONG m_FrameLen;			///<Longitud de la trama en muestras
	XFFT_WIN m_WinType;			///<Tipo de ventana
	pRFFT m_fftObject;			///<Objeto de análisis FFT
	INT m_Nfft;					///<Número de puntos de las FFT
	buffer2D m_memFFT;			///<Array circular de FFT necesario para el cálculo de LTSE
								//Tendrá LTWindowLen elementos
	INT m_bufferedFrames;		///<Auxiliar para saber si ya se ha llenado la mitad del buffer;
//\}

/*!\name Variables asociadas con la señal y el ruido estimados
 */
//\{
	pDOUBLE m_NoiseFFT;			///<Espectro medio del ruido
	INT m_NoiseInitFrames;		///<Número de tramas a usar para la inicialización del ruido
	INT m_NoiseInitCounter;		///<Número de tramas para la inicialización del ruido hasta ahora
	DOUBLE m_NoisePow;			///<Potencia estimada del ruido
	DOUBLE m_SignalPow;			///<Potencia estimada de la señal
	DOUBLE m_SNR;				///<SNR estimada
	DOUBLE m_SNR0;				///<Mínima SNR considerada
	DOUBLE m_SNR1;				///<Máxima SNR considerada
	INT m_ActWindowLen;			///<Número de tramas usadas para actualización de ruido y señal
	FLOAT m_Nalfa;				///<Coeficiente de memoria para la actualización del ruido
	FLOAT m_SNRalfa;			///<Coeficiente de memoria para la actualización de la SNR
	pDOUBLE m_NoiseFFTAct;		///<Usado durante la actualización del espectro de ruido
								//Si hago que se reserve memoria en la inicialización, adelantamos trabajo
//\}

/*!\name Variables asociadas con el algoritmo LTSD
 */
//\{
	INT m_LTWindowLen;			///<Número de tramas usadas para análisis LongTerm
	DOUBLE m_gamma0;			///<Umbral asociado a SNR0
	DOUBLE m_gamma1;			///<Umbral asociado a SNR1
	INT m_Handover;				///<Número de tramas usadas para Hand Over
	LONG m_NumSil;				///<Número de tramas silenciosas hasta ahora (para actualizar espectro)
	LONG m_NumVoice;			///<Número de tramas sonoras hasta ahora (para actualizar espectro)
	LONG m_HoSil;				///<Contador para HandOver
	DOUBLE m_offset;			///<Offset para la LTSD
	pDOUBLE m_LTSE;				///<Usado durante el cálculo de la LTSE
								//Si hago que se reserve memoria en la inicialización, adelantamos trabajo
//\}

};


#endif

