#ifndef __T_LPC10_H
#define __T_LPC10_H


/**********************************************************/

#include "x_lpc10.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

VOID quant( BOOL voice[2], INDEX pitch, FLOAT rms, FLOAT rc[ORDER],
		INDEX itab[13] );
VOID unquant ( INDEX itab[13], BOOL voice[2],
		INT * pitch, FLOAT * rms, FLOAT rc[ORDER]);

INT median( INT d1, INT d2, INT d3 );
VOID ham84 (INT input, INT * output, INT * errcnt);

VOID encode_2400( INDEX itab[13] );
VOID chan_write_2400_i( INDEX itab[12], INDEX ibits[54]);

VOID send_2400_ini( VOID );
VOID send_2400_i( BOOL voice[2], INDEX pitch,
		FLOAT rms, FLOAT rc[ORDER], INDEX ibits[54] );

VOID chan_write_dgram( INDEX itab[13], BYTE buf[7]);
VOID encode_dgram( INDEX itab[13] );
VOID send_dgram_ini( VOID );
VOID send_dgram( BOOL voice[2], INDEX pitch,
		FLOAT rms, FLOAT rc[ORDER], BYTE buf[7] );

VOID chan_read_2400_i( INDEX ibits[54], INDEX itab[12] );
VOID chan_read_dgram( BYTE buf[7], INDEX itab[13] );
VOID decode_2400 ( INDEX itab[13], BOOL voice[2],
	INT * pitch, FLOAT * rms, FLOAT rc[ORDER]);
VOID decode_dgram( INDEX itab[13], BOOL voice[2],
	INT * pitch, FLOAT * rms, FLOAT rc[ORDER]);
VOID recv_2400_ini( VOID );
VOID recv_2400_i( INDEX ibits[54], BOOL voice[2], INDEX *pitch,
		FLOAT *rms, FLOAT rc[ORDER] );
VOID recv_dgram_ini( VOID );

VOID recv_dgram( BYTE buf[7], BOOL voice[2], INDEX *pitch,
		FLOAT *rms, FLOAT rc[ORDER] );

/**********************************************************/

#define ITAB13_IVOICE itab[12]
#define ITAB13_IPITCH itab[0]
#define ITAB13_IRMS itab[1]
#define ITAB13_IRC(i)  itab[2+i]

/**********************************************************/
/* send/recv 2400 */

extern INDEX G_iblist[53];

extern INDEX g_drc[3][ORDER];
extern INDEX g_dpit[3];
extern INDEX g_drms[3];

/**********************************************************/

extern INDEX G_rmst[64];

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
