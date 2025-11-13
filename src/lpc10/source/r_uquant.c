
#include "sr_lpc10.h"
#include <math.h>

/**********************************************************/

static INDEX G_detab7[32] = {
	4, 11, 18, 25, 32, 39, 46, 53, 60, 66, 72, 77, 82, 87, 92, 96, 101,
	104, 108, 111, 114, 115, 117, 119, 121, 122, 123, 124, 125, 126,
	127, 127
};

#define F FLOAT
static FLOAT G_descl[8] = {
	(F).6953, (F).6250, (F).5781, (F).5469, 
	(F).5312, (F).5391, (F).4688, (F).3828
};

static FLOAT G_deadd[8] = {
	(F)1152, (F)-2816, (F)-1536, (F)-3584, 
	(F)-1280, (F)-2432, (F)768, (F)-1920
};
#undef F

static INDEX G_qb[8] = {
	511, 511, 1023, 1023, 1023, 1023, 2047, 4095
};

static INDEX G_nbit[10] = {
	6, 6, 9, 9, 10, 10, 10, 10, 11, 12
};

/**********************************************************/

VOID unquant ( INDEX itab[13], BOOL voice[2],
		INT * pitch, FLOAT * rms, FLOAT rc[ORDER])
{
#define SCALERC (FLOAT)6.103515625e-05  /* 16,384 = 2**14 */
	INDEX i;
#ifdef NEGTRUNC
	FLOAT ftemp;
	INDEX itemp;
#endif

	/* pitch and voicing decision */
	*pitch = ITAB13_IPITCH;
	voice[1] = (ITAB13_IVOICE & 0x01);
	voice[0] = (ITAB13_IVOICE > 1);

	/* Decode RMS */
	*rms = (FLOAT)(G_rmst[(31 - ITAB13_IRMS) * 2]);

	/* Decode RC(1) and RC(2) from log-area-ratios */
	for (i = 0; i <= 1; i++) {
		INDEX i2 = ITAB13_IRC(i);
		if (i2 < 0)
			i2 = -G_detab7[2 * (-i2)];
		else
			i2 = G_detab7[2 * i2];
		rc[i] = (i2 * (2 << G_nbit[i])) * SCALERC;
	}

	/* Decode RC(3)-RC(10) to sign plus 14 bits */
	for (i = 2; i < ORDER; i++) {
		INDEX i2 = ITAB13_IRC(i);
		i2 *= (2 << G_nbit[i]);
		i2 += G_qb[i - 2];
#ifdef NEGTRUNC
		ftemp = i2 * G_descl[i-2] + G_deadd[i-2]; /* rounding problem */
		if (ftemp < 0) {
			ftemp = -ftemp;
			itemp = ftemp;
			rc[i] = (-itemp) * SCALERC;
		}
		else
			rc[i] = (INDEX)ftemp * SCALERC;
#else
		rc[i] = (INDEX)(i2 * G_descl[i-2] + G_deadd[i-2]) * SCALERC;
#endif
	}
}

/**********************************************************/
