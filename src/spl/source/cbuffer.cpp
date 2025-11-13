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

#include "cbuffer.h"

buffer2D::buffer2D (INT Nelem, INT Nfft)
: m_Nelem (Nelem), m_Ndim(Nfft), m_head (0), m_tail (-1)
{ 
	m_buffer = (DOUBLE**)xmalloc (Nelem*sizeof(DOUBLE*));
	for (INT i = 0; i < m_Nelem; ++i)
		m_buffer[i] = (DOUBLE*)xmalloc (Nfft*sizeof(DOUBLE));
}
		
buffer2D::~buffer2D()
{
	if (m_buffer)
	{
		for (INT i = 0; i < m_Nelem; ++i)
			xfree(m_buffer[i]);
		 xfree(m_buffer);
	}
}
		
/*!
* Una llamada a esta función destruye todos los datos del buffer,
* por lo que hay que llamarla con cuidado. Además, ten en cuenta
* que destruye el buffer, pero no libera los arrays a los que apunta.
* 
* Sólo debería llamarse a esta función si se ha utilizado el constructor por
* defecto (que no reserva memoria) o si se ha tenido cuidado de liberar
* primero las memorias a las que apuntan los punteros
*/
void
buffer2D::Resize (INT Nelem, INT Nfft)
{

	if (m_buffer)
	{
		for (INT i = 0; i < m_Nelem; ++i)
			xfree(m_buffer[i]);
		xfree(m_buffer);
	}
	m_buffer = (DOUBLE**)xmalloc (Nelem*sizeof(DOUBLE*));

	for (INT i = 0; i < Nelem; ++i)
		m_buffer[i] = (DOUBLE*)xmalloc (Nfft*sizeof(DOUBLE));
	
	m_head = 0;
	m_tail = -1;
	m_Nelem  = Nelem;
	m_Ndim = Nfft;
}







	
/*!
* Una llamada a esta función destruye todos los datos del buffer,
* por lo que hay que llamarla con cuidado.
*/
void
buffer1D::Resize (INT Nelem)
{
	if (m_buffer)
		xfree(m_buffer);
	
	m_buffer = (DOUBLE*)xmalloc (Nelem*sizeof(DOUBLE));
	
	m_head = 0;
	m_tail = -1;
	m_Nelem  = Nelem;
}





