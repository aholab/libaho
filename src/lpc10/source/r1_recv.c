#include "sr_lpc10.h"

/**********************************************************/

INT g_drc[3][ORDER], g_dpit[3], g_drms[3];

/**********************************************************/
/* extract quantized parameters from bitstream

output:
.  itab - Quantized parameters
input:
.  ibits - Serial "bitstream"
*/

VOID chan_read_2400_i( INDEX ibits[54], INDEX itab[12] )
{
	INDEX i;

	for (i = 0; i < 12; i++)
		itab[i] = 0;

	for (i = 53; (i--)>0; )
		itab[G_iblist[i]] = (itab[G_iblist[i]] << 1) + ibits[i];

}

/**********************************************************/

VOID recv_2400_ini( VOID )
{
	INDEX i, j;

	for (i=0; i<3; i++) {
		g_dpit[i] = g_drms[i] = 0;
		for (j=0; j<ORDER; j++)
			g_drc[i][j] = 0;
	}
}

/**********************************************************/

VOID recv_2400_i( INDEX ibits[54], BOOL voice[2], INDEX *pitch,
		FLOAT *rms, FLOAT rc[ORDER] )
{
	INDEX itab[13];

	chan_read_2400_i(ibits, itab);

	decode_2400 (itab, voice, pitch, rms, rc);
}

/**********************************************************/
