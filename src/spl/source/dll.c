/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ DLL.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... entrada para DLL de MS-Windows
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    04/03/97  Borja     Codificacion inicial.

======================== Contenido ========================
Funcion principal (LibMain) para hacer una DLL de MS-Windows
con las funciones SPL.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

#include <windows.h>

/**********************************************************/

int far pascal LibMain( HANDLE hInstance, WORD wDataSeg, WORD wHeapSize,
		LPSTR lpszCmdLine)
{
	return 1;
}

/**********************************************************/
