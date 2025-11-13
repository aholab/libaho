#include "sr_lpc10.h"

/**********************************************************/
/* extract quantized parameters from
safe buffer (datagram)

output:
.  itab - Quantized parameters
input:
.  buf - Safe datagram */

VOID chan_read_dgram( BYTE buf[7], INDEX itab[13] )
{
	ITAB13_IPITCH = buf[0] >> 2;
	ITAB13_IVOICE = buf[0] & 0x03;
	ITAB13_IRMS = buf[1] >> 3;
	ITAB13_IRC(8) = buf[1] & 0x7;
	ITAB13_IRC(0) = buf[2] & 0x1F;
	ITAB13_IRC(1) = buf[3] & 0x1F;
	ITAB13_IRC(3) = (buf[2] >> 5) | ((buf[3] >> 2) & 0x18);
	ITAB13_IRC(2) = buf[4] & 0x1F;
	ITAB13_IRC(9) = (buf[4] >> 5) & 0x3;
	ITAB13_IRC(4) = buf[5] & 0xF;
	ITAB13_IRC(5) = buf[5] >> 4;
	ITAB13_IRC(6) = buf[6] & 0xF;
	ITAB13_IRC(7) = buf[6] >> 4;
}

/**********************************************************/

VOID recv_dgram_ini( VOID )
{
}

/**********************************************************/

VOID recv_dgram( BYTE buf[7], BOOL voice[2], INDEX *pitch,
		FLOAT *rms, FLOAT rc[ORDER] )
{
	INDEX itab[13];
	chan_read_dgram(buf, itab);

	decode_dgram (itab, voice, pitch, rms, rc);
}

/**********************************************************/
