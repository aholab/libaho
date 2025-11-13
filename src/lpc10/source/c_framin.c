#include "c_lpc10.h"

/**********************************************************/
/* Define RELIABLE_AD if adequate high pass filtering and
reliable A/D conversion is available, so audio has no DC bias.
As initially we high pass filter audio buffer (hp100()), we
can define this safelly */

#ifndef ORIG
#define RELIABLE_AD
#endif

/**********************************************************/

VOID framein( FLOAT speech[] )
{
	INDEX i;
#ifndef RELIABLE_AD
	FLOAT temp;
#endif

	hp100(speech,g_inbuf+(SBUFH-LFRAME+1));

/* Copy input speech, scale to sign+12 bit integers. */
#ifdef RELIABLE_AD
	for (i = 0; i < LFRAME; i++)
		g_inbuf[(SBUFH-LFRAME+1) + i] *= (FLOAT)4096.0;
#else
/* Remove long term DC bias. (This code can be disabled if adequate high
pass filtering and reliable A/D conversion is available) */
	temp = (FLOAT)0.0;
	for (i = 0; i < LFRAME; i++)
		temp += (g_inbuf[(SBUFH-LFRAME+1) + i] =
				g_inbuf[(SBUFH-LFRAME+1) + i] * (FLOAT)4096.0 - g_bias);
	if (temp > LFRAME)
		g_bias++;
	else if (temp < -LFRAME)
		g_bias--;
#endif

}

/**********************************************************/

VOID framein_i16( INT16 speech[] )
{
#ifndef RELIABLE_AD
	FLOAT temp;
	INDEX i;
#endif

	hp100_i16(speech,g_inbuf+(SBUFH-LFRAME+1));

#ifndef RELIABLE_AD
/* Remove long term DC bias. (This code can be disabled if adequate high
pass filtering and reliable A/D conversion is available) */
	temp = (FLOAT)0.0;
	for (i = 0; i < LFRAME; i++)
		temp += (g_inbuf[(SBUFH-LFRAME+1) + i] -= g_bias);
	if (temp > LFRAME)
		g_bias++;
	else if (temp < -LFRAME)
		g_bias--;
#endif
}

/**********************************************************/

