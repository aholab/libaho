#include <math.h>
#include "d_lpc10.h"

/**********************************************************/
/* Pseudo random number generator based on Knuth, Vol 2, p. 27.
Returns INT16 variable, uniformly distributed over -32768 to 32767 */

#define MIDTAP 1
#define MAXTAP 4

INT16 Rrandom( VOID )
{
	INT16 the_random;
	static INT16 y[MAXTAP + 1] =
		{-21161, -8478, 30892, -10216, 16950};
	static INDEX j = MIDTAP;
	static INDEX k = MAXTAP;

/* The following is a 16 bit 2's complement addition,
   with overflow checking disabled */

	y[k] += y[j];
/*
	if (y[k] > 32767)
		y[k] = -(32768 - (y[k] & 32767));
	if (y[k] < -32768)
		y[k] = y[k] & 32767;
*/

	the_random = y[k];

	if (!k)
		k = MAXTAP;
	else
		k--;

	if (!j)
		j = MAXTAP;
	else
		j--;

	return the_random;
}

/**********************************************************/
