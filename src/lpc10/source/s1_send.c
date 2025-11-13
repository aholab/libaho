#include "sr_lpc10.h"

/**********************************************************/

static INT G_enctab[16] = {
	0, 7, 11, 12, 13, 10, 6, 1, 14, 9, 5, 2, 3, 4, 8, 15
};

static INT G_entau[60] = {
	19, 11, 27, 25, 29, 21, 23, 22, 30, 14, 15, 7, 39,
	38, 46, 42, 43, 41, 45, 37, 53, 49, 51, 50, 54, 52,
	60, 56, 58, 26, 90, 88, 92, 84, 86, 82, 83, 81, 85,
	69, 77, 73, 75, 74, 78, 70, 71, 67, 99, 97, 113, 112,
	114, 98, 106, 104, 108, 100, 101, 76
};

static INDEX g_isync;

/**********************************************************/
/* Prepares and protects information to transmit using a
serial bitstream */

VOID encode_2400( INDEX itab[13] )
{
	if (ITAB13_IVOICE==3)
		ITAB13_IPITCH = G_entau[ITAB13_IPITCH];
	else if (!ITAB13_IVOICE)
		ITAB13_IPITCH = 0;
	else
		ITAB13_IPITCH = 127;

	/* Protect the most significant bits of the most
	important parameters during non-voiced frames.
	RC(1) - RC(4) are protected using 20 parity bits
	replacing RC(5) - RC(10). */

	if (!(ITAB13_IPITCH) || (ITAB13_IPITCH == 127)) {
		ITAB13_IRC(4) = G_enctab[(ITAB13_IRC(0) & 30) >> 1];
		ITAB13_IRC(5) = G_enctab[(ITAB13_IRC(1) & 30) >> 1];
		ITAB13_IRC(6) = G_enctab[(ITAB13_IRC(2) & 30) >> 1];
		ITAB13_IRC(7) = G_enctab[(ITAB13_IRMS & 30) >> 1];
		ITAB13_IRC(8) = G_enctab[(ITAB13_IRC(3) & 30) >> 1] >> 1;
		ITAB13_IRC(9) = G_enctab[(ITAB13_IRC(3) & 30) >> 1] & 1;
	}
}

/**********************************************************/
/* Place quantized parameters into bitstream

Input:
.  itab - Quantized parameters
Output:
.  ibits - Serial "bitstream"
Global:
.  sync - sync bit (toggles in each frame)


Formats of {itab} and {ibits} arrays:

itab[] = { ipitch, irms, irc[ORDER] }
ibits[] = {
	R1-0, R2-0, R3-0,  P-0,   A-0,
	R1-1, R2-1, R3-1,  P-1,   A-1,
	R1-2, R4-0, R3-2,  A-2,   P-2,  R4-1,
	R1-3, R2-2, R3-3,  R4-2,  A-3,
	R1-4, R2-3, R3-4,  R4-3,  A-4,
	P-3,  R2-4, R7-0,  R8-0,  P-4,  R4-4,
	R5-0, R6-0, R7-1,  R10-0, R8-1,
	R5-1, R6-1, R7-2,  R9-0,  P-5,
	R5-2, R6-2, R10-1, R8-2,  P-6,  R9-1,
	R5-3, R6-3, R7-3,  R9-2,  R8-3, SYNC
*/

VOID chan_write_2400_i( INDEX itab[12], INDEX ibits[54])
{
	INDEX i;

	/* Put 54 bits into IBITS array */
	for (i = 0; i < 53; i++) {
		ibits[i] = itab[G_iblist[i]] & 1;
		itab[G_iblist[i]] >>= 1;
	}
	ibits[53] = g_isync;
	g_isync ^= 1;
}

/**********************************************************/

VOID send_2400_ini( VOID )
{
	g_isync = 0;
}

/**********************************************************/

VOID send_2400_i( BOOL voice[2], INDEX pitch,
		FLOAT rms, FLOAT rc[ORDER], INDEX ibits[54] )
{
	INDEX itab[13];

	quant(voice, pitch, rms, rc, itab);

	encode_2400(itab);

	chan_write_2400_i(itab, ibits);
}

/**********************************************************/
