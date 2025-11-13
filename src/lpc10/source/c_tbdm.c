#include "c_lpc10.h"

/**********************************************************/
/* TURBO DIFMAG: Compute High Resolution Average Magnitude
Difference Function

Inputs:
.   speech - Low pass filtered speech (0:PWLEN-1
			 samples)(PWLEN=MAXWIN+MAXPIT)
.   tau[LTAU] - Table of lags
Outputs:
.  amdf[LTAU] - Average Magnitude Difference for each lag in {tau}
.  minptr - Index of minimum AMDF value
.  maxptr - Index of maximum AMDF value within +/- 1/2 octave of min
.  mintau - Lag corresponding to minimum AMDF value
Constants
.   LTAU - Number of lags in {tau}; Number of lag values to compute
*/

VOID tbdm( FLOAT speech[], INDEX tau[LTAU], FLOAT amdf[LTAU],
		INDEX* minptr, INDEX* maxptr, INDEX* mintau )
{
	INDEX minp2, maxp2, i, ptr, ltau2, last;
#ifdef ORIG
	int minamd; /* This should be FLOAT */
#else
	FLOAT minamd;
#endif
	FLOAT amdf2[6];
	INDEX tau2[6];

/* Compute full AMDF using log spaced lags, find coarse minimum */
	difmag(speech, tau, LTAU, tau[LTAU-1], amdf, minptr, maxptr);

	*mintau = tau[*minptr];
#ifdef ORIG
	minamd = (int)amdf[*minptr];  /* $$$ cast to avoid warning ifdef ORIG */
#else
	minamd = amdf[*minptr];
#endif
/* Build table containing all lags within +/- 3 of the AMDF minimum
excluding all that have already been computed  */
	ltau2 = 0;
	ptr = (*minptr) - 2;
	last = (*mintau) + 3;
	if (last>tau[LTAU-1])
		last = tau[LTAU-1];

	for (i = (((*mintau)>44) ? (*mintau)-3 : 41); i <= last; i++) {
		while (tau[ptr] < i)
			ptr++;
		if (tau[ptr] != i) {
			tau2[ltau2] = i;
			ltau2++;
		}
	}
/* Compute AMDF of the new lags, if there are any, and choose one
if it is better than the coarse minimum    */
	if (ltau2) {
		difmag(speech, tau2, ltau2, tau[LTAU-1], amdf2, &minp2, &maxp2);
		if (amdf2[minp2] < minamd) {
			*mintau = tau2[minp2];
#ifdef ORIG
			minamd = (int)amdf2[minp2]; /* $$$ cast to avoid warning ifdef ORIG */
#else
			minamd = amdf2[minp2];
#endif

		}
	}

/* Check one octave up, if there are any lags not yet computed */
	if (*mintau >= 80) {
		i = (*mintau) >> 1;	 /* gets (*mintau)/2 */
		if ((i & 1)!=0) {  /* if odd */
			ltau2 = 1;
			tau2[0] = i;
		}
		else {  /* if even */
			ltau2 = 2;
			tau2[0] = i - 1;
			tau2[1] = i + 1;
		}
		difmag(speech, tau2, ltau2, tau[LTAU-1], amdf2, &minp2, &maxp2);
		if (amdf2[minp2] < minamd) {
			*mintau = tau2[minp2];
#ifdef ORIG
			minamd = (int)amdf2[minp2]; /* $$$ cast to avoid warning ifdef ORIG */
#else
			minamd = amdf2[minp2];
#endif
			*(minptr) -= 20;
		}
	}

/* Force minimum of the AMDF array to the high resolution minimum */
#ifdef ORIG
	amdf[*minptr] = (FLOAT)minamd;
#else
	amdf[*minptr] = minamd;
#endif

/* Find maximum of AMDF within 1/2 octave of minimum */
	*maxptr = (*minptr>5) ?  (*minptr)-5 : 0;
	last = ((*minptr)<(LTAU-6)) ? (*minptr)+6 : LTAU;
	for (i = (*maxptr)+1; i < last; i++) {
		if (amdf[i] > amdf[*maxptr])
			*maxptr = i;
	}
}

/**********************************************************/
