/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL9.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... En desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    24/07/95  Borja     correccion nombre funcion bk?_fi2fc
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    18/12/93  Borja     Codificacion inicial sin concluir

======================== Contenido ========================
Gestion de bloques de vectores de voz (tramas y ventanas).


.                longitud o
.             desplazamiento
.              de trama {fl}
.              |<-------->|
.              |          |
.     trama 0  | trama 1  | trama 2    trama 3             {fn} (tramas)
.   |----------|----------|----------|----------|----------|---------->
.   0      \                  /    3*{fl}     4*{fl}        {n} (muestras)
.            \   ventana 1  /
.              \__________/
.               ventana de
.              longitud {wl}


Codigos para los nombres (unidades entre parentesis).
Las unidades comienzan en 0 (la primera trama es la 0, la primera
muestra es la 0).

.    {bk}  Abreviatura de bloque (-)
.    {fr}  Abreviatura de 'trama' (-)
.    {fs}  Comienzo de trama (muestras)
.    {fe}  Fin de trama (muestras)
.    {fc}  Centro de la trama (muestras)
.    {fl}  Longitud (desplazamiento) de trama (muestras)
.    {fi}  Numero de trama, con primera trama=0 (trama)
.    {fn}  cantidad de tramas, con una trama=1 (tramas)

.    {wi}  Abreviatura de 'ventana' (-)
.    {ws}  Comienzo de ventana (muestras)
.    {we}  Fin de ventana (muestras)
.    {wc}  Centro de la ventana (muestras)
.    {wl}  Longitud de la ventana de analisis (muestras)
.    {wi}  Numero de ventana, con primera ventana=0 (ventana)
.    {wn}  cantidad de ventanas, con una ventana=1 (ventanas)

.    {n}   Cantidad de muestras, con {n}=0 para 0 muestras (muestras)
.    {i}   Numero de muestra, con {i}=0 para la primera muestra (muestras)

Hay 3 conjuntos de funciones

.  bk_???() funciones generales sobre bloques (tramas/ventanas)
.  bki_???() funciones sobre tramas/ventanas para trabajar con
.            numeros enteros tipo SPL_INT.
.  bkd_???() funciones sobre tramas/ventanas para trabajar con
.			 valores generalizados de tipo SPL_FLOAT. Ademas imponen
.            menos restricciones (assert) a los parametros de entrada.

Por ejemplo, el valor {fc} que indica cual es el centro de una trama,
puede estar entre dos muestras (trama de longitud par). Las
funciones bki_???() utilizan este valor redondeado hacia abajo (la muestra
inferior) mientras que las funciones bkd_???() utilizan el valor exacto
con decimales). Por poder, hasta admiten tramas de longitud no entera,
aunque no tenga mucho sentido...
Otro ejemplo para bkd_???(): para un desplazamiento {fl}=5 muestras,
la muestra {i}=2 esta en la trama {fi}=0, la muestra {i}=7, en la trama
{fi}=1, la muestra {i}=3 esta en la 'trama' {fi}=0.2, y la 'muestra'
{i}=4.5 esta en la 'trama' {fi}=0.5.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/**********************************************************/
/* calcula el entorno necesario para un analisis por tramas. Dada
la longitud de trama en {fl} (desplazamiento), la longitud de
la ventana de analisis en {wl}, y si la trama de desplazamiento
necesita unas 'alas' anterior y posterior de {prefr} y {posfr},
y la ventana de analisis unas alas anterior y posterior de {prewi} y
{poswi}, esta funcion calcula en {tprefr} cuantas muestras deben
almacenarse en un buffer por delante de la primera muestra de la trama,
en {tposfr} cuantas deben almacenarse por detras, y de forma similar
indica en {tprewi} cuantas muestras hay en ese mismo buffer antes
de la primera muestra de la ventana, y en {tposwi} cuantas hay por
detras. En cualquier caso, tanto para la trama como para la ventana
el buffer debe ser de la misma longitud, por lo que

.    {tprefr}+{fl}+{tposfr} == {tprewi}+{wl}+{tposwi}

{wl} puede ser mayor o menor que {fl}, y todos los parametros
deben ser mayores o iguales que cero, excepto {fl} que debe
ser mayor que cero.
Cuando la ventana no encaje de forma simetrica con una trama (porque
{wl} y {fl} son de diferente paridad) entonces la ventana SIEMPRE se
redondea hacia abajo, tanto si es mayor como si es menor que la trama.
Si no interesa alguno de los valores de retorno ({tprefr},{tposfr},
{tprewi},{tposwi} enviar puntero NULL en su lugar.

La funcion {devuelve} la diferencia en muestras entre el comienzo de
la trama y el comienzo de la ventana para un desplazamiento {fl} y una
longitud de ventana {wl}.
El valor devuelto indica el numero de muestras que hay que RESTAR a
un comienzo de trama {fs} para obtener el comienzo de ventana {ws}.
Por tanto, si el valor devuelto es positivo la ventana comienza antes
que la trama, si es negativo la ventana comienza despues de la trama,
y si es nulo, comienzan en la misma muestra.
Este valor es tambien facilmente calculable a partir de los otros valores
devueltos por la funcion (vale {tprefr}-{tprewi}). */

PUBLIC SPL_INT XAPI bki_env( SPL_INT fl, SPL_INT prefr, SPL_INT posfr,
		SPL_INT wl, SPL_INT prewi, SPL_INT poswi,
		SPL_pINT tprefr, SPL_pINT tposfr,
		SPL_pINT tprewi, SPL_pINT tposwi )
{
	SPL_INT pre, pos, ppre, ppos;
	assert(fl>0);
	assert(prefr>=0);
	assert(posfr>=0);
	assert(wl>=0);
	assert(prewi>=0);
	assert(poswi>=0);

	if (fl<wl) {  /* desp.vent<long.vent. */
		ppos = (SPL_INT)(wl-fl)/(SPL_INT)2; /* ventana tras trama */
		ppre = wl-fl-ppos; /* ventana antes trama */
		pos = ppos+poswi;  /* total ventana despues trama */
		pre = ppre+prewi;  /* total ventana antes trama */
		if (pos<posfr)  /* mas despues trama si hace falta */
			pos = posfr;
		if (pre<prefr)  /* mas antes trama si hace falta */
			pre = prefr;
		if (tprefr!=NULL)
			*tprefr = pre;
		if (tposfr!=NULL)
			*tposfr = pos;
		if (tprewi!=NULL)
			*tprewi = pre-ppre;
		if (tposwi!=NULL)
			*tposwi = pos-ppos;

		return ppre;
	}
	else {  /* desp.vent>=long.vent */
		ppre = (SPL_INT)(fl-wl)/(SPL_INT)2; /* trama antes ventana */
		ppos = fl-wl-ppre; /* trama tras ventana */
		pos = ppos+posfr;  /* total trama despues ventana */
		pre = ppre+prefr;  /* total trama antes ventana */
		if (pos<poswi)  /* mas despues ventana si hace falta */
			pos = poswi;
		if (pre<prewi)  /* mas antes ventana si hace falta */
			pre = prewi;
		if (tprefr!=NULL)
			*tprefr = pre-ppre;  /* trama empieza en antes-trama antes ventana */
		if (tposfr!=NULL)
			*tposfr = pos-ppos;  /* trama termina en despues-trama tras ventana */
		if (tprewi!=NULL)
			*tprewi = pre;  /* ventana empieza en antes ventana */
		if (tposwi!=NULL)
			*tposwi = pos;

		return -ppre;
	}
}

/**********************************************************/
/* {devuelve} el numero de tramas de longitud {fl} (desplazamiento)
que se pueden obtener a partir de un vector de {n} muestras.
La ultima trama, en general, no estara completa.
{n}>=0, {fl}>0 */

PUBLIC SPL_LONG XAPI bki_n2fn( SPL_INT fl, SPL_LONG n )
{
	assert(n>=0);
	assert(fl>0);

	return (SPL_LONG)((n+fl-1)/fl);
}

/**********************************************************/
/* {devuelve} el numero de muestras que se pueden obtener a partir
de {fn} tramas de longitud {fl} muestras cada una, aunque si
la ultima trama no esta completa, el numero puede ser menor.
{fn}>=0, {fl}>0 */

PUBLIC SPL_LONG XAPI bki_fn2n( SPL_INT fl, SPL_LONG fn )
{
	assert(fl>0);
	assert(fn>=0);

	return (SPL_LONG)(fl*fn);
}

/**********************************************************/
/* {devuelve} la muestra en la que comienza la trama numero {fi}
para un desplazamiento de trama {fl}.
{fl} > 0 */

PUBLIC SPL_LONG XAPI bki_fi2fs( SPL_INT fl, SPL_LONG fi )
{
	assert(fl>0);

	return (SPL_LONG)(fl*fi);
}

/**********************************************************/
/* {devuelve} la muestra en la que termina la trama numero {fi}
para un desplazamiento de trama {fl}.
{fl} > 0 */

PUBLIC SPL_LONG XAPI bki_fi2fe( SPL_INT fl, SPL_LONG fi )
{
	assert(fl>0);

	return (SPL_LONG)(fl*(fi+1)-1);
}

/**********************************************************/
/* {devuelve} la muestra central (redondeada hacia abajo) de
la trama numero {fi} para un desplazamiento de trama {fl}.
{fl} > 0 */

PUBLIC SPL_LONG XAPI bki_fi2fc( SPL_INT fl, SPL_LONG fi )
{
	assert(fl>0);

	return (SPL_LONG)(fl*fi+(SPL_LONG)((fl-1)/2));
}

/**********************************************************/
/* {devuelve} la trama en la que esta situada la muestra {i}
para un desplazamiento de trama {fl}.
{fl} > 0 */

PUBLIC SPL_LONG XAPI bki_i2fi( SPL_INT fl, SPL_LONG i )
{
	assert(fl>0);

	return (SPL_LONG)(i/fl);
}

/**********************************************************/
/* similar a bli_env() pero utiliza valores fraccionarios.
Ademas no impone restricciones a los parametros de entrada */

PUBLIC SPL_FLOAT XAPI bkd_env( SPL_FLOAT fl, SPL_FLOAT prefr,
		SPL_FLOAT posfr, SPL_FLOAT wl, SPL_FLOAT prewi, SPL_FLOAT poswi,
		SPL_pFLOAT tprefr, SPL_pFLOAT tposfr,
		SPL_pFLOAT tprewi, SPL_pFLOAT tposwi )
{
	SPL_FLOAT pre, pos, ppre, ppos;

	if (fl<wl) {  /* desp.vent<long.vent. */
		ppos = (wl-fl)/2.0; /* ventana tras trama */
		ppre = wl-fl-ppos;  /* ventana antes trama */
		pos = ppos+poswi;  /* total ventana despues trama */
		pre = ppre+prewi;  /* total ventana antes trama */
		if (pos<posfr)  /* mas despues trama si hace falta */
			pos = posfr;
		if (pre<prefr)  /* mas antes trama si hace falta */
			pre = prefr;
		if (tprefr!=NULL)
			*tprefr = pre;
		if (tposfr!=NULL)
			*tposfr = pos;
		if (tprewi!=NULL)
			*tprewi = pre-ppre;
		if (tposwi!=NULL)
			*tposwi = pos-ppos;

		return ppre;
	}
	else {  /* desp.vent>=long.vent */
		ppre = (fl-wl)/2.0; /* trama antes ventana */
		ppos = fl-wl-ppre;  /* trama tras ventana */
		pos = ppos+posfr;  /* total trama despues ventana */
		pre = ppre+prefr;  /* total trama antes ventana */
		if (pos<poswi)  /* mas despues ventana si hace falta */
			pos = poswi;
		if (pre<prewi)  /* mas antes ventana si hace falta */
			pre = prewi;
		if (tprefr!=NULL)
			*tprefr = pre-ppre;  /* trama empieza en antes-trama antes ventana */
		if (tposfr!=NULL)
			*tposfr = pos-ppos;  /* trama termina en despues-trama tras ventana */
		if (tprewi!=NULL)
			*tprewi = pre;  /* ventana empieza en antes ventana */
		if (tposwi!=NULL)
			*tposwi = pos;

		return -ppre;
	}
}

/**********************************************************/
/* {devuelve} el numero de tramas de longitud {fl} (desplazamiento)
que se pueden obtener a partir de un vector de {n} muestras.
El valor devuelto puede ser fraccionario, ya que la ultima trama,
en general, no estara completa.
{fl}!=0 */

PUBLIC SPL_FLOAT XAPI bkd_n2fn( SPL_FLOAT fl, SPL_FLOAT n )
{
	assert(fl!=0.0);

	return (n/fl);
}

/**********************************************************/
/* {devuelve} el numero de muestras que se pueden obtener a partir
de {fn} tramas de longitud {fl} muestras cada una. Si
la ultima trama no esta completa, {fn} puede ser fraccionario. */

PUBLIC SPL_FLOAT XAPI bkd_fn2n( SPL_FLOAT fl, SPL_FLOAT fn )
{
	return (fl*fn);
}

/**********************************************************/
/* {devuelve} la muestra en la que comienza la trama numero {fi}
para un desplazamiento de trama {fl} */

PUBLIC SPL_FLOAT XAPI bkd_fi2fs( SPL_FLOAT fl, SPL_FLOAT fi )
{
	return (fl*fi)-0.5;
}

/**********************************************************/
/* {devuelve} la muestra en la que termina la trama numero {fi}
para un desplazamiento de trama {fl} */

PUBLIC SPL_FLOAT XAPI bkd_fi2fe( SPL_FLOAT fl, SPL_FLOAT fi )
{
	return (fl*(fi+1.0)-0.5);
}

/**********************************************************/
/* {devuelve} la muestra central de la trama numero {fi} para un
desplazamiento de trama {fl}. */

PUBLIC SPL_FLOAT XAPI bkd_fi2fc( SPL_FLOAT fl, SPL_FLOAT fi )
{
	return (fl*fi+((fl-1.0)/2.0));
}

/**********************************************************/
/* {devuelve} la trama (fraccionaria) en la que esta situada
la muestra {i} para un desplazamiento de trama {fl}.
{fl}!=0 */

PUBLIC SPL_FLOAT XAPI bkd_i2fi( SPL_FLOAT fl, SPL_FLOAT i )
{
	assert(fl!=0.0);

	return (i-(fl-1.0)/2.0)/fl;
}

/**********************************************************/

