/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XALLOC.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... SI (ver un monton de defines mas abajo)

Codificacion................. Borja Etxebarria

Vers.   DD/MM/AA   Autor     Proposito de la edicion
-----   --------   -------   -----------------------
2.0.6   11/05/11   Inaki     actualizacion gcc-4.5
2.0.5   22/05/97   Borja     uso XALLOC_HARD
2.0.4   30/04/97   Borja     testblk(), testheap() no estaban definidas
2.0.3   30/07/96   Borja     usar xdie() para errores
02.02   15/03/95   Borja     reconocim. S.O y compilador (__CC_BORLANDC__, etc)
02.01   21/07/94   Borja     Gestion de todos los casos NULL por separado.
02.00   09/07/94   Borja     Demasiados cambios para ponerlos aqui.
01.00   11/01/93   Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Funciones xmalloc() y xfree() similares a malloc() y free()
pero que chequean la integridad de los bloques, detectando
si se ha escrito por delante o por detras del array. Se
implementan tambien otras funciones utiles relacionadas.

Reserva memoria:
-  xmalloc(...)  como malloc(...)
-  xcalloc(...)  como calloc(...)
-  xstrdup(...)  como strdup(...)
Libera memoria:
-  xfree()  como free(...)
Chequeo:
-  assertblk(ptr)  comprueba que el bloque {ptr} no este corrupto, y
	tambien comprueba la heap libre.
-  assertheap()  comprueba que la heap libre no este corrupta.
-  fillheap()  fuerza el resincronizado de la heap libre.
-  heapcmd(comando)  resincroniza la heap para aquellas funciones que
	liberen memoria a su aire (por ejemplo fclose(f)) libera buffers,
	por lo que haremos assertheap(fclose(f)).
Las funciones assert??? hacen que el programa termine si se detecta
alguna inconsistencia en memoria. Las funciones equivalente testheap()
y testblk() hacen lo mismo, pero el programa no termina, sino que
se {devuelve} un codigo de error (ver doc. en xalloc.h).

xblk2disk() y xdisk2blk() permiten guardar un bloque de xmemoria
a disco, y cargarlo de disco a memoria. Al cargar se hace xmalloc(),
el usuario debera llamar a xfree() cuando termine con el bloque.

Hay debuggers fuertes que se pueden activar/desactivar (por defecto
desactivados) con estas funciones:
xalloc_logf(): fichero de registro de mallocs y frees (xalloc.log)
xalloc_heapcheck(): chequeo de heap (lentooo)
xalloc_showmem(): imprime memoria usada en pantalla
xalloc_showheap(): imprime heap usada en pantalla  (MSDOS)
xalloc_showfree(): imprime memoria libre en pantalla (MSDOS).

Prototipos:
void * xmalloc(unsigned long size)
void * xcalloc(unsigned long nitems, unsigned long size)
void xfree(void * block)
char * xstrdup(char * str)
unsigned long xblksize(void * block)
unsigned long xusedmem( void )
int testblk( void * block )
void assertblk(void * block)
void fillheap( void )
unsigned long xusedheap( void )
int testheap( void )
void assertheap( void )
heapcmd( {comando; comando;...})
void xblk2disk(char * fname,void * block)
void * xdisk2blk(char * fname) \
void xalloc_logf(int on)
void xalloc_heapcheck(int on)
void xalloc_showmem(int on)
void xalloc_showheap(int on)
void xalloc_showfree(int on)

</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "tdef.h"
#include "uti.h"
#include "xalloc.h"

#ifdef __XALLOC_XFUNCS

/**********************************************************/

#if XALLOC_LVL==-2
#undef XALLOC_LVL
#define XALLOC_LVL 2
#elif XALLOC_LVL==-1
#undef XALLOC_LVL
#define XALLOC_LVL 1
#endif

/**********************************************************/
/* Definir MALLOC_ZERO para detectar xmalloc(0)   (un malloc en el que
se van a reserver 0 bytes). Si no se define, xmalloc(0) simplemente
devuelve NULL. Normalmente desactivado.

Definir MALLOC_NOMEM para que un malloc fallido (devuelve NULL,
porque no hay memoria) produzca mensaje de error. Aun asi, puede
suceder que xmalloc() devuelva NULL si se hace xmalloc(0) (a no ser
que tambien se detecte esta situacion, definiendo MALLOC_ZERO.
Normalmente activado.

Definir FREE_NULL para detectar si se hace un free(NULL), que
puede ser fatal!!. Normalmente activado.

NULL_TRAP detecta assertblk(NULL) como error. Tambien detecta
xfree(NULL) aunque FREE_NULL no este definido. Normalmente
desactivado.

Definir BLK_CHECK para activar el chequeo de integridad en todos los
malloc/free, utilizando zonas de proteccion (guardas) por delante
y detras de cada malloc. Activado en el debugger pesado.

Definir LOGF_SAVE para poder activar el salvar a fichero log. de
los malloc/free que se hacen. Activado en el debugger pesado,
pero en estado OFF.

Definir HEAP_CHECK para chequeo de heap (Borland). LENTOOOOO!!!.
Activado en el debugger pesado, pero en estado OFF.

Definir MEM_USED para activar el imprimir bytes de memoria utilizada
por el usuario. Activado en el debugger pesado, pero en estado OFF.

Definir HEAP_USED para activar el imprimir bytes de memoria utilizada
en total (incluyendo guardas XALLOC). Activado en el debugger
pesado, pero en estado OFF.

Definir HEAP_FREE para poder imprimir bytes de memoria libre.
Activado en el debugger pesado, pero en estado OFF.
*/


#if XALLOC_LVL==1   /* debugger suave */

#define MALLOC_ZEROxx
#define MALLOC_NOMEM
#define FREE_NULL
#define NULL_TRAPxx
#define BLK_CHECKxx
#define LOGF_SAVExx
#define HEAP_CHECKxx
#define MEM_USEDxx
#define HEAP_USEDxx
#define HEAP_FREExx

#define LOGF_SAVE_DEF_STATE FALSE
#define HEAP_CHECK_DEF_STATE FALSE
#define MEM_USED_DEF_STATE FALSE
#define HEAP_USED_DEF_STATE FALSE
#define HEAP_FREE_DEF_STATE FALSE

#elif XALLOC_LVL==2  /* debugger pesado */
#define MALLOC_ZEROxx
#define MALLOC_NOMEM
#define FREE_NULL
#define NULL_TRAPxx
#define BLK_CHECK
#define LOGF_SAVE
#define HEAP_CHECK
#define MEM_USED
#define HEAP_USED
#define HEAP_FREE

#define LOGF_SAVE_DEF_STATE FALSE
#define HEAP_CHECK_DEF_STATE FALSE
#define MEM_USED_DEF_STATE FALSE
#define HEAP_USED_DEF_STATE FALSE
#define HEAP_FREE_DEF_STATE FALSE

#endif

/**********************************************************/

#ifdef __XALLOC_SRCNAME
#define IUFLARGS  srcname, line
#define UUFLARGS  ,srcname, line
#else
#define IUFLARGS
#define UUFLARGS
#endif

/**********************************************************/
/* eliminar los defines hechos en xalloc.h si hace falta */

#ifdef __XALLOC_STDOVL

#undef malloc
#undef calloc
#undef free
#undef strdup
#undef farmalloc
#undef farcalloc
#undef farfree
#undef _fstrdup

#endif

/**********************************************************/
/* Valores de configuracion para cada debugger anterior. Se utiliza
solo si el correspondiente debugger esta activado */

/* este define indica la longitud de cada una de las guardas que se
van a reserver al rededor de cada malloc. Se reservan 4 bloques de esta
longitud por delante, y otros 4 por detras.
Ademas se reservan otros 12 bytes mas para informacion auxiliar */
#define BLK_LIMITER_SIZE 4

/* valor de relleno de las guardas (incremento a partir de) */
#define BLK_FILLER 159

/* valor de relleno del heap libre */
#define HEAP_FILLER  123

/* nombre del fichero log de malloc/free */
#define LOGF_NAME "xalloc.log"

/* coordenadas X,Y en las que se imprime la memoria utilizada
por el usuario (1..)*/
#define MEM_USED_X 72
#define MEM_USED_Y 1

/* coordenadas X,Y en las que se imprime el heap total utilizadao (1..)*/
#define HEAP_USED_X 1
#define HEAP_USED_Y 25

/* coordenadas X,Y en las que se imprime el heap libre (1..)*/
#define HEAP_FREE_X 72
#define HEAP_FREE_Y 25

/**********************************************************/
/* codigo dependiente del sistema/compilador */

#if __CC_BORLANDC__

#include <alloc.h>
#include <string.h>  /* strcpy, _fstrcpy */
/* modelos de compilacion far */
#if (defined(__COMPACT__)||defined(__LARGE__)||defined(__HUGE__))
#define XPTR PTRH
#define MALLOC farmalloc
#define CALLOC farcalloc
#define FREE farfree
#define CORELEFT farcoreleft
#define STRCPY _fstrcpy
#define STRLEN _fstrlen
#else
/* modelos de compilacion near */
#define XPTR PTR
#define MALLOC malloc
#define CALLOC calloc
#define FREE free
#define CORELEFT coreleft
#define STRCPY strcpy
#define STRLEN strlen
#endif

#define HEAPCHECK()  heapcheck()
#define HEAPCHECKFREE(value)  heapcheckfree(value)
#define HEAPFILLFREE(value)  heapfillfree(value)
#define HEAP_OK _HEAPOK
#define HEAP_CORRUPT _HEAPCORRUPT
#define HEAP_FREEWRITE _BADVALUE

#else    /* resto de casos, asume compilador 32 bits */

#include <string.h>  /* strcpy, */
#define XPTR PTRH
#define MALLOC malloc
#define CALLOC calloc
#define FREE free
#define CORELEFT() 0
#define STRCPY strcpy
#define STRLEN strlen

/* estas son solo de borland, asi que no hacen nada. */
#define HEAPCHECK()  0
#define HEAPCHECKFREE(value)  0
#define HEAPFILLFREE(value)  {}
#define HEAP_OK 0
#define HEAP_CORRUPT 1
#define HEAP_FREEWRITE 2
#endif

#if __OS_BORLANDC__
#define PRINT(msg,arg1) printf(msg,arg1)
#else
#define PRINT(msg,arg1) fprintf(stderr,msg,arg1)
#endif

/**********************************************************/

#if (defined(MEM_USED) || defined(HEAP_USED) || defined(HEAP_FREE))

#if __CC_BORLANDC__

#include <conio.h>   /* para gotoxy(), wherex() y wherey() */
#define GOTOXY(x,y)  gotoxy(x,y)
#define WHEREX()  wherex()
#define WHEREY()  wherey()

#else    /* resto de casos, ignora posicionamiento de cursor */

#define GOTOXY(x,y)
#define WHEREX()  1
#define WHEREY()  1

#endif  /* dep. maquina/S.O */

#endif  /* ???_USED */

/**********************************************************/
/* Decidir si se deben utilizar bloques de control */

#if (defined(BLK_CHECK)||defined(LOGF_SAVE)||defined(MEM_USED)||defined(HEAP_USED))
#define MEM_LIM
#endif

/**********************************************************/

#ifdef MEM_LIM

/* Asegurar alineamiento minimo de estructuras */
#ifdef __CC_MSVC__
#pragma pack(push,1)
#elif __CC_BORLANDC__
typedef struct {CHAR8 x; INT2 y; } ____tmp_xalloc_align_test___;
#if sizeof(____tmp_xalloc_align_test___)>3
#error You must disable Word Alignment
#endif
#endif

/* estructura de control de bloque de memoria reservada */
typedef struct {
#ifdef BLK_CHECK
#if (BLK_LIMITER_SIZE>0)
	INT8 s0[BLK_LIMITER_SIZE] MINALIGN;
#endif
#endif
	unsigned long blklen  MINALIGN;
#ifdef BLK_CHECK
#if (BLK_LIMITER_SIZE>0)
	INT8 s1[BLK_LIMITER_SIZE]  MINALIGN;
#endif
#endif
#ifdef LOGF_SAVE
	long logf_count MINALIGN;
#endif
#ifdef BLK_CHECK
#if (BLK_LIMITER_SIZE>0)
	INT8 s2[BLK_LIMITER_SIZE] MINALIGN;
#endif
	unsigned long blklen1 MINALIGN;
#if (BLK_LIMITER_SIZE>0)
	INT8 s3[BLK_LIMITER_SIZE] MINALIGN;
#endif
#endif
} mem_limiter;

#define STARTER(blk)  \
	((mem_limiter XPTR)(((INT8 XPTR)(blk))-sizeof(mem_limiter)))

#define ENDER(blk,len) \
	((mem_limiter XPTR)(((INT8 XPTR)(blk))+(len)))

#define XBPTR(blk,n) (((INT8 XPTR)(blk))+(n))

#ifdef __CC_MSVC__
#pragma pack(pop)
#endif

#endif  /* MEM_LIM */

/**********************************************************/
/* Variables de activacion de los debuggers pesados */

#ifdef LOGF_SAVE
PRIVATE BOOL logf_save_on = LOGF_SAVE_DEF_STATE;
#endif
#ifdef HEAP_CHECK
PRIVATE BOOL heap_check_on = HEAP_CHECK_DEF_STATE;
#endif
#ifdef MEM_USED
PRIVATE BOOL mem_used_on = MEM_USED_DEF_STATE;
#endif
#ifdef HEAP_USED
PRIVATE BOOL heap_used_on = HEAP_USED_DEF_STATE;
#endif
#ifdef HEAP_FREE
PRIVATE BOOL heap_free_on = HEAP_FREE_DEF_STATE;
#endif

/**********************************************************/
/* variables internas */

#ifdef LOGF_SAVE
PRIVATE long _logf_count=0;
PRIVATE int _logf_append=0;
#endif

#if (defined(MEM_USED) || defined(LOGF_SAVE))
PRIVATE unsigned long _mem_used=0;
#endif

#ifdef HEAP_USED
PRIVATE unsigned long _heap_used=0;
#endif

#ifdef HEAP_CHECK
PRIVATE int _heap_filled=0;
#endif

/**********************************************************/

PRIVATE void _xalloc_error_handler( char * msg   FLARGS )
{
#ifdef __XALLOC_SRCNAME
	die_beep("%s [%d]  %s" UUFLARGS,msg);
#else
	die_beep("%s",msg);
#endif
}

/**********************************************************/
#ifdef LOGF_SAVE

PRIVATE void _logf_log( long count, unsigned long size,
		int freeing   FLARGS )
{
	FILE * logfile;

	if (_logf_append)
		logfile = fopen(LOGF_NAME,"a");
	else {
		logfile = fopen(LOGF_NAME,"w");
		_logf_append = 1;
	}

	if (logfile) {
#ifdef __XALLOC_SRCNAME
		if (freeing)
			fprintf(logfile,"$F%06ld %12s (%5d) -%7lu (%7lu->%7lu)\n",
					count UUFLARGS, size, _mem_used+size,_mem_used);
		else
			fprintf(logfile,"$M%06ld %12s (%5d) +%7lu (%7lu->%7lu)\n",
					count UUFLARGS, size, _mem_used-size,_mem_used);
#else
		if (freeing)
			fprintf(logfile,"$F%06ld -%7lu (%7lu->%7lu)\n",
					count, size, _mem_used+size,_mem_used);
		else
			fprintf(logfile,"$M%06ld +%7lu (%7lu->%7lu)\n",
					count, size, _mem_used-size,_mem_used);
#endif
#ifdef __XALLOC_SRCNAME
	_assertheap_src(IUFLARGS);
#else
	_assertheap();
#endif
		fclose(logfile);
		fillheap();
	}
	else
		_xalloc_error_handler("error opening " LOGF_NAME  UUFLARGS);
}

#endif
/**********************************************************/
#if (defined(MEM_USED) || defined(HEAP_USED) || defined(HEAP_FREE))

PRIVATE void _show_used( void )
{
 	int x,y;

  BOOL show=FALSE;

#ifdef MEM_USED
	show = show || mem_used_on;
#endif
#ifdef HEAP_USED
	show = show || heap_used_on;
#endif
#ifdef HEAP_FREE
	show = show || heap_free_on;
#endif
	if (!show) return;

	x = WHEREX();
	y = WHEREY();

#ifdef MEM_USED
	GOTOXY(MEM_USED_X,MEM_USED_Y);
	PRINT("%7lu ",_mem_used);
#endif

#ifdef HEAP_USED
	GOTOXY(HEAP_USED_X,HEAP_USED_Y);
	PRINT("%7lu ",_heap_used);
#endif

#ifdef HEAP_FREE
	GOTOXY(HEAP_FREE_X,HEAP_FREE_Y);
	PRINT("%7lu ",(unsigned long)CORELEFT());
#endif

	GOTOXY(x,y);
}
#else

#define _show_used()

#endif
/**********************************************************/
/* dado un bloque {p} de {size} elementos utilizables + sitio para los
limitadores de comienzo, final, etc (si estan activados estos debuggers,
claro), lo prepara y {devuelve} un puntero al bloque utilizable por
el usuario. */

PRIVATE void XPTR _prepare_block( INT8 XPTR p, unsigned long size   FLARGS )
{
#ifdef MEM_LIM
	mem_limiter XPTR ps;
#endif
#ifdef BLK_CHECK
#if (BLK_LIMITER_SIZE>0)
	int i;
#endif
#endif

	if (p==NULL) {
#ifdef MALLOC_NOMEM
		PRIVATE char msg[] = "Out of memory (123456789012)";
		_show_used();
		sprintf(strchr(msg,'(')+1,"%lu)",size);
		_xalloc_error_handler(msg  UUFLARGS);
#else
		_show_used();
#endif
		return NULL;
	}

#if (defined(MEM_USED) || defined(LOGF_SAVE))
	_mem_used += size;
#endif
#ifdef HEAP_USED
	_heap_used += size;
#ifdef MEM_LIM
	_heap_used += sizeof(mem_limiter);
#endif
#ifdef BLK_CHECK
	_heap_used += sizeof(mem_limiter);
#endif
#endif /* HEAP_USED */

	_show_used();

#ifdef MEM_LIM
	ps = (mem_limiter XPTR)p;
	ps->blklen = size;
	p += sizeof(mem_limiter);
#endif

#ifdef LOGF_SAVE
  if (logf_save_on) {
  	ps->logf_count = _logf_count;
		_logf_log(_logf_count,size,0 UUFLARGS);
  }
  else
  	ps->logf_count = -_logf_count;
	_logf_count++;
#endif

#ifdef BLK_CHECK
	ps->blklen1 = ps->blklen;
#if (BLK_LIMITER_SIZE>0)
	for (i=0; i<BLK_LIMITER_SIZE; i++)
		ps->s3[i] = (ps->s2[i] = (ps->s1[i] = (ps->s0[i] = BLK_FILLER+i)
				+ BLK_LIMITER_SIZE) + BLK_LIMITER_SIZE) + BLK_LIMITER_SIZE;
#endif
	*(mem_limiter XPTR)(p + size) = (*ps);
#endif

#ifdef HEAP_CHECK
#ifdef __XALLOC_SRCNAME
	_assertheap_src(IUFLARGS);
#else
	_assertheap();
#endif
#endif

	return (void XPTR)p;
}

/**********************************************************/
/* reserva {size} bytes con todos los debuggers que esten activos */

#ifdef __XALLOC_SRCNAME
void * _xmalloc_src( unsigned long size   FLARGS )
#else
void * _xmalloc( unsigned long size )
#endif
{
	if (!size)
#ifdef MALLOC_ZERO
		_xalloc_error_handler("xmalloc(0)" UUFLARGS);
#else
		return NULL;
#endif

	return _prepare_block( (INT8 XPTR)MALLOC( size
#ifdef MEM_LIM
			+ sizeof(mem_limiter)
#endif
#ifdef BLK_CHECK
			+ sizeof(mem_limiter)
#endif
			), size  UUFLARGS);
}

/**********************************************************/
/* reserva {nitems} elementos de {size} bytes cada uno, protegidos
por todos los debuggers que esten activos */

#ifdef __XALLOC_SRCNAME
void * _xcalloc_src( unsigned long nitems, unsigned long size   FLARGS)
#else
void * _xcalloc( unsigned long nitems, unsigned long size )
#endif
{
	size*=nitems;

	if (!size)
#ifdef MALLOC_ZERO
		_xalloc_error_handler("xcalloc(0)" UUFLARGS);
#else
		return NULL;
#endif

	return _prepare_block( (INT8 XPTR)CALLOC( size
#ifdef MEM_LIM
			+ sizeof(mem_limiter)
#endif
#ifdef BLK_CHECK
			+ sizeof(mem_limiter)
#endif
			, 1), size  UUFLARGS);
}

/**********************************************************/

void fillheap( void )
{
#ifdef HEAP_CHECK
  if (heap_check_on) HEAPFILLFREE(HEAP_FILLER);
#endif
}

/**********************************************************/
/* libera memoria reservada con proteccion (xalloc...) */

#ifdef __XALLOC_SRCNAME
void _xfree_src( void * block   FLARGS )
#else
void _xfree( void * block )
#endif
{
#ifdef LOGF_SAVE
	long count;
#endif
#if (defined(MEM_USED) || defined(LOGF_SAVE) || defined(HEAP_USED))
	unsigned long size;
#endif

#ifdef FREE_NULL
	if (block==NULL)
		_xalloc_error_handler("xfree(NULL)"   UUFLARGS);
#endif

#if (defined(MEM_USED) || defined(LOGF_SAVE) || defined(HEAP_USED))
#ifdef __XALLOC_SRCNAME
	size = _xblksize_src(block UUFLARGS);
#else
	size = _xblksize(block);
#endif
#elif (defined(BLK_CHECK))	//inaki gcc-4.5
	_assertblk(block   UUFLARGS);
#elif (defined(HEAP_CHECK))	//inaki gcc-4.5
	_assertheap(IUFLARGS);
#endif

#ifdef LOGF_SAVE
	count = (STARTER(block))->logf_count;
#endif

#ifdef MEM_LIM
	FREE((((INT8 XPTR)block)-sizeof(mem_limiter)));
#else
	FREE(block);
#endif

#ifdef HEAP_CHECK
	fillheap();
#endif

#if (defined(MEM_USED) || defined(LOGF_SAVE))
	_mem_used -= size;
#endif
#ifdef HEAP_USED
	_heap_used -= size;
#ifdef MEM_LIM
	_heap_used -= sizeof(mem_limiter);
#endif
#ifdef BLK_CHECK
	_heap_used -= sizeof(mem_limiter);
#endif
#endif /* HEAP_USED */

	_show_used();

#ifdef LOGF_SAVE
	if (logf_save_on) {
		if (count>=0)	_logf_log(count,size,1 UUFLARGS);
	}
#endif
}

/**********************************************************/
/* {devuelve} la memoria en uso con proteccion, o 0 si no hay
informacion disponible */

unsigned long xusedmem( void )
{
#if (defined(MEM_USED) || defined(LOGF_SAVE))
	return _mem_used;
#else
	return 0;
#endif
}

/**********************************************************/
/* {devuelve} el heap total en uso, o 0 si no hay
informacion disponible */

unsigned long xusedheap( void )
{
#ifdef HEAP_USED
	return _heap_used;
#else
	return 0;
#endif
}

/**********************************************************/
/* duplica una cadena de forma protegida */

#ifdef __XALLOC_SRCNAME
char * _xstrdup_src( const char * str   FLARGS )
#else
char * _xstrdup( const char * str )
#endif
{
	char * s;

#ifdef __XALLOC_SRCNAME
	s = (char *)_xmalloc_src(sizeof(char)*(STRLEN(str)+1)  UUFLARGS);
#else
	s = (char *)_xmalloc(sizeof(char)*(STRLEN(str)+1));
#endif

	if (s)
		STRCPY(s,str);

	return s;
}

/**********************************************************/
/* {devuelve} el numero de bytes reservados en un bloque protegido,
o 0 si no hay posibilidad de devolver dicha informacion (es necesario
que el debugger BLK_CHECK este activo).
Se chequea que el bloque y la heap no esten corruptos */

#ifdef __XALLOC_SRCNAME
unsigned long _xblksize_src( void * block   FLARGS )
#else
unsigned long _xblksize( void * block )
#endif
{
#ifdef __XALLOC_SRCNAME
	_assertblk_src(block   UUFLARGS);
#else
	_assertblk(block);
#endif

#ifdef BLK_CHECK
	return (STARTER(block))->blklen;
#else
	return 0;
#endif
}

/**********************************************************/
#ifdef BLK_CHECK

PRIVATE int _test_limiter( mem_limiter XPTR m )
{
#if (BLK_LIMITER_SIZE>0)
	int i;

	for (i=0; i<BLK_LIMITER_SIZE; i++)
		if ((m->s0[i]!=(INT8)(BLK_FILLER+i))||
				(m->s1[i]!=(INT8)(BLK_FILLER+BLK_LIMITER_SIZE+i))||
				(m->s2[i]!=(INT8)(BLK_FILLER+BLK_LIMITER_SIZE*2+i))||
				(m->s3[i]!=(INT8)(BLK_FILLER+BLK_LIMITER_SIZE*3+i))||
				(m->blklen!=m->blklen1) )
			return XALLOC_BLK_CORRUPTED;
#endif

	return XALLOC_BLK_NOT_CORRUPTED;
}

#endif

/**********************************************************/
/* {devuelve} un codigo de corrupcion de bloque para el bloque
protegido {block} (si estan activados los debuggers adecuados). */

int testblk( void * block )
{
#ifdef BLK_CHECK
	mem_limiter XPTR ps;
	mem_limiter XPTR pe;
#endif

	if (block==NULL)
#ifdef NULL_TRAP
		return XALLOC_BLK_NULLPTR;
#else
		return XALLOC_BLK_NOT_CORRUPTED;
#endif

#ifdef BLK_CHECK
	ps = STARTER(block);
	if (_test_limiter(ps)!=XALLOC_BLK_NOT_CORRUPTED)
		return XALLOC_BLK_CORRUPTED_START;

	pe = ENDER(block,ps->blklen);

	if (_test_limiter(pe)!=XALLOC_BLK_NOT_CORRUPTED)
		return XALLOC_BLK_CORRUPTED_END;

	if (ps->blklen!=pe->blklen)
		return XALLOC_BLK_CORRUPTED;
#endif

	return XALLOC_BLK_NOT_CORRUPTED;
}

/**********************************************************/
/* {devuelve} codigo de estado de corrupcion de la heap */

int testheap( void )
{
#ifdef HEAP_CHECK
	int i;

  if (!heap_check_on) return XALLOC_HEAP_NOT_CORRUPTED;

	if (!_heap_filled) {
		_heap_filled = 1;
		HEAPFILLFREE(HEAP_FILLER);
	}

	if (HEAPCHECK()!=HEAP_OK) return XALLOC_HEAP_CORRUPTED;

	i = HEAPCHECKFREE(HEAP_FILLER);
	if (i==HEAP_CORRUPT) return XALLOC_HEAP_CORRUPTED;
	else if (i==HEAP_FREEWRITE) return XALLOC_HEAP_WRITE_IN_FREE_MEM;
	else if (i!=HEAP_OK) return XALLOC_HEAP_VERY_CORRUPTED;
#endif

	return XALLOC_HEAP_NOT_CORRUPTED;
}

/**********************************************************/
/* comprueba si un bloque protegido {block} esta corrupto, en cuyo
caso imprime un mensaje y termina el programa. Despues comprueba
el estado general de la heap, si esta activo este debugger */

#ifdef __XALLOC_SRCNAME
void _assertblk_src( void * block   FLARGS )
#else
void _assertblk( void * block )
#endif
{
#ifdef BLK_CHECK
	switch (testblk(block)) {
	case XALLOC_BLK_CORRUPTED_START :
		_xalloc_error_handler("out of range in memblock (begin)"   UUFLARGS);
	case XALLOC_BLK_CORRUPTED_END :
		_xalloc_error_handler("out of range in memblock (end)"   UUFLARGS);
	case XALLOC_BLK_CORRUPTED :
		_xalloc_error_handler("out of range in memblock (???)"   UUFLARGS);
	case XALLOC_BLK_NULLPTR :
		_xalloc_error_handler("NULL pointer"   UUFLARGS);
	}
#endif

#ifdef HEAP_CHECK
#ifdef __XALLOC_SRCNAME
	_assertheap_src(IUFLARGS);
#else
	_assertheap();
#endif
#endif
	(void)block;  /* quita warning arg. not used */
#ifdef __XALLOC_SRCNAME
	(void)srcname;  /* quitar warnings */
	(void)line;
#endif
}

/**********************************************************/

#ifdef __XALLOC_SRCNAME
VOID _assertheap_src( IFLARGS )
#else
VOID _assertheap( VOID )
#endif
{
#ifdef HEAP_CHECK
	switch (testheap()) {
	case XALLOC_HEAP_CORRUPTED :
		_xalloc_error_handler("corrupted heap" UUFLARGS);
	case XALLOC_HEAP_WRITE_IN_FREE_MEM :
		_xalloc_error_handler("free heap overwritten" UUFLARGS);
	case XALLOC_HEAP_VERY_CORRUPTED :
		_xalloc_error_handler("high corruption in heap!" UUFLARGS);
	}
#elif defined(__XALLOC_SRCNAME)
	(void)srcname;  /* quitar warnings */
	(void)line;
#endif
}

/**********************************************************/

void xalloc_logf(int b)
{
#ifdef LOGF_SAVE
	logf_save_on = (BOOL)b;
#else
	(void)b;
#endif
}

/**********************************************************/

void xalloc_heapcheck( int b )
{
#ifdef HEAP_CHECK
	heap_check_on = (BOOL)b;
#else
	(void)b;
#endif
}

/**********************************************************/

void xalloc_showmem(int b)
{
#ifdef MEM_USED
	mem_used_on = (BOOL)b;
#else
	(void)b;
#endif
}

/**********************************************************/

void xalloc_showheap(int b)
{
#ifdef HEAP_USED
	heap_used_on = (BOOL)b;
#else
	(void)b;
#endif
}

/**********************************************************/

void xalloc_showfree(int b)
{
#ifdef HEAP_FREE
	heap_free_on = (BOOL)b;
#else
	(void)b;
#endif
}

/**********************************************************/
#endif   /* _XALLOC_XFUNCS */

/**********************************************************/
/* vuelca un bloque protegido a disco, y libera la memoria.
el bloque se envia en {block} y el fichero destino se
llama {fname} */

#ifdef __XALLOC_SRCNAME
void _xmem2disk_src( char * fname, void * block   FLARGS )
#else
void _xmem2disk( char * fname, void * block )
#endif
{
#ifndef MEM_LIM
	die_beep("error: xmem2disk not compiled in");
  (void)fname;
  (void)block;
#if __XALLOC_SRCNAME
	(void)srcname;  /* quitar warnings */
	(void)line;
#endif

#else
	FILE * f;
	unsigned long l, i;

#ifdef __XALLOC_SRCNAME
	l = _xblksize_src(block  UUFLARGS);
#else
	l = _xblksize(block);
#endif

	if ((f=fopen(fname,"wb"))==NULL)
		_xalloc_error_handler("error saving memblock (fopen)" UUFLARGS);

	if (fwrite(STARTER(block),sizeof(mem_limiter),1,f)!=1)
		_xalloc_error_handler("error saving memblock (fwrite hdr)" UUFLARGS);

#define N 1000
  i=0;
  while (i+N<=l) {
		if (fwrite((void *)XBPTR(block,i),N,1,f)!=1)
			_xalloc_error_handler("error saving memblock (fwrite blk)" UUFLARGS);
    i+=N;
  }
  if (i<l) {
  	if (fwrite((void *)XBPTR(block,i),(size_t)(l-i),1,f)!=1)
			_xalloc_error_handler("error saving memblock (fwrite blk)" UUFLARGS);
  }
#undef N

#ifdef BLK_CHECK
	if (fwrite(ENDER(block,l),sizeof(mem_limiter),1,f)!=1)
		_xalloc_error_handler("error saving memblock (fwrite ehdr)" UUFLARGS);
#endif

	if (fclose(f)==EOF)
		_xalloc_error_handler("error saving memblock (fclose)" UUFLARGS);

#ifdef __XALLOC_SRCNAME
  _xfree_src(block UUFLARGS);
#else
  _xfree(block);
#endif

#endif
}

/**********************************************************/
/* lee un bloque protegido de disco, contenido en el fichero
de nombre {fname}, y {devuelve} un puntero al bloque
leido. El usuario debe liberar el bloque con xfree() */

#ifdef __XALLOC_SRCNAME
void * _xdisk2mem_src( char * fname   FLARGS )
#else
void * _xdisk2mem( char * fname )
#endif
{
#ifndef MEM_LIM
	die_beep("error: xdisk2mem not compiled in");
  (void)fname;
#if __XALLOC_SRCNAME
	(void)srcname;  /* quitar warnings */
	(void)line;
#endif
	return NULL;

#else

	FILE * f;
	mem_limiter m;
	unsigned long l, i;
	void * p;

	if ((f=fopen(fname,"rb"))==NULL)
		_xalloc_error_handler("error loading memblock (fopen)" UUFLARGS);

	if (fread(&m,sizeof(mem_limiter),1,f)!=1)
		_xalloc_error_handler("error loading memblock (fread hdr)" UUFLARGS);
#ifdef BLK_CHECK
	if (_test_limiter(&m)!=XALLOC_BLK_NOT_CORRUPTED)
		_xalloc_error_handler("error loading memblock (test hdr)" UUFLARGS);
#endif
	l=m.blklen;

#ifdef __XALLOC_SRCNAME
	p=_xmalloc_src(l UUFLARGS);
#else
	p=_xmalloc(l);
#endif

#define N 1000
  i=0;
  while (i+N<=l) {
		if (fread((void *)XBPTR(p,i),N,1,f)!=1)
			_xalloc_error_handler("error loading memblock (fread blk)" UUFLARGS);
    i+=N;
  }
  if (i<l) {
  	if (fread((void *)XBPTR(p,i),(size_t)(l-i),1,f)!=1)
			_xalloc_error_handler("error loading memblock (fread blk)" UUFLARGS);
  }
#undef N

#ifdef BLK_CHECK
	if (fread(&m,sizeof(mem_limiter),1,f)!=1)
		_xalloc_error_handler("error loading memblock (fread ehdr)" UUFLARGS);
#ifdef BLK_CHECK
	if (_test_limiter(&m)!=XALLOC_BLK_NOT_CORRUPTED)
		_xalloc_error_handler("error loading memblock (test ehdr)" UUFLARGS);
#endif
	if (l!=m.blklen)
		_xalloc_error_handler("error loading memblock (comp hdr-ehdr)" UUFLARGS);
#endif

#ifdef __XALLOC_SRCNAME
	_assertblk_src(p UUFLARGS);
#else
	_assertblk(p);
#endif

	if (fclose(f)==EOF)
		_xalloc_error_handler("error loading memblock (fclose)" UUFLARGS);

	return (p);
#endif
}

/**********************************************************/
