/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ STAT.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    04/03/97  Borja     eliminar warnings inocentes
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision de tipos.
1.0.0    11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
Calculo de estadisticos de primer y segundo orden.

NOTA: Para la reserva de memoria dinamica se han definido
dos macros en _STAT.H: stat_malloc() y stat_free(), que mapean
a las funciones _malloc() y _free(), definidas en _ALLOC.H.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "_stat.h"

/**********************************************************/
/* Constructor de una estructura stat.
{devuelve} un puntero a la estructura creada, o NULL si no se puede.
{nhistdivs}=numero de celdas para el histograma */

PUBLIC pSTAT XAPI stat_construct( SPL_INT nhistdivs )
{
	pSTAT st;
	assert(nhistdivs>=0);

	st = (pSTAT)stat_malloc(sizeof(STAT));
	if (st!=NULL) {
		st->_divs = nhistdivs;
		st->_hist = (SPL_pLONG)stat_malloc(
				sizeof(SPL_LONG)*nhistdivs);

		if (st->_hist!=NULL)
			stat_first(st);
		else {
			stat_destruct(st);
			st=NULL;
		}
	}

	return st;
}

/**********************************************************/
/* destructor para estructura stat {st} */

PUBLIC SPL_VOID XAPI stat_destruct( pSTAT st )
{
	assert(st!=NULL);

	if (st->_hist!=NULL)
		stat_free(st->_hist);
	stat_free(st);
}

/**********************************************************/
/* llamar para reinicializar el calculo de estadisticos de primer
orden. Durante este analisis, los estadisticos de primer orden (media,
valor cuadratico medio, valor minimo, maximo, numero de elementos) se
actualizan constantemente mientras llegan nuevos datos,
mientras que los de segundo orden valen 0 */

PUBLIC SPL_VOID XAPI stat_first( pSTAT st )
{
	assert(st!=NULL);

	st->_sec=1;
	stat_second(st);
	st->_sec=0;

	st->_n=0;
	st->_mean=0.0;
	st->_msqv=0.0;
	st->_min=0.0;
	st->_max=0.0;
	st->_var=0.0;
	st->_range=0.0;
}

/**********************************************************/
/* pasa al sistema una ocurrencia {x} de la variable en estudio,
para analisis de primer o segundo orden. Esta es la funcion de
entrada de datos al sistema de analisis */

PUBLIC SPL_VOID XAPI stat_setvalue( pSTAT st, SPL_FLOAT x )
{
	SPL_INT i;
	assert(st!=NULL);

	st->_n++;
	if (st->_sec) {
		i = stat_histlocate(st,x);
		st->_var+=(x-st->_mean)*(x-st->_mean);
		if (i>=0)
			st->_hist[i]++;
		else if (i==-1)
			st->_under++;
		else if (i==-2)
			st->_over++;
	}
	else {
		st->_mean+=x;
		st->_msqv+=x*x;
		if (x<st->_min)
			st->_min=x;
		if (x>st->_max)
			st->_max=x;
	}
}

/**********************************************************/
/* {devuelve} la media hasta el momento (durante el analisis de primer
orden) o la definitiva (durante el analisis de segundo orden) */

PUBLIC SPL_FLOAT XAPI stat_mean( pSTAT st )
{
	if (st->_sec)
		return st->_mean;
	else if (st->_n)
		return st->_mean/st->_n;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} el valor cuadratico medio hasta el momento (durante
el analisis de primer orden) o el definitivo (durante el analisis
de segundo orden) */

PUBLIC SPL_FLOAT XAPI stat_msqv( pSTAT st )
{
	assert(st!=NULL);

	if (st->_sec)
		return st->_msqv;
	else if (st->_n)
		return st->_msqv/st->_n;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} el numero de ocurrencias de la variable analizada que se
llevan pasados al sistema mediante la funcion stat_setvalue() */

PUBLIC SPL_LONG XAPI stat_n( pSTAT st )
{
	assert(st!=NULL);

	return st->_n;
}

/**********************************************************/
/* {devuelve} el valor minimo sucedido hasta el momento (durante
el analisis de primer orden) o el definitivo (durante el analisis
de segundo orden) */

PUBLIC SPL_FLOAT XAPI stat_min( pSTAT st )
{
	assert(st!=NULL);

	return st->_min;
}

/**********************************************************/
/* {devuelve} el valor maximo sucedido hasta el momento (durante
el analisis de primer orden) o el definitivo (durante el analisis
de segundo orden) */

PUBLIC SPL_FLOAT XAPI stat_max( pSTAT st )
{
	assert(st!=NULL);

	return st->_max;
}

/**********************************************************/
/* Fuerza los valores minimo y maximo con {min} y {max}. Solo funciona
si se realiza durante el analisis de primer orden, forzando estos
como los nuevos extremos, aunque pueden variar si se continua
enviando valores con stat_setvalue() en analisis de primer orden. */

PUBLIC SPL_VOID XAPI stat_setminmax( pSTAT st,
		SPL_FLOAT min, SPL_FLOAT max )
{
	assert(st!=NULL);

	if (!st->_sec) {
		st->_min=min;
		st->_max=max;
	}
}

/**********************************************************/
/* Da por concluido el analisis de primer orden, calcula
los estadisticos de primer orden definitivos (media, valor
cuadratico medio, numero de elementos, valor minimo y maximo),
e inicializa el analisis de segundo orden. Si ya se estaba realizando un
analisis de segundo orden, simplemente reinicializa este analisis.
Durante el analisis de segundo orden los estadisticos de primer
orden no se modifican, aunque cambie la naturaleza de los datos.
Los estadisticos de segundo orden (varianza, desv. tipica
e histograma) se actualizan constantemente segun llegan nuevos
datos */

PUBLIC SPL_VOID XAPI stat_second( pSTAT st )
{
	SPL_INT i;
	assert(st!=NULL);

	if (!st->_sec) {
		st->_mean=stat_mean(st);
		st->_msqv=stat_msqv(st);
		st->_range=(st->_max-st->_min);
	}
	st->_n=0;
	st->_var=0.0;
	st->_sec=1;
	for (i=0; i<st->_divs; i++)
		st->_hist[i]=0;
	st->_under=0;
	st->_over=0;
}

/**********************************************************/
/* {devuelve} la varianza si se esta realizando un analisis de segundo
orden, o 0 si aun se esta en uno de primer orden */

PUBLIC SPL_FLOAT XAPI stat_variance( pSTAT st )
{
	assert(st!=NULL);

	if ((st->_sec)&&(st->_n))
		return st->_var/st->_n;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} la desviacion tipica si se esta realizando un analisis
de segundo orden, o 0 si aun se esta en uno de primer orden */

PUBLIC SPL_FLOAT XAPI stat_stddev( pSTAT st )
{
	assert(st!=NULL);

	return sqrt(stat_variance(st));
}

/**********************************************************/
/* {devuelve} el numero de divisiones que tiene el histograma */

PUBLIC SPL_INT XAPI stat_nhistdivs( pSTAT st )
{
	assert(st!=NULL);

	return st->_divs;
}

/**********************************************************/
/* {devuelve} el array de contadores del histograma */

PUBLIC SPL_pLONG XAPI stat_hists( pSTAT st )
{
	assert(st!=NULL);

	return st->_hist;
}

/**********************************************************/
/* {devuelve} el numero de valores acumulado por debajo
del rango del histograma */

PUBLIC SPL_LONG XAPI stat_histunder( pSTAT st )
{
	assert(st!=NULL);

	return st->_under;
}

/**********************************************************/
/* {devuelve} el numero de valores acumulado por encima del rango
del histograma */

PUBLIC SPL_LONG XAPI stat_histover( pSTAT st )
{
	assert(st!=NULL);

	return st->_over;
}

/**********************************************************/
/* {devuelve} la anchura de una celda del histograma. */

PUBLIC SPL_FLOAT XAPI stat_histdivwidth( pSTAT st )
{
	assert(st!=NULL);

	if (st->_divs)
		return st->_range/st->_divs;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} la cuenta (numero de ocurrencias) acumulada en la
celda {div} del histograma.
Si {div}<0 {devuelve} las acumuladas por debajo del minimo, y si
{div}>=numero de celdas (stat_nhistdivs()) {devuelve} las acumuladas
por encima del maximo */

PUBLIC SPL_LONG XAPI stat_histdivcount( pSTAT st, SPL_INT div )
{
	assert(st!=NULL);

	if (div<0)
		return st->_under;
	else if (div>=st->_divs)
		return st->_over;
	else
		return st->_hist[div];
}

/**********************************************************/
/* {devuelve} la cuenta (numero de ocurrencias) acumuladas desde menos
infinito hasta la celda {div} (incluida esta) del histograma.
Si {div}<0 {devuelve} las acumuladas por debajo del minimo del histograma
Si {div}>=numero de celdas (stat_nhistdivs()) entonces {devuelve} el total */

PUBLIC SPL_LONG XAPI stat_histdivacccount( pSTAT st, SPL_INT div )
{
	SPL_INT i;
	SPL_LONG a;
	assert(st!=NULL);

	a=0;
	for (i=-1; i<=div; i++)
		a += stat_histdivcount(st,i);

	return a;
}

/**********************************************************/
/* {devuelve} la probabilidad (0-1) de la celda {div} del
histograma. Es la funcion de densidad de probabilidad de
cada celda. Si se quiere calcular la funcion de densidad
de probabilidad en un punto, dividir este valor por la
anchura de la celda (stat_histdivwidth()).
Si {div}<0 {devuelve} la probabilidad de los valores por debajo del
minimo del histograma, y si {div}>=numero de celdas (stat_nhistdivs())
{devuelve} la probabilidad de los valores por encima del maximo del
histograma */

PUBLIC SPL_FLOAT XAPI stat_histdivprob( pSTAT st, SPL_INT div )
{
	assert(st!=NULL);

	if (st->_n)
		return ((SPL_FLOAT)stat_histdivcount(st,div))/st->_n;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} la probabilidad (0-1) acumulada desde menos infinito
hasta la celda {div} (inclusive).
Si {div}<0 {devuelve} la probabilidad por debajo del minimo del histograma
Si {div}>=numero de celdas (stat_nhistdivs()) entonces {devuelve} todo (1) */

PUBLIC SPL_FLOAT XAPI stat_histdivaccprob( pSTAT st, SPL_INT div )
{
	assert(st!=NULL);

	if (st->_n)
		return ((SPL_FLOAT)stat_histdivacccount(st,div))/st->_n;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} el extremo inferior de la celda {div}. Este valor
esta contenido en esta celda */

PUBLIC SPL_FLOAT XAPI stat_histdivmin( pSTAT st, SPL_INT div )
{
	assert(st!=NULL);

	if (st->_divs)
		return st->_min+(div*st->_range)/st->_divs;
	else
		return 0.0;
}

/**********************************************************/
/* {devuelve} el extremo superior de la celda {div}. Este valor
_no_ esta contenido en esta celda. Esta celda acumula ocurrencias
hasta justo antes de este valor. Este valor pertenece ya a la
celda siguiente. La excepcion es la ultima celda, que si incluye
al valor superior */

PUBLIC SPL_FLOAT XAPI stat_histdivmax( pSTAT st, SPL_INT div )
{
	assert(st!=NULL);

	return stat_histdivmin(st,div+1);
}

/**********************************************************/
/* Dada una ocurrencia {x} de la variable en estudio, {devuelve}
el indice de la celda del histograma en la que se debe integrar.
{devuelve} -1 si esta por debajo del histograma, y -2 si esta por
encima */

PUBLIC SPL_INT XAPI stat_histlocate( pSTAT st, SPL_FLOAT x )
{
	assert(st!=NULL);

	if (x<st->_min)
		return -1;
	else if (x>st->_max)
		return -2;
	else if (x==st->_max)
		return st->_divs-1;
	else if (st->_range)
		return (SPL_INT)(((x-st->_min)/(st->_range))*st->_divs);
	else
		return 0;
}

/**********************************************************/
/* dada una probabilidad {blkprob} (0-1), la funcion calcula el rango
{min},{max} en el que se asegura que la probabilidad acumulada
es la indicada en {blkprob} */

PUBLIC SPL_VOID XAPI stat_blocklimits( pSTAT st, SPL_FLOAT blkprob,
		SPL_FLOAT * min, SPL_FLOAT * max )
{
	SPL_LONG blk;
	SPL_INT i,j, maxj;
	SPL_LONG sum, maxsum;
	assert(st!=NULL);

	blk=(SPL_LONG)(blkprob*st->_n+0.5);
	maxj=0;

	for (i=0; i<st->_divs; i++) {
		maxsum=sum=stat_histdivacccount(st,i);
		for (j=1; j<st->_divs-i; j++) {
			sum += stat_histdivcount(st,j+i) -
					stat_histdivcount(st,j-1);
			if (sum>maxsum) {
				maxsum=sum;
				maxj=j;
			}
		}
		if (maxsum>=blk)
			break;
	}
	*min=stat_histdivmin(st,maxj);
	*max=stat_histdivmax(st,maxj+i);
}

/**********************************************************/

