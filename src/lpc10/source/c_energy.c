#include <math.h>
#include "c_lpc10.h"

/**********************************************************/

FLOAT energy_sqrt( INDEX len, FLOAT speech[] )
{
	INDEX i;
	FLOAT rms = (FLOAT)0.0;

	for (i = 0; i < len; i++)
		rms += speech[i] * speech[i];

	return (FLOAT)sqrt(rms / len);
}

/**********************************************************/
