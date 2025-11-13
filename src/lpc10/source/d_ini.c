#include "d_lpc10.h"

#include <string.h>

/**********************************************************/
/* $$$ optimizar, 10->order, etc */

VOID decoder_ini(VOID)
{
	INDEX i, j;

/* bsynz */
	for (i = 0; i < MAXPIT + ORDER; i++) {
		exc[i] = 0.0;
		exc2[i] = 0.0;
	}

/* synths */
	for (i=0; i<MAXPEP; i++) {
		ipiti[i] = 0;
		ivuv[i] = FALSE;
	}
	for (i = 0; i < ORDER; i++) {  // $$$ rmsi, 10 o 11???
		rmsi[i] = 0.0;
		pc[i] = 0.0;
	}
	for (i = 0; i < ORDER; i++)
		for (j = 0; j < MAXPEP; j++)
			rci[j][i] = 0.0;

}

/**********************************************************/
