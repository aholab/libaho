#include "c_lpc10.h"

/**********************************************************/

#define xDISABLE_HP100

#ifndef ORIG
#define FCAST(n)  (FLOAT)(n)
#else
#define FCAST(n)  (n)
#endif

/**********************************************************/
/* 100 Hz High Pass Filter.

Two cells cascaded.

First one:  double zero at 1+0i, pole at 0.93+-0.028i
Second one: double zero at 1+0i, pole at 0.968+-0.07i  */

VOID hp100( FLOAT speech[], FLOAT dest[] )
{
	INDEX i;
	FLOAT si, err;

	for (i = 0; i < LFRAME; i++) {
		si = speech[i];

		err = (FLOAT)(si + FCAST(1.859076) * g_hpa1 - FCAST(0.8648249) * g_hpa2);
		si = (FLOAT)(err - FCAST(2.0) * g_hpa1 + g_hpa2);
		g_hpa2 = g_hpa1;
		g_hpa1 = err;

		err = (FLOAT)(si + FCAST(1.935715) * g_hpb1 - FCAST(0.9417004) * g_hpb2);
		si = (FLOAT)(err - FCAST(2.0) * g_hpb1 + g_hpb2);
		g_hpb2 = g_hpb1;
		g_hpb1 = err;

#ifdef DISABLE_HP100
		dest[i] = speech[i];
#else
		dest[i] = (FLOAT)(FCAST(0.902428) * si);
#endif
	}
}

/**********************************************************/
/* 16 bit input version.
Output is scaled to 12 bits+sign (input*4096/32768) */

VOID hp100_i16( INT16 speech[], FLOAT dest[] )
{
	INDEX i;
	FLOAT si, err;

	for (i = 0; i < LFRAME; i++) {
		si = ((FLOAT)speech[i]/(FLOAT)8.0); /* scale to 12 bit+sign */

		err = (FLOAT)(si + FCAST(1.859076) * g_hpa1 - FCAST(0.8648249) * g_hpa2);
		si = (FLOAT)(err - FCAST(2.0) * g_hpa1 + g_hpa2);
		g_hpa2 = g_hpa1;
		g_hpa1 = err;

		err = (FLOAT)(si + FCAST(1.935715) * g_hpb1 - FCAST(0.9417004) * g_hpb2);
		si = (FLOAT)(err - FCAST(2.0) * g_hpb1 + g_hpb2);
		g_hpb2 = g_hpb1;
		g_hpb1 = err;

#ifdef DISABLE_HP100
		dest[i] = ((FLOAT)speech[i]/(FLOAT)8.0);
#else
		dest[i] = (FLOAT)(FCAST(0.902428) * si);
#endif
	}
}

/**********************************************************/
