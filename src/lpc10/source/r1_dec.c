/******************************************************************
*
*	DECODE Version 52
*
******************************************************************
*
*   This subroutine provides error correction and decoding
*   for all LPC parameters
*
* INPUTS:
*  ORDER  - Number of RC's
*  IPITV  - Index value of pitch
*  IRMS   - Coded Energy
*  IRC    - Coded Reflection Coefficients
*  CORRP  - Error correction:
*    If FALSE, parameters are decoded directly with no delay.  If TRUE,
*    most important parameter bits are protected by Hamming code and
*    median smoothed.  This requires an additional frame of delay.
* OUTPUTS:
*  VOICE  - Half frame voicing decisions
*  PITCH  - Decoded pitch
*  RMS    - Energy
*  RC     - Reflection coefficients
*
*  NOTE: Zero RC's should be done more directly, but this would affect
*   coded parameter printout.
*/

/* $$$ he cambiado los abs() por fabs() e incluido un cast a FLOAT explicito */

#include "sr_lpc10.h"
#include <math.h>


/* last RCs bit */
static INDEX bit[10] = {
  16, 16, 16, 16, 8, 8, 8, 8, 4, 2
};


INT ivtab[32] =
{
  24960, 24960, 24960, 24960, 25480, 25480, 25483, 25480,
  16640, 1560, 1560, 1560, 16640, 1816, 1563,
  1560, 24960, 24960, 24859, 24856, 26001,
  25881, 25915, 25913, 1560, 1560, 7800,
  3640, 1561, 1561, 3643, 3641
};
#define F FLOAT
FLOAT corth[4][8] =
{
  {(F)32767., (F)32767., (F)32., (F)32., (F)32., (F)32., (F)16., (F)16.},
  {(F)10., (F)8., (F)6.4, (F)6.4, (F)11.2, (F)11.2, (F)5.6, (F)5.6},
  {(F)5., (F)4., (F)3.2, (F)3.2, (F)6.4, (F)6.4, (F)3.2, (F)3.2},
  {(F)0., (F)0., (F)0., (F)0., (F)0., (F)0., (F)0., (F)0.}
};
#undef F
INT detau[128] =
{
  0, 0, 0, 3, 0, 3, 3, 31, 0, 3, 3, 21, 3, 3, 29, 30,
  0, 3, 3, 20, 3, 25, 27, 26, 3, 23, 58, 22, 3, 24, 28, 3,
  0, 3, 3, 3, 3, 39, 33, 32, 3, 37, 35, 36, 3, 38, 34, 3,
  3, 42, 46, 44, 50, 40, 48, 3, 54, 3, 56, 3, 52, 3, 3, 1,
  0, 3, 3, 108, 3, 78, 100, 104, 3, 84, 92, 88, 156, 80, 96, 3,
  3, 74, 70, 72, 66, 76, 68, 3, 62, 3, 60, 3, 64, 3, 3, 1,
  3, 116, 132, 112, 148, 152, 3, 3, 140, 3, 136, 3, 144, 3, 3, 1,
  124, 120, 128, 3, 3, 3, 3, 1, 3, 3, 3, 1, 3, 1, 1, 1
};
INT zrc[ORDER] =
{0, 0, 0, 0, 0, 3, 0, 2, 0, 0};
INT abit[5] =
{2, 4, 8, 16, 32};


void decode_2400 ( INDEX itab[13], BOOL voice[2],
	INT * pitch, FLOAT * rms, FLOAT rc[ORDER])
{
  INT ivoic;
  static INT ivp2h = 0, erate = 0, iovoic = 0;
  INT i, i1, i2, i4, iavgp = 60, icorf, index, iout;
  INT ipit, ixcor, lsb;
  INT errcnt;
  INT ethrs = 2048, ethrs1 = 128, ethrs2 = 1024, ethrs3 = 2048;
  INT fut = 0, pres = 1, past = 2;
  static short first = 1;


	/* Sign extend RC's */
	for (i = 0; i < ORDER; i++)
		if ((ITAB13_IRC(i) & bit[i]) != 0)
			ITAB13_IRC(i) -= (bit[i] << 1);

	/* If no error correction, do pitch and voicing then jump to decode */
	i4 = detau[ITAB13_IPITCH];

	/* Do error correction pitch and voicing */
	if (i4 > 4) {
	  g_dpit[fut] = i4;
	  ivoic = 2;
		iavgp = (INT)((15 * iavgp + i4 + 8) * 0.0625);  /* $$$ cast a int */
	}
	else {
		ivoic = i4;
		g_dpit[fut] = iavgp;
	}
	g_drms[fut] = ITAB13_IRMS;

	for (i = 0; i < ORDER; i++)
		g_drc[fut][i] = ITAB13_IRC(i);

	/* Determine index to IVTAB from V/UV decision
	If error rate is high then use alternate table */

	index = 16 * ivp2h + 4 * iovoic + ivoic + 1;
	i1 = ivtab[index - 1];
	ipit = i1 & 3;
	icorf = (INT)(i1 * 0.125); /* $$$ cast a int */
	if (erate < ethrs)
		icorf = (INT)(icorf * 0.015625); /* $$$ cast a int */

	/* Determine error rate:  4=high    1=low */
	ixcor = 4;
	if (erate < ethrs3)
		ixcor = 3;
	if (erate < ethrs2)
		ixcor = 2;
	if (erate < ethrs1)
		ixcor = 1;

	/* Voice/unvoice decision determined from bits 0 and 1 of IVTAB.
	voice[0] = bit1  voice[1]=bit0 */
	ITAB13_IVOICE = icorf & 3;

	/* Skip decoding on first frame because present data not yet available */

	if (first)
		first = 0;
	else {

/*  If bit 4 of ICORF is set then correct RMS and RC(1) - RC(4).
   *    Determine error rate and correct errors using a Hamming 8,4 code
   *    during transition or unvoiced frame.  If IOUT is negative,
   *    more than 1 error occurred, use previous frame's parameters.    */

	  if ((icorf & abit[3]) != 0)
	{
	  errcnt = 0;
	  lsb = g_drms[pres] & 1;
		index = (INT)(g_drc[pres][7] * 16 + g_drms[pres] * 0.5); /* $$$ cast a int */
	  ham84 (index, &iout, &errcnt);
	  g_drms[pres] = g_drms[past];
	  if (iout >= 0)
		g_drms[pres] = iout * 2 + lsb;

	  for (i = 1; i <= 4; i++)
	    {
		  if (i == 1)
		i1 = (g_drc[pres][8] & 7) * 2 + (g_drc[pres][9] & 1);
		  else
		i1 = g_drc[pres][8 - i] & 15;

		  i2 = g_drc[pres][4 - i] & 31;
	      lsb = i2 & 1;
				index = (INT)(16 * i1 + i2 * 0.5);  /* $$$ cast a int */
				ham84 (index, &iout, &errcnt);
	      if (iout >= 0)
		{
		  iout = iout * 2 + lsb;
		  if ((iout & 16) == 16)
		    iout = iout - 32;
		}
	      else
		iout = g_drc[past][4 - i];
		  g_drc[pres][4 - i] = iout;
	    }

/*  Determine error rate        */
		erate = (INT)(erate * .96875 + errcnt * 102); /* $$$ cast a int */
	}

/*  Get unsmoothed RMS, RC's, and PITCH */

	ITAB13_IRMS = g_drms[pres];

	for (i = 0; i < ORDER; i++)
		ITAB13_IRC(i) = g_drc[pres][i];
	if (ipit == 1)
		g_dpit[pres] = g_dpit[past];
	if (ipit == 3)
		g_dpit[pres] = g_dpit[fut];
	ITAB13_IPITCH = g_dpit[pres];

/*  If bit 2 of ICORF is set then smooth RMS and RC's,  */

      if ((icorf & abit[1]) != 0)
	{
		if (fabs ((g_drms[pres] - g_drms[fut])) >= corth[ixcor - 1][1]
			&& fabs ((g_drms[pres] - g_drms[past])) >= corth[ixcor - 1][1])
		ITAB13_IRMS = median (g_drms[past], g_drms[pres], g_drms[fut]);

	  for (i = 0; i < 6; i++)
		if (fabs ((g_drc[pres][i] - g_drc[fut][i])) >= corth[ixcor - 1][i + 2]
		&& fabs ((g_drc[pres][i] - g_drc[past][i])) >= corth[ixcor - 1][i + 2])
		  ITAB13_IRC(i) = median (g_drc[past][i], g_drc[pres][i], g_drc[fut][i]);
	}

/*  If bit 3 of ICORF is set then smooth pitch  */

      if ((icorf & abit[2]) != 0)
	{
		if (fabs ((g_dpit[pres] - g_dpit[fut])) >= corth[ixcor - 1][0]
			&& fabs ((g_dpit[pres] - g_dpit[past])) >= corth[ixcor - 1][0])
		ITAB13_IPITCH = median (g_dpit[past], g_dpit[pres], g_dpit[fut]);
	}

/*  If bit 5 of ICORF is set then RC(5) - RC(10) are loaded with
   *  values so that after quantization bias is removed in decode
   *  the values will be zero.  */
    }				/* end if first */
  if ((icorf & abit[4]) != 0)
	for (i = 4; i < ORDER; i++)
	  ITAB13_IRC(i) = zrc[i];



/*  House keeping  - one frame delay    */

  iovoic = ivoic;
  ivp2h = ITAB13_IVOICE & 1; /* voice[1] */
  g_dpit[past] = g_dpit[pres];
  g_dpit[pres] = g_dpit[fut];
  g_drms[past] = g_drms[pres];
  g_drms[pres] = g_drms[fut];

  for (i = 1; i <= ORDER; i++)
    {
	  g_drc[past][i - 1] = g_drc[pres][i - 1];
	  g_drc[pres][i - 1] = g_drc[fut][i - 1];
	}

	unquant (itab,voice,pitch,rms,rc);
}
