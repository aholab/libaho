/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LMEM.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Memoria segmentada 16:16
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
3.0.0    22/02/94  Borja     unificacion 64kb y 128kb ---> {kbpage}, etc
2.1.0    15/03/94  Borja     permite enviar NULL en {lmem} y {usebytes}
2.0.0    20/02/94  Borja     funciones fixlen???()
1.0.1    20/02/94  Borja     incorporacion de {align} y {granularity}
1.0.0    06/08/93  Borja     Codificacion inicial.

======================== Contenido ========================
Funciones fix??() para la adecuacion de bloques de memoria en
operaciones de transferencia DMA. Utilizan los tipos y macros
definidos en LMEM.H.
Mirar LMEM.H para informacion sobre las macros definidas.
....................
The fix??() functions are used to manage memory blocks
adecuate for DMA transfer. They use types and macros defined
in file LMEM.H.
Refer to LMEM.H for more information about the defined macros.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "lmem.h"

/**********************************************************/
/* pasa kb de kbytes a bytes */

#define KB2B(kb)  (((UINT32)(kb)) << 10)

/**********************************************************/
/* Recibe en {ptr} un puntero a un bloque de {nbytes} bytes de
longitud. La funcion busca dentro de este bloque, el subbloque
mas grande que no cruce una pagina de {kbpage} kilobytes (usar
64kb para buffer DMA-8bits, o 128kb para DMA-16 bits), y {devuelve}
un puntero a ese subbloque, y tambien devuelve este mismo puntero
pero en memoria lineal (direccionamiento plano) en {lmem}. En {usebytes}
se devuelve el numero de bytes que tiene el subbloque.
Ademas, el buffer devuelto cumple las condiciones de estar alineado
en un multiplo de {align} bytes, y de tener una longitud multiplo
de {granularity}.
Para asegurarse de que el buffer DMA devuelto
tenga un numero N de elementos, utilizar un vector {ptr} con el doble
de elementos (N*2 en {nbytes}). No tiene sentido enviar {nbytes}
mayor que 2*{kbpage}kb, pues logicamente, {usebytes} sera
{kbpage}kb como maximo.
En {lmem} y {usebytes} se puede enviar NULL si no interesa alguno
de estos valores.
....................
This function receives in {ptr} a pointer to a memory block of length
{nbytes}. The function will search in this memory block the largest
sub-block valid for DMA transfers. The DMA transfer condition will be
the following: the buffer must not cross a {kbpage} kilobytes memory
page (use {kbpage}=64 for DMA8 and {kbpage}=128 for DMA16).
The function {returns} a pointer to this usable buffer. It will also
return this same pointer in {lmem} but using 32bit lineal memory format.
In {usebytes} the function returns the block length in bytes of the
usable buffer.
Moreover, the returned buffer will be memory-aligned to a multiple of
{align}  (you can send 1 for DMA8, and 2 for DMA16), and it's length
will be a multiple of {granularity}.

If you want to assure that the usable DMA buffer will be N bytes
length, you can allocate ({ptr}) a memory block of N*2 bytes ({nbytes}).
It's a nonsense to send a buffer with  {nbytes} > 2*{kbpage}*1024
as the maximum value for {usebytes} will be {kbpage}*1024.
You can send NULL in {lmem} and {usebytes} if you are not interested
in any of this values. */

pfVOID fixmem( pfVOID ptr, UINT32 nbytes, UINT16 kbpage, UINT32 align,
        UINT32 granularity, UINT32 * lmem, UINT32 * usebytes )
{
    UINT32 lm, lm2, bpage, tmp, lmem2, page;

	page = KB2B(kbpage);
	if (nbytes > (page<<1))  /* limita nbytes a 2*page */
		nbytes = (page << 1);
	if (!align)  /* el alineamiento minimo es 1 */
		align = 1;
	if (!granularity)  /* la granularidad minima es 1 */
		granularity = 1;

	lm = PTR2LMEM(ptr);  /* convierte a puntero lineal */
	tmp = (align - (lm % align)) % align;  /* correccion de alineamiento */
	lm += tmp;
	nbytes -= tmp;

	/* busca siguiente cambio de {page} bytes */
	bpage = ((lm + page) / page) * page;
	lm2 = bpage + (align - (bpage % align)) % align; /* alineamiento */
	tmp = bpage - lm;  /* longitud primer bloque */

	if (tmp >= (nbytes >> 1)) {  /*primera parte mayor que la mitad*/
		lmem2 = lm;   /* usa la primera parte */
		if (nbytes > tmp)  /* fija longitud a usar */
			nbytes = tmp;
	}
	else {
		lmem2 = lm2;   /* usa la segunda parte */
		nbytes -= (lm2 - lm);  /* y olvida la primera */
	}
	if (nbytes > page)
		nbytes = page;

	/* vector a usar */
	if (lmem)
		*lmem = lmem2;
	/* longitud a usar, teniendo en cuenta la granularidad */
	if (usebytes)
		*usebytes = nbytes - (nbytes % granularity);
	return LMEM2PTR(lmem2);  /* devuelve puntero al bloque utilizable */
}

/**********************************************************/
/* similar a fixmem() solo que {devuelve} directamente la longitud
utilizable comenzando en el propio {ptr}, si el buffer tiene
{nbytes} bytes, no debe cruzar una pagina de {kbpage} kbytes y debe
cumplir estar alineado a {align} y con granularidad {granularity}
....................
this function is similar to fixmem() but it will directly {return}
the usable length from the first byte pointed by {ptr}. The
whole buffer is {nbytes} long. The usable buffer will not cross
a {kbpage} kilobytes memory page (send 64 for DMA8 or 128 for DMA16).
The buffer should be aligned to {align} bytes, and the granularity
will be {granularity} */

UINT32 fixlen( pfVOID ptr, UINT32 nbytes, UINT16 kbpage, UINT32 align,
        UINT32 granularity )
{
    UINT32 lm, bpage, tmp, page;

	page = KB2B(kbpage);
	if (!align)  /* el alineamiento minimo es 1 */
		align = 1;
	if (!granularity)  /* la granularidad minima es 1 */
		granularity = 1;

	lm = PTR2LMEM(ptr);  /* convierte a puntero lineal */
	tmp = lm % align;  /* alineamiento */
	if (!tmp) {   /* si esta alineado */
		bpage = ((lm + page) / page) * page;  /* siguientes 64k */
		tmp = bpage - lm;  /* longitud primera parte */
		if (tmp > nbytes)  /* fija longitud a usar */
			tmp = nbytes;
	}

	/* {devuelve} longitud a usar, teniendo en cuenta la granularidad */
	return (tmp - (tmp % granularity));
}

/**********************************************************/

