#include <stdio.h>
#include <stdlib.h>

#include "lpc10.h"
#include "d_lpc10.h"
#include "sr_lpc10.h"

/**********************************************************/

PRIVATE BOOL g_used_d_2400 = FALSE;
PRIVATE FLOAT g_out[LPC10_DOUTMAX];
PRIVATE INT16 g_out_i16[LPC10_DOUTMAX];

/**********************************************************/

DLPC10_2400 *dlpc10_2400_construct( VOID )
{
	if (g_used_d_2400) {
		fprintf(stderr,"\nPor ahora no soporto mas de una estructura DLPC10_2400!\7\n");
		exit(1);
	}

	g_used_d_2400 = TRUE;

	dlpc10_2400_reset((DLPC10_2400*)1);
	return (DLPC10_2400*)1;
}

/**********************************************************/

VOID dlpc10_2400_destruct( DLPC10_2400 *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	g_used_d_2400 = FALSE;
}

/**********************************************************/

VOID dlpc10_2400_reset( DLPC10_2400 *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	decoder_ini();
	recv_2400_ini();
}

/**********************************************************/

FLOAT *dlpc10_2400_outvec( DLPC10_2400 *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	return g_out;
}

/**********************************************************/

INT16 *dlpc10_2400_outvec_i16( DLPC10_2400 *d )
{
	(void)d;  /* avoid "unused parameter" warning */
	return g_out_i16;
}

/**********************************************************/

INT dlpc10_2400_idecode( DLPC10_2400 *d, LPC10_2400_idata *bits )
{
	INDEX len;
	INT pitch;
	FLOAT rms;
	FLOAT rc[LPC10_ORDER];
	BOOL voice[2];

	(void)d;  /* avoid "unused parameter" warning */
	recv_2400_i(*bits,voice,&pitch,&rms,rc);
	synths(voice, pitch, rms, rc, g_out, &len);

	return len;
}

/**********************************************************/

INT dlpc10_2400_idecode_i16( DLPC10_2400 *d, LPC10_2400_idata *bits )
{
	INDEX len;
	INT i;

	len = dlpc10_2400_idecode(d,bits);

	for (i=0; i<len; i++)
		g_out_i16[i] = (INT16)(MMAX(-32768., MMIN(32768. * g_out[i], 32767.)));

	return len;
}

/**********************************************************/

VOID dlpc10_2400_unpack( LPC10_2400_data *in, LPC10_2400_idata *o)
{
	INDEX i,j,n;

	for (i=0; i<7; i++) {
		for (j=0; j<8; j++) {
			n=i*8+j;
			if (n>=(INDEX)sizeof(LPC10_2400_idata)) break;
			(*o)[n] = ((*in)[i] & (1<<(7-j)))?1:0;
		}
	}
}

/**********************************************************/
