#ifndef __KINDOF_HPP__
#define __KINDOF_HPP__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1997 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ KINDOF.HPP
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.1    08/10/98  Borja     Punto y coma en DECL(), versiones INLINE;
1.0.0    17/09/97  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Macros para implementar metodo isKindOf() y className
en una clase.

Estos metodos tienen estos prototipos:
virtual BOOL isKindOf(const CHAR *s );
virtual const CHAR *className( VOID );

El primero permite comprobar si un objeto es de la clase nombrada
por la cadena {s} (funciona tambien con clases derivadas). El
segundo {devuelve} una cadena estatica interna que contiene el
nombre de la clase del objeto.

Se utiliza asi:
El la declaracion de la clase, en una seccion public
anyadir esto:
        KINDOF_DECL();

En algun fichero de implementacion de la clase, anyadir esto:
        KINDOF_BDEF(cname)
o bien esto:
        KINDOF_DDEF(cname,cbase)
El primero es para clases base, y el segundo para clases derivadas,
siendo {cname} el nombre de la clase y {cbase} el nombre de la clase
de la que derivamos.
Cuidado con el punto y coma final, no hay que ponerlo salvo
en KINDOF_DECL !!

Tambien hay versiones inline para declarar directamente en el .hpp.
En tal caso no hace falta usar KINDOF_DECL() y directamente se puede
poner en la declaracion de la clase:
        KINDOF_BDEF_INLINE(cname);
o bien esto:
        KINDOF_DDEF_INLINE(cname,cbase);

</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <string.h>

/**********************************************************/

#define KINDOF_DECL() \
	virtual BOOL isKindOf(const CHAR *s ) const; \
        virtual const CHAR *className( VOID ) const

#define KINDOF_BDEF(cname) \
	BOOL cname::isKindOf(const CHAR *s ) const { return !strcmp(#cname,s); } \
	const CHAR * cname::className( VOID ) const { return #cname; }

#define KINDOF_DDEF(cname,cbase) \
	BOOL cname::isKindOf(const CHAR *s ) const { return strcmp(#cname,s)?cbase::isKindOf(s):TRUE; } \
	const CHAR * cname::className( VOID ) const { return #cname; }

#define KINDOF_BDEF_INLINE(cname) \
	virtual BOOL isKindOf(const CHAR *s ) const { return !strcmp(#cname,s); } \
	virtual const CHAR * className( VOID ) const { return #cname; }

#define KINDOF_DDEF_INLINE(cname,cbase) \
	virtual BOOL isKindOf(const CHAR *s ) const { return strcmp(#cname,s)?cbase::isKindOf(s):TRUE; } \
	virtual const CHAR * className( VOID ) const { return #cname; }

/**********************************************************/

#endif

