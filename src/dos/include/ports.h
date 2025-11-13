#ifndef __PORTS_H
#define __PORTS_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ PORTS.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Puertos
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Macros para leer/escribir en un puerto.
....................
Macros to read/write a port
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#include <dos.h>   /* para outportb() e inportb() */

/* escribir {value} (UINT8) a un puerto {port} (UINT16)
....................
writes {value} (byte, UINT8) to port {port} (UINT16). */
#define OUTPORT8(port,value)  outportb(port,value)
/* macro que lee un octeto (UINT8) de un puerto {port} (UINT16)
....................
reads a byte (UINT8) from port {port} (UINT16) */
#define INPORT8(port)  inportb(port)

/* escribir {value} (UINT16) a un puerto {port} (UINT16)
....................
writes {value} (int, UINT16) to port {port} (UINT16). */
#define OUTPORT16(port,value)  outport(port,value)
/* macro que lee un UINT16 de un puerto {port} (UINT16)
....................
reads an int (UINT16) from port {port} (UINT16) */
#define INPORT16(port)  inport(port)

/**********************************************************/

#endif

