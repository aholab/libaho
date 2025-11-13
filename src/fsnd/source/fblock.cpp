/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ -
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    09/04/99  Borja     uso tipos UINT
0.0.1    08/04/97  Borja     optimizar y corregir algunos bugs
0.0.0    15/07/96  Borja     codificacion inicial.

======================== Contenido ========================
$$$ Todo: optimizar las de bloque
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fblock.hpp"
#include "xalloc.h"
#include "uti.h"

/**********************************************************/

FBlock::FBlock( VOID )
{
	f = NULL;
	bTotSize = bPos = 0;
	borderpolicy = 0;
  stdinout=FALSE;
}

/**********************************************************/

VOID FBlock::setPos( LONG pos )
{
	if (bPos==pos) return;
	bPos=pos;
	if (bPos<0) xfseek(f,bPos0,SEEK_SET);
	else if ((bPos>=bTotSize)&&(!stdinout)) xfseek(f,bPos0+bTotSize,SEEK_SET);
	else xfseek(f,bPos+bPos0,SEEK_SET);
}

/**********************************************************/
/* N=-1 para auto len, N=-2 para no-len (stdin/stdout) */

VOID FBlock::attach( FILE* fBin, LONG N, UINT bp )
{
	LONG l;

	f = fBin;
	bTotSize = bPos = 0;
	stdinout=FALSE;

	if (!f) return;

	borderpolicy=bp;
	bPos0 = xftell(f);  /* bytes delante */
	if (N!=-2) { // N==-2 caso especial para stdin/stdout
		xfseek(f,0,SEEK_END); /* calculo de bytes detras... */
		l = xftell(f)-bPos0;
	}
	else l=0;

	if (N==-1) {    /* calculo automatico? */
		bTotSize = l;   /* todo cuenta */
		bTail = 0;      /* no hay nada detras */
	} else if (N==-2) {
		bTotSize=0;
		bTail=0;
		stdinout=TRUE;
	} else {
		bTotSize = N;    /* N indicado de bytes */
		bTail = l - bTotSize;  /* el resto no cuenta */

		/* que no nos mientan con el numero N */
		cdie_beep(bTail<0,"FBlock: invalid file size");
	}

	if (!stdinout) xfseek(f,bPos0,SEEK_SET); // volvemos al primer byte
	bPos = 0;
	writeMode = FALSE;
}

/**********************************************************/

VOID FBlock::setRW( BOOL write )
{
	if (writeMode!=write) {
		writeMode = write;
		if (!stdinout) xfseek(f,0,SEEK_CUR);
	}
}

/**********************************************************/

UINT FBlock::get( BYTE& byte )
{
	if ((bPos>=0) && ((bPos<bTotSize)||stdinout)) {   /* estamos en rango valido? */
		setRW(FALSE);
		xfread(&byte, sizeof(byte), 1, f);
		bPos++;
		return 1;
	}
	else {
		byte = 0;
		bPos++;
		return 0;
	}
}

/**********************************************************/
#ifdef ORI
UINT FBlock::get( BYTE* block, UINT len )
{
	UINT n;

	if ((bPos>=0) && (bPos<bTotSize)) {
		n = len;
		if (bPos+n>bTotSize)
			n = (UINT)(bTotSize-bPos);
		if (block) {
			setRW(FALSE);
			xfread(block, sizeof(BYTE), n, f);
			if (n<len)
				memset(block+n,0,len-n);
		}
		else
			xfseek(f,n,SEEK_CUR);
		bPos+=len;
		return n;
	}

	if ((bPos<0)&&(-bPos<len)) {
		n = (UINT)(len+bPos);
		if (block) {
			setRW(FALSE);
			memset(block,0,len-n);
			xfread(block+len-n, sizeof(BYTE), n, f);
		}
		else
			xfseek(f,n,SEEK_CUR);
		bPos+=len;
		return n;
	}

	bPos+=len;
	if (block)
		memset(block,0,len);
	return 0;
}
#endif
/**********************************************************/

UINT FBlock::get( BYTE* block, UINT len )
{
	if ( ((bPos<0)&&(-bPos>=(LONG)len)) || ((bPos>=bTotSize)&&(!stdinout)) ) {
		bPos+=len;
		if (block) memset(block,0,len);
		return 0;
	}

	if (bPos<0) {
		if (block) { memset(block,0,(UINT)(-bPos)); block+=(UINT)(-bPos); }
		len -= (UINT)(-bPos);
		bPos = 0;
	}

	UINT n = len;
	if (!stdinout && (bPos+(LONG)n>bTotSize))
		n = (UINT)(bTotSize-bPos);
	if (block) {
		setRW(FALSE);
		UINT nn=fread(block, sizeof(BYTE), n, f);
		if ((!stdinout)&&(nn<n)) xfile_error("fread");
		if (nn<len) memset(block+nn,0,len-nn);
		if (nn<n) { len-=(n-nn); n=nn; };
	}
	else
		xfseek(f,n,SEEK_CUR);
	bPos+=len;
	if ((stdinout)&&(bPos>bTotSize)) bTotSize=bPos;
	return n;
}

/**********************************************************/

UINT FBlock::set( const BYTE* block, UINT len )
{
	UINT n=0;

	if (bPos<0) {
		if (-bPos>=(LONG)len) { bPos += (LONG)len; return 0; }
		len -= (UINT)(-bPos);
		if (block) block += (UINT)(-bPos);
		bPos = 0;
	}

	if (bPos<bTotSize) {
		n=len;
		if (bPos+(LONG)n>bTotSize) n = (UINT)(bTotSize-bPos);
		setRW(TRUE);
		if (block)
			xfwrite(block,sizeof(BYTE),n,f);
		else {
			UINT i;
			BYTE bb=0;
			for (i=0; i<n; i++) xfwrite(&bb,sizeof(BYTE),1,f);
		}
		bPos+=n;
		len-=n;
		block+=n;
	}
	if (!len) return n;

	if ((bPos>bTotSize)&&(!(borderpolicy&FBLOCK_BP_AFTER))&&(!stdinout)) {
		bPos+=len;
		return n;
	}

	BYTE bb=0;
	LONG i;
	if (bTail>0) {
		xfinsn(f,bPos0+bTotSize,(bPos-bTotSize+len));
		xfseek(f,bPos0+bTotSize,SEEK_SET);
		writeMode = TRUE;
	}
	else
		setRW(TRUE);

	for (i=bTotSize; i<bPos; i++) xfwrite(&bb,sizeof(bb),1,f);

	if (block) xfwrite(block,sizeof(BYTE),len,f);
	else for (UINT j=0; j<len; j++) xfwrite(&bb,sizeof(BYTE),1,f);

	bPos+=len;
	bTotSize = bPos;

	return len+n;
}

/**********************************************************/

UINT FBlock::ins( const BYTE* block, UINT len )
{
	BYTE bb=0;
	UINT i, p;

	if (stdinout) {
		if (bPos>=bTotSize) return set(block,len);
		else return 0;
	}

	if (bPos<0) {
		if (borderpolicy&FBLOCK_BP_BEFORE) {
			xfinsn(f,bPos0,len);
			xfseek(f,bPos0,SEEK_SET);
			if (bPos+len>0) {
				p = (UINT)(len+bPos);
				if (block) xfwrite(block+(UINT)(-bPos),sizeof(BYTE),p,f);
				else for ( i=0; i<p; i++) xfwrite(&bb,sizeof(bb),1,f);
			}
			else p=0;
			for ( i=p; i<len; i++) xfwrite(&bb,sizeof(bb),1,f);
			xfseek(f,bPos0+p,SEEK_SET);
			writeMode = TRUE;
			bTotSize+=len;
			bPos+=len;
			return len;
		}
		else {
			if (bPos+len>0) {
				p = (UINT)(len+bPos);
				xfinsn(f,bPos0,p);
				xfseek(f,bPos0,SEEK_SET);
				if (block) xfwrite(block+(UINT)(-bPos),sizeof(BYTE),p,f);
				else for ( i=0; i<p; i++) xfwrite(&bb,sizeof(bb),1,f);
				bTotSize+=p;
				bPos+=len;
				return p;
			}
			bPos+=len;
			return 0;
		}
	}

	if (bPos<=bTotSize) {
		if (((bPos<bTotSize)||(bTail>0))) {
			xfinsn(f,bPos0+bPos,len);
			xfseek(f,bPos0+bPos,SEEK_SET);
			writeMode = TRUE;
		}
		else
			setRW(TRUE);
		if (block) xfwrite(block,sizeof(BYTE),len,f);
		else for (i=0; i<len; i++) xfwrite(&bb,sizeof(bb),1,f);
		bTotSize+=len;
		bPos+=len;
		return len;
	}

	if (borderpolicy&FBLOCK_BP_AFTER) {
		LONG j;
		if (bTail>0) xfinsn(f,bPos0+bTotSize,(bPos-bTotSize+len));
		xfseek(f,bPos0+bTotSize,SEEK_SET);
		for (j=bTotSize; j<bPos; j++) xfwrite(&bb,sizeof(bb),1,f);
		writeMode = TRUE;
		if (block) xfwrite(block,sizeof(BYTE),len,f);
		else for (i=0; i<len; i++) xfwrite(&bb,sizeof(bb),1,f);
		bPos+=len;
		bTotSize=bPos;
		return len;
	}
	bPos+=len;
	return 0;
}

/**********************************************************/

UINT FBlock::del( UINT len )
{
	if (bPos>=bTotSize) return 0;
	if (stdinout) return 0;
	if ((bPos<0)&&(!(borderpolicy&FBLOCK_BP_BEFORE))) return 0;

	LONG pos=bPos;
	if (pos<0) pos=0;

	if (pos+(LONG)len>bTotSize) len = (UINT)(bTotSize-pos);

	xfdeln(f,bPos0+pos,len);
	xfseek(f,bPos0+pos,SEEK_SET);
	bTotSize-=len;
	return len;
}

/**********************************************************/
// $$$ deberia cambiar la variable??

VOID FBlock::toggleRW( VOID )
{
	xfseek(f,0,SEEK_CUR);
}

/**********************************************************/

BOOL FBlock::OK( VOID ) const
{
	long pos;
	long len;

	if (!f) return TRUE;
	if (stdinout) return TRUE;

	pos = xftell(f);
	xfseek(f,0,SEEK_END);
	len = xftell(f);
	xfseek(f,pos,SEEK_SET);

	if (bPos+bPos0!=pos) return FALSE;
	if (bPos0+bTotSize+bTail!=len) return FALSE;

	return TRUE;
}

/**********************************************************/
