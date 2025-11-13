#include "sr_lpc10.h"

// $$$ estudiar NEG_TRUNC

/**********************************************************/
/* quantize parameters (actually, only rms and RCs are quantized):

pitch: 6 bits
rms: 5 bits
RC(1), RC(2), RC(3), RC(4): 5 bits
RC(5), RC(6), RC(7), RC(8): 4 bits
RC(9): 3 bits
RC(10): 2 bits
voice: 2 bits (one per half-frame)

Results are written in {itab} following the previous order
*/

/**********************************************************/

static INDEX G_enadd[10] = {
	0, 0, -1152, 2816, 1536, 3584, 1280, 2432, -768, 1920
};

#define F FLOAT
static FLOAT G_enscl[10] = {
	(F)0, (F)0, (F).0112, (F).0125, (F).0135, 
	(F).0143, (F).0147, (F).0145, (F).0167, (F).0204
};
#undef F

static INDEX G_enbits[10] = {
	0, 0, 2, 2, 3, 3, 3, 3, 4, 5
};

static INDEX G_entab6[64] = {
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8,
	9, 9, 9, 10, 10, 11, 11, 12, 13, 14, 15
};

/**********************************************************/

VOID quant( BOOL voice[2], INDEX pitch, FLOAT rms, FLOAT rc[ORDER],
		INDEX itab[13] )
{
	INDEX i2, i, j, idel;
#ifdef NEG_TRUNC
	FLOAT ftemp;
#endif

	/* voicing */
	ITAB13_IVOICE = (voice[1]) ? 1 : 0;
	if (voice[0])
		ITAB13_IVOICE += 2;

	/* pitch */
	ITAB13_IPITCH = pitch;  /* 0 - 59 */  /* $$$ no es 0-63??? */

	/* Encode RMS by binary table search */
	ITAB13_IRMS = (INDEX)rms;  /* scale to int */  /* $$$ cast a int */
	j = 31;
	idel = 16;
	if (ITAB13_IRMS>1023)
		ITAB13_IRMS = 1023;
	while (idel > 0) {
		if (ITAB13_IRMS > G_rmst[j])
			j -= idel;
		else if (ITAB13_IRMS < G_rmst[j])
			j += idel;
		idel >>= 1;
	}
	if (ITAB13_IRMS > G_rmst[j])
		j--;
	ITAB13_IRMS = (j>60) ? 0 : 30 - (j >> 1);  /* q. to 5 bits */

	/* prepare RCs */
	for (i = 0; i < ORDER; i++)
		ITAB13_IRC(i) = (INDEX)(rc[i] * 32768L);  /* mirar bien esto */  /* $$$ cast to int */
	/* Encode RC(1) and (2) as log-area-ratios */
	for (i = 0; i <2; i++) {
		if (ITAB13_IRC(i)<0) {
			i2 = (-ITAB13_IRC(i)) >> 9;
			ITAB13_IRC(i) = - ((i2>63) ? G_entab6[63] : G_entab6[i2]);
		}
		else {
			i2 = ITAB13_IRC(i) >> 9;
			ITAB13_IRC(i) = (i2>63) ? G_entab6[63] : G_entab6[i2];
		}
	}

	/* Encode RC(3) - (10) linearly, remove bias then scale */
	for (i = 2; i < ORDER; i++) {
#ifdef NEG_TRUNC
		ftemp = ((ITAB13_IRC(i) >> 1) + G_enadd[i]) * G_enscl[i];
		i2 = (INDEX)((ftemp<0) ? -((INDEX)(-ftemp)) : ftemp);
#else
		i2 = (INDEX)(((ITAB13_IRC(i) >> 1) + G_enadd[i]) * G_enscl[i]);
#endif
		if (i2<0)
			ITAB13_IRC(i) = (((i2 < -127) ? -127 : i2) / (2 << G_enbits[i]))-1;
		else
			ITAB13_IRC(i) = ((i2 > 127) ? 127 : i2) / (2 << G_enbits[i]);
	}
}

/**********************************************************/
