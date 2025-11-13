#include "c_lpc10.h"

/**********************************************************/
/* Preemphasize speech, using a couple of zeros at 0.5 +- 0.387i.
Previous version used ( 1 - .9375z**-1 ) [old preemphasis] cascaded
width ( 1 + .2z**-1 ) / ( 1 + .5z**-1 ) [6 db/oct ramp] */

VOID preemp( FLOAT inbuf[], FLOAT pebuf[], INDEX len, FLOAT z[2] )
{
	INDEX i;

	for (i = 0; i < len; i++) {
		pebuf[i] = inbuf[i] - z[0] + (FLOAT)0.4 * z[1];
		z[1] = z[0];
		z[0] = inbuf[i];
	}
}

/**********************************************************/
