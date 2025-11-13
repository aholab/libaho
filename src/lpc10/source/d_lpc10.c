#include "d_lpc10.h"

/**********************************************************/

VOID d_lpc10_ini( VOID )
{
	decoder_ini();
}

/**********************************************************/

INT d_lpc10_decode( BOOL voice[2], INT pitch, FLOAT rms,
		FLOAT rc[ORDER], FLOAT speech[] )
{
	INDEX len;

	synths(voice, (INDEX)pitch, rms, rc, speech, &len);

	return (INT)len;
}

/**********************************************************/
