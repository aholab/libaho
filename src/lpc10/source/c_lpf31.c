#include "c_lpc10.h"

/**********************************************************/
/* 31 Point Equiripple FIR Low-Pass Filter  (filter delay=15)
Passband:  ripple = 0.5 dB, cutoff =  800 Hz
Stopband:  atten. =  40 dB, cutoff = 1240 Hz
Inputs:
.  inbuf[] - Input speech buffer (0:PWLEN-1, last LFRAME+30 samples used)
Output:
.  lpbuf[] - Low passed speech buffer (0:PWLEN-1, last LFRAME written) */

VOID lpfilt31( FLOAT inbuf[], FLOAT lpbuf[] )
{
	INDEX j, i;
#define F FLOAT
	static FLOAT h[16] = {
		(F)-0.0097201988, (F)-0.0105179986, (F)-0.0083479648, 
		(F)0.0005860774, (F)0.0130892089, (F)0.0217052232, 
		(F)0.0184161253, (F)0.0003397230,	(F)-0.0260797087, 
		(F)-0.0455563702, (F)-0.0403068550, (F)0.0005029835,
		(F)0.0729262903, (F)0.1572008878, (F)0.2247288674, 
		(F)0.2505359650
	};
#undef F

	for (j = PWLEN - LFRAME; j < PWLEN; j++) {
		lpbuf[j] = h[15] * inbuf[j - 15];
		for (i = 0; i < 15; i++)
			lpbuf[j] += h[i] * (inbuf[j - i] + inbuf[j - 30 + i]);
	}
}

/* Original implementation: */
/*
VOID lpfilt31( FLOAT inbuf[], FLOAT lpbuf[] )
{
	INDEX j;
	static FLOAT h0 = -0.0097201988;
	static FLOAT h1 = -0.0105179986;
	static FLOAT h2 = -0.0083479648;
	static FLOAT h3 = 0.0005860774;
	static FLOAT h4 = 0.0130892089;
	static FLOAT h5 = 0.0217052232;
	static FLOAT h6 = 0.0184161253;
	static FLOAT h7 = 0.0003397230;
	static FLOAT h8 = -0.0260797087;
	static FLOAT h9 = -0.0455563702;
	static FLOAT h10 = -0.0403068550;
	static FLOAT h11 = 0.0005029835;
	static FLOAT h12 = 0.0729262903;
	static FLOAT h13 = 0.1572008878;
	static FLOAT h14 = 0.2247288674;
	static FLOAT h15 = 0.2505359650;

	for (j = PWLEN - LFRAME; j < PWLEN; j++) {
		lpbuf[j] = h0 * (inbuf[j] + inbuf[j - 30]);
		lpbuf[j] += h1 * (inbuf[j - 1] + inbuf[j - 29]);
		lpbuf[j] += h2 * (inbuf[j - 2] + inbuf[j - 28]);
		lpbuf[j] += h3 * (inbuf[j - 3] + inbuf[j - 27]);
		lpbuf[j] += h4 * (inbuf[j - 4] + inbuf[j - 26]);
		lpbuf[j] += h5 * (inbuf[j - 5] + inbuf[j - 25]);
		lpbuf[j] += h6 * (inbuf[j - 6] + inbuf[j - 24]);
		lpbuf[j] += h7 * (inbuf[j - 7] + inbuf[j - 23]);
		lpbuf[j] += h8 * (inbuf[j - 8] + inbuf[j - 22]);
		lpbuf[j] += h9 * (inbuf[j - 9] + inbuf[j - 21]);
		lpbuf[j] += h10 * (inbuf[j - 10] + inbuf[j - 20]);
		lpbuf[j] += h11 * (inbuf[j - 11] + inbuf[j - 19]);
		lpbuf[j] += h12 * (inbuf[j - 12] + inbuf[j - 18]);
		lpbuf[j] += h13 * (inbuf[j - 13] + inbuf[j - 17]);
		lpbuf[j] += h14 * (inbuf[j - 14] + inbuf[j - 16]);
		lpbuf[j] += h15 * (inbuf[j - 15]);
	}
}

*/

/**********************************************************/
