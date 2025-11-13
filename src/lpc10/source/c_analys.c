#include "c_lpc10.h"

/**********************************************************/
/* Main analisys loop.

Constants:
.  NF - Number of frames
.  AF - Frame in which analysis is done
.  OSLEN - Length of the onset buffer
.  LTAU - Number of pitch lags
.  SBUFL, SBUFH - Start and end index of speech buffers
.  LBUFL, LBUFH - Start and end index of LPF speech buffer
.  MINWIN, MAXWIN - Min and Max length of voicing (and analysis) windows
.  PWLEN, PWINH, PWINL - Length, upper and lower limits of pitch window
.  DVWINL, DVWINH - Default lower and upper limits of voicing window

Data Buffers:
.  inbuf - Raw speech (with DC bias removed each frame)
.  pebuf - Preemphasized speech
.  lpbuf - Low pass speech buffer
.  ivbuf - Inverse filtered speech
.  osbuf - Indexes of onsets in speech buffers
.  vwin - Voicing window indices
.  awin - Analysis window indices
.  ewin - Energy window indices
.  voibuf - Voicing decisions on windows in {vwin}
.  rmsbuf - RMS energy
.  rcbuf - Reflection Coefficients

Pitch is handled separately from the above parameters. The following
variables deal with pitch:
.  midx - Encoded initial pitch estimate for analysis frame
.  pitch - The encoded pitch value (index into {tau}) for the present
.          frame (delayed and smoothed by dyptrk) */

VOID analys( VOID )
{
	static FLOAT abuf[MAXWIN];
	static FLOAT psi[ORDER];
	static FLOAT amdf[LTAU];
	static FLOAT phi[ORDER][ORDER];
	INDEX ewin[2];
	INDEX lanal, midx, minptr, maxptr, mintau;
	FLOAT ivrc[2];

/* Calculations are done on future frame due to requirements
of the pitch tracker.  Delay RMS and RC's 2 frames to give
current frame parameters on return: */

/* Place Voicing Window */
	preemp(g_inbuf+(SBUFH-LFRAME+1), g_pebuf+(SBUFH-LFRAME+1), LFRAME, g_zpre);
	onset(g_pebuf, g_osbuf, &g_osptr);
	placev(g_osbuf, g_osptr, &g_obound[AF-1], g_vwin);

/* The Pitch Extraction algorithm estimates the pitch for a frame
of speech by locating the minimum of the average magnitude difference
function (AMDF).  The AMDF operates on low-pass, inverse filtered
speech (The low-pass filter is an 800 Hz, 31 tap, equiripple, FIR
filter and the inverse filter is a 2nd-order LPC filter).  The pitch
estimate is later refined by dynamic programming (dyptrk).  However,
since some of dyptrk's parameters are a function of the voicing
decisions, a voicing decision must precede the final pitch estimation.
See subroutines lpfilt31(), ivfilt(), and tbdm(). */
	lpfilt31(g_inbuf+LBUFH-PWLEN-1, g_lpbuf+LBUFH-PWLEN+1);
	ivfilt(g_lpbuf+PWINL, g_ivbuf+PWINL, ivrc);
	tbdm(g_ivbuf+PWINL, G_tau, amdf, &minptr, &maxptr, &mintau);

/* Voicing decisions are made for each half frame of input speech.
An initial voicing classification is made for each half of the
analysis frame, and the voicing decisions for the present frame
are finalized.  See subroutine voicin().
The voicing detector (voicin()) classifies the input signal as
unvoiced (including silence) or voiced using the AMDF windowed
maximum-to-minimum ratio, the zero crossing rate, energy measures,
reflection coefficients, and prediction gains.
The pitch and voicing rules (dyptrk()) apply smoothing and isolated
corrections to the pitch and voicing estimates and, in the process,
introduce two frames of delay into the corrected pitch estimates and
voicing decisions. */
	/* first half */
	voicin(g_vwin, g_inbuf, g_lpbuf, 0, amdf[minptr], amdf[maxptr], mintau, ivrc, g_obound, g_voibuf);
	/* second half */
	voicin(g_vwin, g_inbuf, g_lpbuf, 1, amdf[minptr], amdf[maxptr], mintau, ivrc, g_obound, g_voibuf);
/* Find the minimum cost pitch decision over several frames given
the current voicing decision and the AMDF array. tau[midx] is the
initial pitch computed for frame AF (ipitch, decoded from {midx})*/
	dyptrk(amdf, minptr, g_voibuf[AF][1], &g_pitch, &midx);

/* Place spectrum analysis and energy windows.
{awin} marks start/end of analysis window. {ewin} marks start/end
of energy calculation window. Both are refered to {pebuf} buffer. */
	placea(G_tau[midx], g_voibuf, g_obound[AF-1], g_vwin, g_awin, ewin);
/* analysis window length */
	lanal = g_awin[AF-1][1] - g_awin[AF-1][0] + 1;
/* Remove short term DC bias over the analysis window. Put result in {abuf},
which will hold directly the {lanal} elements of the analysis frame. */
	dcbias(lanal, g_pebuf + g_awin[AF-1][0], abuf);
/* Compute RMS over integer number of pitch periods within the analysis window. */
	g_rmsbuf[AF-1] = energy_sqrt(ewin[1] - ewin[0] + 1,
		abuf + (ewin[0] - g_awin[AF-1][0]));

/* Matrix load and invert, check RC's for stability  */
	mload(lanal, abuf, phi, psi);

	invert(phi, psi, g_rcbuf[AF-1]);
	rcchk(g_rcbuf[AF-2], g_rcbuf[AF-1]);
}

/**********************************************************/

