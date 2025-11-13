#include <math.h>
#include "d_lpc10.h"

/**********************************************************/
/* Convert Reflection Coefficients to Predictor Coeficients
Inputs:
- rc     - Reflection coefficients
- gprime - Excitation modification gain
Outputs:
- pc     - Predictor coefficients
- g2pass - Excitation modification sharpening factor
Constants:
- ORDER  - Number of RC's */

VOID irc2pc( FLOAT rc[], FLOAT pc[], FLOAT gprime, FLOAT *g2pass )
{
	INDEX i, j;
	FLOAT temp[ORDER], t;

	t = 1.;

	for (i = 0; i < ORDER; i++)
		t *= (1 - rc[i] * rc[i]);

	*g2pass = (FLOAT)(gprime * sqrt(t));
	pc[0] = rc[0];

	for (i = 1; i < ORDER; i++) {
		for (j = 0; j < i; j++)
			temp[j] = pc[j] - rc[i] * pc[i-j-1];

		for (j = 0; j < i; j++)
			pc[j] = temp[j];

		pc[i] = rc[i];
	}
}

/**********************************************************/
