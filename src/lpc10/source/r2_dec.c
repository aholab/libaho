#include "sr_lpc10.h"


/* $$$ esto esta repe */
static INDEX tau[64] = {
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,

	40, 42, 44, 46, 48,
	50, 52, 54, 56, 58,
	60, 62, 64, 66, 68,
	70, 72, 74, 76, 78,

	80, 84, 88, 92, 96,
	100, 104, 108, 112, 116,
	120, 124, 128, 132, 136,
	140, 144, 148, 152, 156
};

/* last RCs bit */
INDEX bit[10] = {
  16, 16, 16, 16, 8, 8, 8, 8, 4, 2
};



VOID decode_dgram ( INDEX itab[13], BOOL voice[2],
	INT * pitch, FLOAT * rms, FLOAT rc[ORDER])
{
	INDEX i;

	/* Sign extend RC's */
	for (i = 0; i < ORDER; i++)
		if ((ITAB13_IRC(i) & bit[i]) != 0)
			ITAB13_IRC(i) -= (bit[i] << 1);

	unquant (itab,voice,pitch,rms,rc);
	*pitch = tau[*pitch];
}

/**********************************************************/
