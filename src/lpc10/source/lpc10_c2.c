#include <stdio.h>
#include <stdlib.h>

#include "lpc10.h"
#include "c_lpc10.h"
#include "sr_lpc10.h"

/**********************************************************/

BOOL g_used_c_dgram = FALSE;
BYTE g_dgram[7];

/**********************************************************/

CLPC10_DGRAM *clpc10_dgram_construct( VOID )
{
	if (g_used_c_dgram) {
		fprintf(stderr,"\nPor ahora no soporto mas de una estructura CLPC10_DGRAM!\7\n");
		exit(1);
	}

	g_used_c_dgram = TRUE;

	clpc10_dgram_reset((CLPC10_DGRAM*)1);
	return (CLPC10_DGRAM*)1;
}

/**********************************************************/

VOID clpc10_dgram_destruct( CLPC10_DGRAM *c )
{
	(void)c;  /* avoid "unused parameter" warning */
	g_used_c_dgram = FALSE;
}

/**********************************************************/

VOID clpc10_dgram_reset( CLPC10_DGRAM *c )
{
	(void)c;  /* avoid "unused parameter" warning */
	coder_ini();
	send_dgram_ini();
}

/**********************************************************/

LPC10_dgram_data *clpc10_dgram_code( CLPC10_DGRAM *c, FLOAT speech[LPC10_LFRAME] )
{
	(void)c;  /* avoid "unused parameter" warning */
/* shift sample and other buffers */
	shift();
/* prepare and copy input buffer */
	framein(speech);
/* analyze frame */
	analys();
/* channel encode and write */
	send_dgram(CODED_VOICE, CODED_PITCH, CODED_RMS, CODED_RC, g_dgram);

	return (LPC10_dgram_data*)g_dgram;
}

/**********************************************************/

LPC10_dgram_data *clpc10_dgram_code_i16( CLPC10_DGRAM *c, INT16 speech[LPC10_LFRAME] )
{
	(void)c;  /* avoid "unused parameter" warning */
/* shift sample and other buffers */
	shift();
/* prepare and copy input buffer */
	framein_i16(speech);
/* analyze frame */
	analys();
/* channel encode and write */
	send_dgram(CODED_VOICE, CODED_PITCH, CODED_RMS, CODED_RC, g_dgram);

	return (LPC10_dgram_data*)g_dgram;
}

/**********************************************************/
