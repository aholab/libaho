#ifndef __STAT_H
#define __STAT_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ STAT.H
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.1    30/07/95  Borja     TIPO SPL_PTR --> pTIPO
1.1.0    08/12/94  Borja     revision de tipos.
1.0.0    11/01/93  Borja     Codificacion inicial.

======================== Contenido ========================
Fichero de cabecera de STAT.C

Calculo de estadisticos de primer y segundo orden.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spltdef.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

typedef struct _STAT {
	SPL_LONG _n;
	SPL_FLOAT _mean, _msqv, _var, _min, _max, _range;
	SPL_pLONG _hist;
	SPL_LONG _under, _over;
	SPL_INT _divs, _sec;
} STAT, SPL_PTR pSTAT;

/**********************************************************/

pSTAT API stat_construct( SPL_INT nhistdivs );
SPL_VOID API stat_destruct( pSTAT st );

SPL_VOID API stat_first( pSTAT st );
SPL_VOID API stat_second( pSTAT st );
SPL_VOID API stat_setvalue( pSTAT st, SPL_FLOAT x );
SPL_VOID API stat_setminmax( pSTAT st,
		SPL_FLOAT min, SPL_FLOAT max );

SPL_LONG API stat_n( pSTAT st );
SPL_FLOAT API stat_mean( pSTAT st );
SPL_FLOAT API stat_msqv( pSTAT st );
SPL_FLOAT API stat_min( pSTAT st );
SPL_FLOAT API stat_max( pSTAT st );

SPL_FLOAT API stat_variance( pSTAT st );
SPL_FLOAT API stat_stddev( pSTAT st );

SPL_INT API stat_nhistdivs( pSTAT st );
SPL_pLONG API stat_hists( pSTAT st );
SPL_LONG API stat_histunder( pSTAT st );
SPL_LONG API stat_histover( pSTAT st );
SPL_FLOAT API stat_histdivwidth( pSTAT st );
SPL_LONG API stat_histdivcount( pSTAT st, SPL_INT div );
SPL_LONG API stat_histdivacccount( pSTAT st, SPL_INT div );
SPL_FLOAT API stat_histdivprob( pSTAT st, SPL_INT div );
SPL_FLOAT API stat_histdivaccprob( pSTAT st, SPL_INT div );
SPL_FLOAT API stat_histdivmin( pSTAT st, SPL_INT div );
SPL_FLOAT API stat_histdivmax( pSTAT st, SPL_INT div );
SPL_INT API stat_histlocate( pSTAT st, SPL_FLOAT x );
SPL_VOID API stat_blocklimits( pSTAT st, SPL_FLOAT blkprob,
		SPL_FLOAT * min, SPL_FLOAT * max );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif

