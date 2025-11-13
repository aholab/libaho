/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ DMABUFF.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... DMA
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.2    07/09/95  Borja     valor de STEP y 'while' en vez de 'for'
1.0.1    06/04/95  Borja     diferente {extra} y STEP
1.0.0    20/02/94  Borja     Codificacion inicial

======================== Contenido ========================
Reserva de buffer DMA sin derrochar mucho espacio.
....................
Intelligent DMA buffer memory allocation to save memory.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>

#include "xalloc.h"
#include "lmem.h"
#include "dmabuff.h"

/**********************************************************/
/* Reserva un buffer con xmalloc() (usar xfree() para liberarlo) y
{devuelve} un puntero a el, o NULL si no se puede reservar correctamente.
En buffer cumplira que por lo menos hay {nbytes} utilizables en el para
hacer operaciones DMA; el buffer no cruzara una pagina de {kbpage}kb
(generalmente 64 o 128 kb), estara alineado a {align} bytes, y sera de
multiplicidad {granularity}. Usar fixmem() definida en LMEM.C para
obtener el buffer a utilizar dentro de este buffer que se reserva.
En {allocbytes} se devuelve el numero de bytes reservados (se puede
enviar NULL si no interesa. El valor devuelto en {allocbytes} sera
en general mayor que {nbytes}.
....................
This function allocates a memory buffer using xmalloc() so you must
use xfree() to free it. The function {returns} a pointer to the
allocated buffer, or NULL if it can not be allocated correctly.
The buffer will have {nbytes} or more bytes usable for DMA operations,
i.e, it will not cross a memory page of {kbpage} kbytes (send 64 for
DMA8 and 128 for DMA16), it will be memory-aligned to {align} bytes,
and it will have {granularity} multiplicity (i.e, the number of usable
bytes will at least be a multipe of {granularity}).

You should use fixmem()  (defined in LMEM.C) to get the actual dma
buffer from the memory block returned by this function.
The function returns the actual allocated memory in {allocbytes}
and {allocbytes} will be greater or equal than {nbytes}. If you are
not interested in this value, you can send NULL in {allocbytes} */

pfVOID dmabuff_malloc( UINT32 nbytes, UINT16 kbpage, UINT32 align,
	UINT32 granularity, UINT32 * allocbytes )
{
#define STEP 16
	pfVOID bptr, tptr;
	UINT32 tbytes, ubytes;
	UINT16 i;
	UINT32 extra;

	tptr = NULL;
	extra = 0;
	if (align > extra)
		extra = align;
	if (granularity > extra)
		extra = granularity;

	tbytes = nbytes + extra;
	bptr = xmalloc(tbytes);

	if (allocbytes)
		*allocbytes = tbytes;

	i = 0;
	do {
		if (bptr) {
			fixmem(bptr, nbytes, kbpage, align, granularity, NULL, &ubytes);
			if (ubytes >= nbytes)
				break;
		}
		else
			break;
		xfree(bptr);
		bptr = NULL;
		if (tptr)
			xfree(tptr);
		i++;
		tptr = xmalloc((UINT32)i * STEP);
		bptr = xmalloc(tbytes);
	} while (tptr);

	if (tptr)
		xfree(tptr);
	if (bptr) {
		fixmem(bptr, nbytes, kbpage, align, granularity, NULL, &ubytes);
		if (ubytes >= nbytes)
			return bptr;
		xfree(bptr);
	}

	return NULL;
#undef STEP
}

/**********************************************************/
