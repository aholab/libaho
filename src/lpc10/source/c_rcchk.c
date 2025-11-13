#include <math.h>
#include "c_lpc10.h"

/**********************************************************/
/*  Check RC's, repeat previous frame's RC's if unstable
Inputs:
.  rc1f - Previous frame's RC's
In/Outputs:
.  rc2f  - Present frame's RC's
Constants:
.  ORDER - Analysis order */

VOID rcchk( FLOAT rc1f[ORDER], FLOAT rc2f[ORDER] )
{
	INDEX i;

	for (i = 0; i < ORDER; i++) {
		if ((FLOAT)fabs(rc2f[i]) > .99) {
			for (i = 0; i < ORDER; i++)
				rc2f[i] = rc1f[i];
			return;
		}
	}
}

/**********************************************************/

