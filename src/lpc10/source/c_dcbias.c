#include "c_lpc10.h"

/**********************************************************/

VOID dcbias( INDEX len, FLOAT speech[], FLOAT sigout[] )
{
	INDEX i;
	FLOAT bias;

	bias = (FLOAT)0.0;
	for (i = 0; i < len; i++)
		bias += speech[i];
	bias /= len;

	for (i = 0; i < len; i++)
		sigout[i] = speech[i] - bias;
}

/**********************************************************/
