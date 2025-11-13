#ifndef __CBUFFER_H__
#define __CBUFFER_H__

/////////////////////////////////////////////////////////////////////
/*!
   \file cbuffer.h
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




#include <stdio.h>
#include <cassert>
#include "xfft.h"
#include "uti.h"
#include "xalloc.h"


///Clase para implementar un buffer 2D. Generalmente, un buffer de tramas parametrizadas
/*!
 * Esta clase está poco definida, y es muy rígida. Define un array
 * bidimensional donde una de las dimensiones es circular y la otra
 * es fina. Se supone que la dimensión fija es el tamaño de los vectores
 * de parámetros y la circular es el tiempo.
 *  
 * Un ejemplo sencillo del uso de esta clase:
 *  
 * \code
 * 		INT BufSize = 10;	//Tamaño del buffer. Dimensión circular
 * 		INT fftsize;		//Tmaño de los vectores. Dimensión fija
 * 		buffer2D fftbuf(BufSize, fftsize);
 *  
 * 		//Llenamos el buffer con datos
 * 		pDOUBLE fft;
 * 		for (INT i = 0; i < BufSize; ++i)
 * 		{
 * 			fft = GetNewFft();
 * 			fftbuf.Push(fft);
 * 		}
 *  
 * 		//Ahora trabajamos con el buffer
 * 		//Se puede utilizar el operator[] de forma normal
 * 		DoSomething(fftbuf);
 *  
 * 		//Y podemos meter nuevos elementos.
 *		//Cuando el buffer se llene, el elemento más viejo se borra para
 *		//dar espacio al nuevo.
 * 		fft = GetNewFft();
 * 		fftbuf.Push(fft);
 * \endcode
 */
class buffer2D
{
protected:
	DOUBLE** m_buffer;	///<La memoria del buffer
	INT m_Nelem;		///<Tamaño del buffer
	INT m_Ndim;			///<Número de dimensiones
	INT m_head;			///<índice a cabeza (elemento más viejo)
	INT m_tail;			///<índice a cola (elemento más nuevo)
	//Si m_tail == -1 es que el buffer está vacío.
	//Si m_head == (m_tail+1)%m_Nelem es que el buffer está lleno del todo

public:
	
	inline
	buffer2D ()
	: m_buffer(NULL), m_Nelem (0), m_Ndim(0), m_head (0), m_tail (-1)
	{}
	
	buffer2D (INT Nelem, INT Nfft);
	
	~buffer2D();
	
	///Cambia el tamaño del buffer
	void
	Resize (INT Nelem, INT Nfft);
			
	///Sobrecarga del operador []
	/*!
	 * Permite abstraerse de la naturaleza circular del buffer
	 * 
	 * \note No comprueba si idx está dentro de los límites del buffer o no. Esto
	 * 		es coherente con el comportamiento de un array C estándar.
	 */
	inline
	const DOUBLE*
	operator[] (INT idx) const
	{ return m_buffer[(m_head + idx)%m_Nelem]; }
	
	///Sobrecarga del operador []
	/*!
	 * Permite abstraerse de la naturaleza circular del buffer
	 * 
	 * \note No comprueba si idx está dentro de los límites del buffer o no. Esto
	 * 		es coherente con el comportamiento de un array C estándar.
	 */
	inline
	DOUBLE*
	operator[] (INT idx)
	{ return m_buffer[(m_head + idx)%m_Nelem]; }
	
	///Añade un nuevo vector de parámetros al buffer
	inline
	void
	Push (const DOUBLE* e)
	{
		//Lo primero, avanzar el puntero de cola
		Advance();

		//Ahora copiamos
		memcpy (m_buffer[m_tail], e, m_Ndim*sizeof(DOUBLE));
	}
	
	///Devuelve el tamaño del buffer
	inline
	INT
	Size() const
	{ return m_Nelem; }
	
	///Devuelve la dimensión del buffer
	inline
	INT
	Dim() const
	{ return m_Ndim; }
	
	///Verdadero si el buffer está vacío
	inline
	bool
	Empty() const
	{ return m_tail == -1; }
	
	///Devuelve el elemento más viejo del buffer
	inline
	const DOUBLE*
	Head () const
	{ return m_buffer[m_head]; }

	///Devuelve el elemento más viejo del buffer
	inline
	void
	Head (DOUBLE* h) const
	{ memcpy(h, m_buffer[m_head], m_Ndim*sizeof(DOUBLE)); }

	///Devuelve el elemento más nuevo del buffer
	inline
	const DOUBLE*
	Tail () const
	{ return m_buffer[m_tail]; }

	///Devuelve el elemento más viejo del buffer
	inline
	void
	Tail (DOUBLE* t) const
	{ memcpy(t, m_buffer[m_tail], m_Ndim*sizeof(DOUBLE)); }

	
protected:
	
	///Avanza el puntero de cola
	inline
	void
	Advance ()
	{
		if (m_tail >= 0 && m_head == (m_tail+1)%m_Nelem)
			//Buffer lleno, debemos eliminar un elemento de cabeza
			m_head = (m_head+1)%m_Nelem;

		m_tail = (m_tail+1)%m_Nelem;
	}

};


///Clase para implementar un buffer 1D. Generalmente, de muestras de audio
/*!
 * Clase equivalente a buffer2D, pero para muestras unidimensionales.
 * La ventaja es que requiere una dimensión menos. Por lo demás,
 * funciona exactamente igual.
 * 
 * Un ejemplo sencillo del uso de esta clase:
 *  
 * \code
 * 		INT BufSize = 10;	//Tamaño del buffer. Dimensión circular
 * 		buffer1D samplebuf(BufSize);
 *  
 * 		//Llenamos el buffer con datos
 * 		DOUBLE sample;
 * 		for (INT i = 0; i < BufSize; ++i)
 * 		{
 * 			sample = GetNewSample();
 * 			samplebuf.Push(sample);
 * 		}
 *  
 * 		//Ahora trabajamos con el buffer
 * 		//Se puede utilizar el operator[] de forma normal
 * 		DoSomething(samplebuf);
 *  
 * 		//Y podemos meter nuevos elementos.
 *		//Cuando el buffer se llene, el elemento más viejo se borra para
 *		//dar espacio al nuevo.
 * 		sample = GetNewSample();
 * 		samplebuf.Push(sample);
 * \endcode
 */
class buffer1D
{
protected:
	DOUBLE* m_buffer;	///<La memoria del buffer
	INT m_Nelem;		///<Tamaño del buffer
	INT m_head;			///<índice a cabeza (elemento más viejo)
	INT m_tail;			///<índice a cola (elemento más nuevo)
	//Si m_tail == -1 es que el buffer está vacío.
	//Si m_head == (m_tail+1)%m_Nelem es que el buffer está lleno del todo

public:
	
	inline
	buffer1D ()
	: m_buffer(NULL), m_Nelem (0), m_head (0), m_tail (-1)
	{}
	
	inline
	buffer1D (INT Nelem)
	: m_Nelem (Nelem), m_head (0), m_tail (-1)
	{ m_buffer = (DOUBLE*)xmalloc (Nelem*sizeof(DOUBLE)); }

	inline
	~buffer1D()
	{ if (m_buffer) xfree(m_buffer); }

	
	///Cambia el tamaño del buffer
	void
	Resize (INT Nelem);
			
	///Sobrecarga del operador []
	/*!
	 * Permite abstraerse de la naturaleza circular del buffer
	 * 
	 * \note No comprueba si idx está dentro de los límites del buffer o no. Esto
	 * 		es coherente con el comportamiento de un array C estándar.
	 */
	inline
	const DOUBLE
	operator[] (INT idx) const
	{ return m_buffer[(m_head + idx)%m_Nelem]; }
	
	///Sobrecarga del operador []
	/*!
	 * Permite abstraerse de la naturaleza circular del buffer
	 * 
	 * \note No comprueba si idx está dentro de los límites del buffer o no. Esto
	 * 		es coherente con el comportamiento de un array C estándar.
	 */
	inline
	DOUBLE
	operator[] (INT idx)
	{ return m_buffer[(m_head + idx)%m_Nelem]; }
	
	///Añade un nuevo vector de parámetros al buffer
	inline
	void
	Push (DOUBLE e)
	{
		//Lo primero, avanzar el puntero de cola
		Advance();
		
		//Ahora copiamos
		m_buffer[m_tail] = e;
	}
	
	///Devuelve el tamaño del buffer
	inline
	INT
	Size() const
	{ return m_Nelem; }
	
	///Verdadero si el buffer está vacío
	inline
	bool
	Empty() const
	{ return m_tail == -1; }
	
	///Devuelve el elemento más viejo del buffer
	inline
	DOUBLE
	Head () const
	{ return m_buffer[m_head]; }

	///Devuelve el elemento más nuevo del buffer
	inline
	DOUBLE
	Tail () const
	{ return m_buffer[m_tail]; }

	
protected:
	
	///Avanza el puntero de cola
	inline
	void
	Advance ()
	{
		if (m_tail >= 0 && m_head == (m_tail+1)%m_Nelem)
			//Buffer lleno, debemos eliminar un elemento de cabeza
			m_head = (m_head+1)%m_Nelem;
		m_tail = (m_tail+1)%m_Nelem;
	}

};


#endif



