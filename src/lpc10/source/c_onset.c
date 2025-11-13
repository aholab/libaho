#include <math.h>
#include "c_lpc10.h"

/**********************************************************/
/* Onset detection.
Detection of onsets in (or slightly preceding) the futuremost frame
of speech.

Inputs
.  pebuf[] - Preemphasized speech (SBUFH+1 samples, last LFRAME used)
In/Outputs
.  osbuf[OSLEN] - Buffer which holds sorted indexes of onsets
.  osptr - Free pointer into OSBUF
Constants:
.  L2LAG - Lag due to both filters which compute filtered slope of FPC
.  L2WID - Width of the filter which computes the slope of FPC
.  OSHYST - The number of samples which of slope (FPC) which must be below
.           the threshold before a new onset may be declared.
.  L2 - Threshold for filtered slope of FPC (function of L2WID!)
Variables
.  n, d - Numerator and denominator of prediction filters
.  fpc - Current prediction coefs
.  l2buf, l2sum1, l2sum2 - State of slope filter
Globals
.  FLOAT n, d, fpc
.  FLOAT l2buf[L2WID], l2sum1
.  INDEX l2ptr1, l2ptr2, lasti
.  BOOL hyst  */

VOID onset( FLOAT pebuf[], INDEX osbuf[OSLEN], INDEX* osptr )
{
#define L2LAG (L2WID/2+1) /* 16/2+1=9 */
#define OSHYST 10
#define L2 ((FLOAT)1.7)
	INDEX i;
	FLOAT l2sum2;

	if (g_hyst)
		g_lasti -= LFRAME;

	for (i = (SBUFH-LFRAME+1); i <= SBUFH; i++) {

/* Compute FPC; Use old FPC on divide by zero;
Clamp FPC to +/-1;  0.015625=1/64 */
		g_n = (pebuf[i]*pebuf[i-1] + (FLOAT)63.0*g_n) * (FLOAT)0.015625;
		g_d = (pebuf[i-1]*pebuf[i-1] + (FLOAT)63.0*g_d) * (FLOAT)0.015625;
		if (g_d) {
			if (g_n > g_d)
				g_fpc = (FLOAT)1.0;
			else if (g_n<-g_d)
				g_fpc = (FLOAT)-1.0;
			else
				g_fpc = g_n / g_d;
		}

/*   Filter FPC */
		l2sum2 = g_l2buf[g_l2ptr1];
		g_l2sum1 += g_fpc - g_l2buf[g_l2ptr2];
		g_l2buf[g_l2ptr2] = g_l2sum1;
		g_l2buf[g_l2ptr1] = g_fpc;
		g_l2ptr1++;
		if (g_l2ptr1==L2WID)
			g_l2ptr1=0;
		g_l2ptr2++;
		if (g_l2ptr2==L2WID)
			g_l2ptr2=0;

		if ((FLOAT)fabs((g_l2sum1 - l2sum2)) > L2) {
			if (!g_hyst) {
				if (*osptr < OSLEN) {  /* Ignore if buffer full */
					osbuf[*osptr] = i - L2LAG;
					(*osptr)++;
				}
				g_hyst = TRUE;
			}
			g_lasti = i;
		}
		else if (g_hyst && (i - g_lasti) >= OSHYST)
			g_hyst = FALSE;
	}
}

/**********************************************************/
