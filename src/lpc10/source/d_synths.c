#include "d_lpc10.h"
#include <math.h>

/**********************************************************/
/*
NOTE: There is excessive buffering here, BSYNZ and DEEMP should be
  changed to operate on variable positions within SOUT.  Also,
  the output length parameter is bogus, and PITSYN should be
  rewritten to allow a constant frame length output */

static FLOAT sout[LFRAME];	/* $$$ y si genera mas de LFRAME?? */

void synths(BOOL voice[], INDEX pitch, FLOAT rms, FLOAT rc[], FLOAT speech[], INDEX * k)
{
	INDEX i, j, nout;
	FLOAT ratio, g2pass;
	FLOAT gprime = (FLOAT)0.7;

	pitch = MMAX(MMIN(pitch, 156), 20);
	for (i = 0; i < ORDER; i++)
		rc[i] = MMAX(MMIN(rc[i], (FLOAT).99), (FLOAT)-.99);

	*k = 0;
	pitsyn(voice, pitch, rms, rc, ivuv, ipiti, rmsi, rci, &nout, &ratio);
	if (nout > 0) {
		for (j = 0; j < nout; j++) {
			irc2pc(rci[j], pc, gprime, &g2pass);
			bsynz(pc, ipiti[j], ivuv[j], sout, rmsi[j], ratio, g2pass);

#ifdef VIEW_EXCIT
/* define VIEW_EXCIT to view just voiced/unvoiced source */
/* Don't apply deemphasis */
#else
			deemp(sout, ipiti[j]);
#endif

			for (i = 0; i < ipiti[j]; i++) {
				speech[*k] = (FLOAT)(sout[i] * 0.000244140625);
				(*k)++;
			}
		}
	}
}

/**********************************************************/
