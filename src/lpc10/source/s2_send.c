#include "sr_lpc10.h"

/**********************************************************/
/* Prepares and protects information to transmit using a
safe-channel datagram */

VOID encode_dgram( INDEX itab[13] )
{
	(void)itab;  /* avoid "unused parameter" warning */
}

/**********************************************************/
/* Place quantized parameters into safe buffer (datagram)

Input:
.  itab - Quantized parameters
Output:
.  buf - Safe datagram */

VOID chan_write_dgram( INDEX itab[13], BYTE buf[7] )
{
	buf[0] = (ITAB13_IPITCH << 2) | ITAB13_IVOICE;
	buf[1] = (ITAB13_IRMS << 3) | (ITAB13_IRC(8) & 0x07);
	buf[2] = (ITAB13_IRC(0) & 0x1F) | (ITAB13_IRC(3) << 5);
	buf[3] = (ITAB13_IRC(1) & 0x1F) | ((ITAB13_IRC(3) & 0x18) << 2);
	buf[4] = (ITAB13_IRC(2) & 0x1F) | (ITAB13_IRC(9) << 5);
	buf[5] = (ITAB13_IRC(4) & 0x0F) | (ITAB13_IRC(5) << 4);
	buf[6] = (ITAB13_IRC(6) & 0x0F) | (ITAB13_IRC(7) << 4);
}

/**********************************************************/

VOID send_dgram_ini( VOID )
{
}

/**********************************************************/

VOID send_dgram( BOOL voice[2], INDEX pitch,
		FLOAT rms, FLOAT rc[ORDER], BYTE buf[7] )
{
	INDEX itab[13];

	quant(voice, pitch, rms, rc, itab);
	encode_dgram(itab);

	chan_write_dgram(itab, buf);
}

/**********************************************************/
