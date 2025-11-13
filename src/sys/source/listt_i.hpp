#ifndef __LISTT_I_HPP
#define __LISTT_I_HPP

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ LISTT.CPP
Nombre paquete............... -
Lenguaje fuente.............. C++
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.1    30/08/98  Borja     split varios modulos listt_?.cpp

======================== Contenido ========================
ver doc. en listt.cpp
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "listt.hpp"
#include "uti.h"

/**********************************************************/

#ifdef LIST_PTRVERIFY
#define LISTVERIFY(p,func) cdie_beep(!owns(p),"List %s(): invalid Lix",func)
#define XLISTVERIFY(l,p,func) cdie_beep(!l.owns(p),"List %s(): invalid Lix",func)
#define LISTNONULL(p,func) cdie_beep(p==0,"List %s(): null Lix",func)
#define LISTERROR(func,cause) die_beep("List %s(): %s",func,cause)
#else
#define LISTVERIFY(p,func) {}
#define XLISTVERIFY(l,p,func) {}
#define LISTNONULL(p,func) {}
#define LISTERROR(func,cause) { (void)func; }
#endif

/**********************************************************/

#define __SWAP__(a,b,tmp) { tmp=a; a=b; b=tmp; }

/**********************************************************/

#ifdef LIST_PTRVERIFY
#define LIX_SAFE(i,f,a) __lix_safe(i,f,a)
#else
#define LIX_SAFE(i,f,a) lix(i)
#endif

/**********************************************************/

#endif
