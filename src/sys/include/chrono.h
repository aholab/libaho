#ifndef __CHRONO_H
#define __CHRONO_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ CHRONO.H
Nombre paquete............... -
Lenguaje fuente.............. C (BC31/GCC)
Estado....................... Utilizable
Dependencia Hard/OS.......... NO
Codigo condicional........... NO

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.0    17/09/92  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera de CHRONO.C.
Implementacion de un cronometro.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

typedef struct {
	unsigned long _ms;
    int _on;
  } chrono;

/**********************************************************/

void chrono_initialize( chrono * c );
void chrono_reset( chrono * c );
void chrono_start( chrono * c );
void chrono_stop( chrono * c );
void chrono_restart( chrono * c );
void chrono_restop( chrono * c );
int chrono_on( chrono * c );
unsigned long chrono_ms( chrono * c );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
