#include <math.h>
#include "c_lpc10.h"

/**********************************************************/
/* Voicing coefficient and Linear Discriminant Analysis variables:
Max number of VDC's and VDC levels */
#define MAXVDC 10
#define NVDCL 5

/* Voicing Decision Parameter vector ((*) denotes zero coefficient):
.    maxmin (*)
.    lbe/lbve
.    zc
.    rc1
.    qs
.    ivrc2
.    ar_b
.    ar_f
.    log(lbe/lbve) (*)
Define 2-D voicing decision coefficient vector according to the voicing
parameter order above.  Each column (VDC vector) is optimized for a specific
SNR. The last element of the vector is the constant. */
static FLOAT G_vdc[MAXVDC][NVDCL] = {
	{     0,     0,     0,     0,     0 },
	{  1714,   874,   510,   500,   500 },
	{  -110,   -97,   -70,   -10,     0 },
	{   334,   300,   250,   200,     0 },
	{ -4096, -4096, -4096, -4096, -4096 },
	{  -654, -1021, -1270, -1300, -1300 },
	{  3752,  2451,  2194,  2000,  2000 },
	{  3769,  2527,  2491,  2000,  2000 },
	{     0,     0,     0,     0,     0 },
	{  1181,  -500, -1500, -2000, -2500 }
};

/* SNR levels */
static FLOAT G_vdcl[NVDCL] = { 600, 450, 300, 200, 0 };

/**********************************************************/
/* Voicing Detection: makes voicing decisions for each half
frame of input speech.  Tentative voicing decisions are made two frames
in the future (2F) for each half frame.  These decisions are carried
through one frame in the future (1F) to the present (P) frame where
they are examined and smoothed, resulting in the final voicing
decisions for each half frame.
The voicing parameter (signal measurement) column vector ({value})
is based on a rectangular window of speech samples determined by the
window placement algorithm.  The voicing parameter vector contains the
AMDF windowed maximum-to-minimum ratio, the zero crossing rate, energy
measures, reflection coefficients, and prediction gains.  The voicing
window is placed to avoid contamination of the voicing parameter vector
with speech onsets.
The input signal is then classified as unvoiced (including
silence) or voiced.  This decision is made by a linear discriminant
function consisting of a dot product of the voicing decision
coefficient (vdc) row vector with the measurement column vector
({value}).  The {vdc} vector is 2-dimensional, each row vector is optimized
for a particular signal-to-noise ratio (SNR).  So, before the dot
product is performed, the SNR is estimated to select the appropriate
{vdc} vector.
The smoothing algorithm is a modified median smoother.  The
voicing discriminant function is used by the smoother to determine how
strongly voiced or unvoiced a signal is.  The smoothing is further
modified if a speech onset and a voicing decision transition occur
within one half frame.  In this case, the voicing decision transition
is extended to the speech onset.  For transmission purposes, there are
constraints on the duration and transition of voicing decisions.  The
smoother takes these constraints into account.
Finally, the energy estimates are updated along with the dither
threshold used to calculate the zero crossing rate (ZC).

Inputs:
.  vwin[AF][2] - Voicing window limits (vwin[AF-1][0] to vwin[AF-1][1])
.  inbuf[] - Raw input speech (with DC bias removed each frame) (vwin limited)
.  lpbuf[] - Low-pass filtered speech buffer (vwin limited)
.  half - Present analysis half frame number
.  minamd - Minimum value of the AMDF
.  maxamd - Maximum value of the AMDF
.  mintau - Pointer to the lag of the minimum AMDF value
.  ivrc[2] - Inverse filter's RC's
.  obound[AF] - Onset boundary descriptions
Output:
.  voibuf[AF+1][2] - Buffer of voicing decisions
Internal global:
.  maxmin - AMDF's 1 octave windowed maximum-to-minimum ratio
.  lbve - Low band voiced energy
.  lbue - Low band unvoiced energy
.  fbve - Full band voiced energy
.  fbue - Full band unvoiced energy
.  slbue - Scaled low band unvoiced energy
.  sfbue - Scaled full band unvoiced energy
.  voice[AF][2] - History of LDA results
.  snr - Estimate of signal-to-noise ratio
.  dither - Zero crossing threshold level
.  ofbue - Previous full-band unvoiced energy
.  olbue - Previous low-band unvoiced energy

Internal:
.  qs - Ratio of preemphasized to full-band energies
.  rc1 - First reflection coefficient
.  ar_b - Product of the causal forward and reverse pitch prediction gains
.  ar_f - Product of the noncausal forward and rev. pitch prediction gains
.  zc - Zero crossing rate
.  vdcl[NVDCL] - SNR values corresponding to the set of vdc's
.  vdc[MAXVDC][NVDCL] - 2-D voicing decision coefficient vector
.  value[9] - Voicing Parameters
.  lbe - Ratio of low-band instantaneous to average energies
.  fbe - Ratio of full-band instantaneous to average energies
.  snr2 - Estimate of low-band signal-to-noise ratio
.  snrl - SNR level number
.  ot - Onset transition present
.  vstate - Decimal interpretation of binary voicing classifications
Constants:
.  AF - The analysis frame number
.  REF - Reference energy for initialization and {dither} threshold
.  MAXVDC - Max number of VDC's
.  NVDCL - Number of VDC levels */

VOID voicin( INDEX vwin[AF][2], FLOAT inbuf[], FLOAT lpbuf[], INDEX half,
		FLOAT minamd, FLOAT maxamd, INDEX mintau, FLOAT ivrc[2],
		INDEX* obound, BOOL voibuf[AF+1][2] )
{
	INDEX zc, lbe, fbe, i, snrl, vstate;
	FLOAT snr2, qs, rc1, ar_b, ar_f;
	FLOAT value[9];
	BOOL ot;

/* The {voice} array contains the result of the linear discriminant function
(analog values).  The {voibuf} array contains the hard-limited binary
voicing decisions.  The {voice} and {voibuf} arrays, according to C
memory allocation, are addressed as: [future-frame num][half-frame num]
|   Past    |  Present  |  Future1  |  Future2  |
| 0,0 | 0,1 | 1,0 | 1,1 | 2,0 | 2,1 | 3,0 | 3,1 |  --->  time */

/* Update linear discriminant function history each frame: */
	if (!half) {
		g_voice[0][0] = g_voice[1][0];
		g_voice[0][1] = g_voice[1][1];
		g_voice[1][0] = g_voice[2][0];
		g_voice[1][1] = g_voice[2][1];
		if (minamd>1)
			g_maxmin = maxamd / minamd;
		else
			g_maxmin = maxamd;
	}

/* Calculate voicing parameters, twice per frame: */
	vparms(vwin, inbuf, lpbuf, half, &g_dither, mintau, &zc,
			&lbe, &fbe, &qs, &rc1, &ar_b, &ar_f);

/* Estimate signal-to-noise ratio to select the appropriate {vdc} vector.
The SNR is estimated as the running average of the ratio of the
running average full-band voiced energy to the running average
full-band unvoiced energy. SNR filter has gain of 63. */
	g_snr = (FLOAT)((63.*(g_snr + ((g_fbue>1)?g_fbve/(FLOAT)g_fbue:g_fbve))/64.)+.5);
	snr2 = (g_lbue>1) ? (g_snr*g_fbue)/g_lbue : (g_snr*g_fbue);

/* Quantize SNR to {snrl} according to {vdcl} thresholds.
Note: {snrl} can reach up to NVDCL-1 here */
	for (snrl = 0; snrl < (NVDCL-1); snrl++)
		if (snr2 > G_vdcl[snrl])
			break;

/* Linear discriminant voicing parameters: */
	value[0] = g_maxmin;
	value[1] = (g_lbve>1) ? (FLOAT)lbe/(FLOAT)g_lbve : (FLOAT)lbe;
	value[2] = (FLOAT)zc;
	value[3] = rc1;
	value[4] = qs;
	value[5] = ivrc[1];
	value[6] = ar_b;
	value[7] = ar_f;
	value[8] = 0.;

/* Evaluation of linear discriminant function: */
	g_voice[AF-1][half] = G_vdc[9][snrl];
	for (i = 0; i < 9; i++)
		g_voice[AF-1][half] += G_vdc[i][snrl] * value[i];

/* Classify as voiced if discriminant > 0, otherwise unvoiced
Voicing decision for current half-frame: TRUE = voiced; FALSE = Unvoiced */
	voibuf[AF][half] = (g_voice[AF-1][half] > 0.);

	if (half) {  /* Skip voicing decision smoothing in first half-frame: */
/* Voicing decision smoothing rules (override of linear combination):
- Unvoiced half-frames: At least two in a row.
- voiced half-frames: At least two in a row in one frame. Otherwise
at least three in a row. (Due to the way transition frames are encoded)

In many cases, the discriminant function determines how to smooth.
In the following chart, the decisions marked with a * may be overridden.

Voicing override of transitions at onsets: If a V/UV or UV/V voicing
decision transition occurs within one-half frame of an onset bounding
a voicing window, then the transition is moved to occur at the onset.

P       1F
-----   -----
0   0   0   0
0   0   0*  1   (If there is an onset there)
0   0   1*  0*  (Based on 2F and discriminant distance)
0   0   1   1
0   1*  0   0   (Always)
0   1*  0*  1   (Based on discriminant distance)
0*  1   1   0*  (Based on past, 2F, and discriminant distance)
0   1*  1   1   (If there is an onset there)
1   0*  0   0   (If there is an onset there)
1   0   0   1
1   0*  1*  0   (Based on discriminant distance)
1   0*  1   1   (Always)
1   1   0   0
1   1   0*  1*  (Based on 2F and discriminant distance)
1   1   1*  0   (If there is an onset there)
1   1   1   1  */

/*Determine if there is an onset transition between P and 1F.
{ot} (Onset Transition) is true if there is an onset between
P and 1F but not after 1F. */
		ot = ((obound[0] & 2) || (obound[1]==1)) && !(obound[2] & 1);

/* Multi-way dispatch on voicing decision history: */
		vstate = (INDEX)(voibuf[1][0])*8 + (INDEX)(voibuf[1][1])*4
				+ (INDEX)(voibuf[2][0])*2 + (INDEX)(voibuf[2][1]);
		switch (vstate) {
		case 0:
			break;
		case 1:
			if (ot && voibuf[3][0])
				voibuf[2][0] = TRUE;
			break;
		case 2:
			if ((!voibuf[3][0]) || (g_voice[1][0] < -g_voice[1][1]))
				voibuf[2][0] = FALSE;
			else
				voibuf[2][1] = TRUE;
			break;
		case 3:
			break;
		case 4:
			voibuf[1][1] = FALSE;
			break;
		case 5:
			if (g_voice[0][1] < -g_voice[1][0])
				voibuf[1][1] = FALSE;
			else
				voibuf[2][0] = TRUE;
			break;
		case 6:
			if (voibuf[0][0] || voibuf[3][0] || (g_voice[1][1] > g_voice[0][0]))
				voibuf[2][1] = TRUE;
			else
				voibuf[1][0] = TRUE;
			break;
		case 7:
			if (ot)
				voibuf[1][1] = FALSE;
			break;
		case 8:
			if (ot)
				voibuf[1][1] = TRUE;
			break;
		case 9:
			break;
		case 10:
			if (g_voice[1][0] < -g_voice[0][1])
				voibuf[2][0] = FALSE;
			else
				voibuf[1][1] = TRUE;
			break;
		case 11:
			voibuf[1][1] = TRUE;
			break;
		case 12:
			break;
		case 13:
			if ((!voibuf[3][0]) && (g_voice[1][1] < -g_voice[1][0]))
				voibuf[2][1] = FALSE;
			else
				voibuf[2][0] = TRUE;
			break;
		case 14:
			if (ot && (!voibuf[3][0]))
				voibuf[2][0] = FALSE;
			break;
		default:
			break;
		}
	}

/* Now update parameters:
During unvoiced half-frames, update the low band and full band unvoiced
energy estimates ({lbue} and {fbue}) and also the zero crossing
threshold ({dither}).  (The input to the unvoiced energy filters is
restricted to be less than 10dB (*3.) above the previous inputs of the
filters.)
During voiced half-frames, update the low-pass ({lbve}) and all-pass
({fbve}) voiced energy estimates. */
	if (!voibuf[3][half]) {
		g_ofbue *= 3;   /* +10dB */
		g_sfbue = (LINDEX)((63. * g_sfbue + 8. * MMIN(fbe, g_ofbue)) / 64. + .5);
		g_fbue = (INDEX)(g_sfbue / 8);
		g_ofbue = fbe;
		g_olbue *= 3;   /* +10dB */
		g_slbue = (LINDEX)((63. * g_slbue + 8. * MMIN(lbe, g_olbue)) / 64. + .5);
		g_lbue = (INDEX)(g_slbue / 8);
		g_olbue = lbe;
	}
	else {
		g_lbve = (INDEX)((63. * g_lbve + lbe) / 64. + .5);
		g_fbve = (INDEX)((63. * g_fbve + fbe) / 64. + .5);
	}

/* Set dither threshold to yield proper zero crossing rates in the
presence of low frequency noise and low level signal input.
NOTE: The divisor is a function of REF, the expected energies. */
	g_dither = (FLOAT)(64. * sqrt((FLOAT)g_lbue * (FLOAT)g_lbve) / REF);
	if (g_dither<1.)
		g_dither = 1.;
	else if (g_dither>20.)
		g_dither = 20.;

/* Voicing decisions are returned in {voibuf}. */
}

/**********************************************************/

