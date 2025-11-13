#include "c_lpc10.h"

/**********************************************************/

VOID c_lpc10_ini( VOID )
{
	coder_ini();
}

/**********************************************************/

VOID c_lpc10_code( FLOAT speech[] )
{
/* shift sample and other buffers */
	shift();
/* prepare and copy input buffer */
	framein(speech);
/* analyze frame */
	analys();
}

/**********************************************************/

VOID c_lpc10_code_i16( INT16 speech[] )
{
/* shift sample and other buffers */
	shift();
/* prepare and copy input buffer */
	framein_i16(speech);
/* analyze frame */
	analys();
}

/**********************************************************/

VOID c_lpc10_peek( BOOL voice[2], INT *pitch,
		FLOAT *rms, FLOAT rc[ORDER] )
{
	INDEX i;

	/* Set return parameters */
	voice[0] = CODED_VOICE[0];
	voice[1] = CODED_VOICE[1];

	if (CODED_VOICE[0]||CODED_VOICE[1])
		*pitch=(INT)(G_tau[CODED_PITCH]);
	else
		*pitch = 0;
	*rms = CODED_RMS;
	for (i = 0; i < ORDER; i++)
		rc[i] = CODED_RC[i];
}

/**********************************************************/

