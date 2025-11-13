#include <stdio.h>
#include <stdlib.h>

#include "lpc10.h"
#include "d_lpc10.h"
#include "sr_lpc10.h"

/**********************************************************/

PRIVATE BOOL g_used_d_dgram = FALSE;
PRIVATE FLOAT g_out[LPC10_DOUTMAX];
PRIVATE INT16 g_out_i16[LPC10_DOUTMAX];

/**********************************************************/

DLPC10_DGRAM *dlpc10_dgram_construct( VOID )
{
	if (g_used_d_dgram) {
		fprintf(stderr,"\nPor ahora no soporto mas de una estructura DLPC10_DGRAM!\7\n");
		exit(1);
	}

	g_used_d_dgram = TRUE;

	dlpc10_dgram_reset((DLPC10_DGRAM*)1);
	return (DLPC10_DGRAM*)1;
}

/**********************************************************/

VOID dlpc10_dgram_destruct( DLPC10_DGRAM *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	g_used_d_dgram = FALSE;
}

/**********************************************************/

VOID dlpc10_dgram_reset( DLPC10_DGRAM *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	decoder_ini();
	recv_dgram_ini();
}

/**********************************************************/

FLOAT *dlpc10_dgram_outvec( DLPC10_DGRAM *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	return g_out;
}

/**********************************************************/

INT16 *dlpc10_dgram_outvec_i16( DLPC10_DGRAM *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	return g_out_i16;
}

/**********************************************************/

INT dlpc10_dgram_decode( DLPC10_DGRAM *d, LPC10_dgram_data *dgram )
{
	INDEX len;
	INT pitch;
	FLOAT rms;
	FLOAT rc[LPC10_ORDER];
	BOOL voice[2];

	(void)d;  /* avoid "unused parameter" warning */
	recv_dgram(*dgram,voice,&pitch,&rms,rc);
	synths(voice, pitch, rms, rc, g_out, &len);

	return len;
}

/**********************************************************/

INT dlpc10_dgram_decode_i16( DLPC10_DGRAM *d, LPC10_dgram_data *dgram )
{
	INDEX len;
	INT i;

	len = dlpc10_dgram_decode(d,dgram);

	for (i=0; i<len; i++)
		g_out_i16[i] = (INT16)(MMAX(-32768., MMIN(32768. * g_out[i], 32767.)));

	return len;
}

/**********************************************************/
