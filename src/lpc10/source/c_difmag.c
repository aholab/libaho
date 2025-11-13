#include <math.h>
#include "c_lpc10.h"

/**********************************************************/
/* Compute Average Magnitude Difference Function
Inputs:
.   speech[] - Low pass filtered speech (0:MAXWIN+maxlag-1)
.	           where maximun maxlag=MAXPIT
.  tau - Table of lags
.  ltau - Number of lag values to compute
.  maxlag - Maximum possible lag value
Outputs:
.  amdf - Average Magnitude Difference for each lag in {tau}
.  minptr - Index of minimum AMDF value
.  maxptr - Index of maximum AMDF value
Constants:
.  MAXWIN - max window length */

VOID difmag( FLOAT speech[], INDEX tau[], INDEX ltau, INDEX maxlag,
		FLOAT amdf[], INDEX* minptr, INDEX* maxptr )
{
	INDEX i, j, n1, n2;
	FLOAT sum;

	*minptr = 0;
	*maxptr = 0;

	for (i = 0; i < ltau; i++) {
		n1 = (maxlag - tau[i]) >> 1;  /* IDIV 2 */
		n2 = n1 + MAXWIN;
		sum = (FLOAT)0.0;
		for (j = n1; j < n2; j += 4)
			sum += (FLOAT)fabs(speech[j] - speech[j + tau[i]]);

		amdf[i] = sum;
		if (amdf[i] < amdf[*minptr])
			*minptr = i;
		if (amdf[i] > amdf[*maxptr])
			*maxptr = i;
	}
}

/**********************************************************/
