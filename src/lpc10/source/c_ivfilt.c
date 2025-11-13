#include "c_lpc10.h"

/**********************************************************/
/* 2nd order inverse filter, speech is decimated 4:1
Inputs:
.  lpbuf[] - Low pass filtered speech buffer (0:PWLEN-1 samples)
Output:
.  ivbuf[] - Inverse filtered speech buffer (0:PWLEN-1 samples,
.            last LFRAME written)
.  ivrc[2] - Inverse filter reflection coefficients (for voicing).
Constants:
.  LFRAME - speech frame length
.  PWLEN - pitch window length */

VOID ivfilt( FLOAT lpbuf[], FLOAT ivbuf[], FLOAT ivrc[2] )
{
	INDEX i, j, k;
	FLOAT r[3], pc1, pc2, tmp;

/* Calculate Autocorrelations */
	for (i = 0; i <= 2; i++) {
		k = i * 4;
		r[i] = (FLOAT)0.0;
		for (j = i * 4 + (PWLEN - LFRAME + 3); j < PWLEN; j += 2)
			r[i] += lpbuf[j] * lpbuf[j - k];
	}

/* Calculate predictor coefficients */
	pc1 = pc2 = ivrc[0] = ivrc[1] = (FLOAT)0.0;
	if (r[0] > (FLOAT)1e-6) {
		ivrc[0] = r[1] / r[0];
		tmp = ivrc[0] * r[1];
		pc2 = ivrc[1] = (r[2] - tmp) / (r[0] - tmp);
		pc1 = ivrc[0] - ivrc[0] * ivrc[1];
	}

/* Inverse filter {lpbuf} into {ivbuf} */
	for (i = PWLEN - LFRAME; i < PWLEN; i++)
		ivbuf[i] = lpbuf[i] - pc1 * lpbuf[i-4] - pc2 * lpbuf[i-8];
}

/**********************************************************/
