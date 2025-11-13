#ifndef __XALLOC_H
#define __XALLOC_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XALLOC.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... si (mirar abajo)

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
2.0.2    22/05/97  Borja     uso XALLOC_HARD
2.0.1    22/07/96  Borja     uso de ARCH.H
2.0.0    09/07/94  Borja     Demasiados cambios para ponerlos aqui.
1.0.0    11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Fichero de cabecera de XALLOC.C.

Funciones xmalloc() y xfree() similares a malloc() y free()
pero que chequean la integridad de los bloques, detectando
si se ha escrito por delante o por detras del array. Se
implementan tambien otras funciones utiles relacionadas.

Mas informacion en xalloc.c
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "arch.h"

/**********************************************************/
/* Si no hay un nivel de debugeo preseleccionado, seleccionamos
uno automaticamente en funcion de NDEBUG */

#ifndef XALLOC_LVL
#ifdef NDEBUG
#define XALLOC_LVL 1
#else
#define XALLOC_LVL 2
#endif
#endif

/**********************************************************/
/* En funcion del nivel, activar/desactivar definiciones internas */

#if XALLOC_LVL==2
#define __XALLOC_XFUNCS
#define __XALLOC_SRCNAME

#elif XALLOC_LVL==1
#define __XALLOC_XFUNCS

#elif XALLOC_LVL==-2
#define __XALLOC_XFUNCS
#define __XALLOC_SRCNAME
#define __XALLOC_STDOVL

#elif XALLOC_LVL==-1
#define __XALLOC_XFUNCS
#define __XALLOC_STDOVL

#endif

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/*<DOC>*/
/* codigos de retorno de testblk() y testheap() */

#define XALLOC_BLK_NOT_CORRUPTED 0
#define XALLOC_BLK_CORRUPTED_START 1
#define XALLOC_BLK_CORRUPTED_END 2
#define XALLOC_BLK_CORRUPTED 3
#define XALLOC_BLK_NULLPTR 4

#define XALLOC_HEAP_NOT_CORRUPTED 0
#define XALLOC_HEAP_CORRUPTED 5
#define XALLOC_HEAP_WRITE_IN_FREE_MEM 6
#define XALLOC_HEAP_VERY_CORRUPTED 7
/*</DOC>*/
/**********************************************************/
/* uso interno */

#ifdef __XALLOC_SRCNAME
#define IFLARGS  char *srcname, int line
#define IFLPARAM  __FILE__,__LINE__
#define FLARGS  ,char *srcname, int line
#define FLPARAM  ,__FILE__,__LINE__
#else
#define IFLARGS
#define IFLPARAM
#define FLARGS
#define FLPARAM
#endif

#ifdef __XALLOC_XFUNCS
/* nombres diferentes si reciben mas parametros */
#ifdef __XALLOC_SRCNAME
void * _xmalloc_src( unsigned long size   FLARGS );
void * _xcalloc_src( unsigned long nitems, unsigned long size   FLARGS );
void _xfree_src( void * block   FLARGS );
char * _xstrdup_src( const char * str   FLARGS );
unsigned long _xblksize_src( void * block	  FLARGS );
void _xblk2disk_src( char * fname, void * block	  FLARGS );
void * _xdisk2blk_src( char * fname   FLARGS );
void _assertblk_src( void * block   FLARGS );
void _assertheap_src( IFLARGS );
#else
void * _xmalloc( unsigned long size );
void * _xcalloc( unsigned long nitems, unsigned long size );
void _xfree( void * block );
char * _xstrdup( const char * str );
unsigned long _xblksize( void * block );
void _xblk2disk( char * fname, void * block );
void * _xdisk2blk( char * fname );
void _assertblk( void * block );
void _assertheap( void );
#endif
#endif

/**********************************************************/

#ifdef __XALLOC_XFUNCS

#ifdef __XALLOC_SRCNAME
#define xmalloc(unsigned_long_size) _xmalloc_src(unsigned_long_size  FLPARAM)
#define xcalloc(unsigned_long_nitems,unsigned_long_size) _xcalloc_src(unsigned_long_nitems,unsigned_long_size  FLPARAM)
#define xfree(void_ptr_block) _xfree_src(void_ptr_block	 FLPARAM)
#define xstrdup(const_char_ptr_str) _xstrdup_src(const_char_ptr_str	 FLPARAM)
#define xblksize(void_ptr_block) _xblksize_src(void_ptr_block	 FLPARAM)
unsigned long xusedmem( void );
int testblk( void * block );
#define assertblk(void_ptr_block) _assertblk_src(void_ptr_block	 FLPARAM)
void fillheap( void );
unsigned long xusedheap( void );
int testheap( void );
#define assertheap() _assertheap_src(FLPARAM)
#define heapcmd(cmd)  {  assertheap();  cmd;  fillheap(); }
#define xblk2disk(char_ptr_fname,void_ptr_block) \
		_xblk2disk_src(char_ptr_fname,void_ptr_block	 FLPARAM)
#define xdisk2blk(char_ptr_fname) \
		_xdisk2blk_src(char_ptr_fname   FLPARAM)
#else   /* __XALLOC_SRCNAME */
#define xmalloc(unsigned_long_size) _xmalloc(unsigned_long_size )
#define xcalloc(unsigned_long_nitems,unsigned_long_size) \
		_xcalloc(unsigned_long_nitems,unsigned_long_size)
#define xfree(void_ptr_block) _xfree(void_ptr_block)
#define xstrdup(const_char_ptr_str) _xstrdup(const_char_ptr_str)
#define xblksize(void_ptr_block) _xblksize(void_ptr_block)
unsigned long xusedmem( void );
int testblk( void * block );
#define assertblk(void_ptr_block) _assertblk(void_ptr_block)
void fillheap( void );
unsigned long xusedheap( void );
int testheap( void );
#define assertheap() _assertheap()
#define heapcmd(cmd)  {  assertheap();  cmd;  fillheap(); }
#define xblk2disk(char_ptr_fname,void_ptr_block) \
		_xblk2disk(char_ptr_fname,void_ptr_block)
#define xdisk2blk(char_ptr_fname) \
		_xdisk2blk(char_ptr_fname)
#endif  /* __XALLOC_SRCNAME */

void xalloc_logf(int on);
void xalloc_heapcheck(int on);
void xalloc_showmem(int on);
void xalloc_showheap(int on);
void xalloc_showfree(int on);

#else  /* __XALLOC_XFUNCS */

#include <stdlib.h>
#include <string.h>

#define xmalloc(unsigned_long_size) malloc(unsigned_long_size)
#define xcalloc(unsigned_long_nitems,unsigned_long_size) \
		calloc(unsigned_long_nitems,unsigned_long_size)
#define xfree(void_ptr_block) free(void_ptr_block)
#define xstrdup(const_char_ptr_str) strdup(const_char_ptr_str)
#define xblksize(void_ptr_block) 0
#define xusedmem() 0
#define testblk(block) XALLOC_BLK_NOT_CORRUPTED
#define assertblk(void_ptr_block) {}
#define fillheap() {}
#define xusedheap() 0
#define testheap() XALLOC_HEAP_NOT_CORRUPTED
#define assertheap() {}
#define heapcmd(cmd)  { cmd; }
#define xblk2disk(char_ptr_fname,void_ptr_block) {}
#define xdisk2blk(char_ptr_fname) NULL

#define xalloc_logf(b) {}
#define xalloc_heapcheck(b) {}
#define xalloc_showmem(b) {}
#define xalloc_showheap(b) {}
#define xalloc_showfree(b) {}

#endif /* __XALLOC_XFUNCS */

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/
/* si queremos matxacar las funciones de la libreria estandar */

#ifdef __XALLOC_STDOVL

#define malloc(unsigned_long_size)  xmalloc(unsigned_long_size)
#define calloc(unsigned_long_nitems,unsigned_long_size) \
		xcalloc(unsigned_long_nitems,unsigned_long_size)
#define free(void_ptr_block)  xfree(void_ptr_block)
#define strdup(const_char_ptr_str)  xstrdup(const_char_ptr_str)

#ifdef __CC_BORLANDC__
#define farmalloc(unsigned_long_size)  xmalloc(unsigned_long_size)
#define farcalloc(unsigned_long_nitems,unsigned_long_size) \
		xcalloc(unsigned_long_nitems,unsigned_long_size)
#define farfree(void_ptr_block)  xfree(void_ptr_block)
#define _fstrdup(const_char_ptr_str)  xstrdup(const_char_ptr_str)
#endif   /* BORLANDC */

#endif

/**********************************************************/

#endif
