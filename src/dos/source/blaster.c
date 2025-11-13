/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ BLASTER.C
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Sound Blaster
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.0    24/08/94  Borja     Manejo dec/hex. Param {hex} en blaster_env().
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Lectura de configuracion de tarjetas SoundBlaster a partir
de la variable de entorno BLASTER.
....................
Gets SoundBlaster card configuration reading the BLASTER
environment variable.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "tdef.h"
#include "blaster.h"

/**********************************************************/
/* {devuelve} el valor del flag {code} o -1 en caso de error.
Se escanea la cadena {env_str} en busca del caracter  {code},
y se recupera el valor hexadecimal (si {hex}=TRUE) o decimal
(si {hex}=FALSE) indicado a continuacion.
....................
{returns} the value for flag {code} or -1 if error.
The {env_str} environment string is searched for character
{code}, and the function {returns} the numerical value appended
to that character. The numerical value may be an hexa (send TRUE
in {hex}) or decimal (send FALSE in {hex}) */

PRIVATE INT blaster_field_value( CHAR code, pCHAR env_str, BOOL hex )
{
	pCHAR c;
	int n;

	c=strchr(env_str,code);
	if (c!=NULL) {
		if (sscanf(c+1,((hex)?"%x":"%d"),&n) != 1)
			return -1;
		else
			return (INT)n;
	}
	else
		return -1;
}

/**********************************************************/
/* {devuelve} el valor que tiene el campo {field} dentro de
la variable de entorno BLASTER. Si sucede algun error (la
variable BLASTER no esta definida, o el campo {field} no esta
definido, se {devuelve} el valor por defecto {default_value}.
La variable BLASTER es de la forma: BLASTER=Fnn Fnn ...
En {hex} se debe indicar la base numerica del campo: FALSE para
decimal, TRUE para hexadecimal.
donde F es el caracter que identifica al campo (caracter {field})
y nn es el valor numerico hexadecimal/decimal correspondiente.
En BLASTER.H estan definidos los campos {field} que se pueden
sondear (constantes BLASTER_????) y los valores por
defecto que se pueden utilizar BLASTER_DEFAULT_????
....................
{returns} the value for the field {field} in the BLASTER
environment variable. If error (BLASTER not defined, {field}
not valid...) {returns} the default value {default_value}.
You must send un {hex} the field value numerical base : FALSE for
decimal base and TRUE for hexadecimal base.
The BLASTER env. var. sintaxis is:  BLASTER=Fnn Fnn ... where
F is the field identifier character ({field} parameter)
and nn is a numerical hex/decimal value. The valid {field} values
for SB16, the default values you can use and the base (hex/decimal)
for each one, are defined as constants in BLASTER.H */

INT blaster_env( CHAR field, INT default_value, BOOL hex )
{
	pCHAR e;
	INT i;

	if ((e=getenv("BLASTER")) != NULL) {
		if ((i=blaster_field_value(toupper(field),e,hex)) != -1)
			return i;
		if ((i=blaster_field_value(tolower(field),e,hex)) != -1)
			return i;
	}

	return default_value;
}

/**********************************************************/

