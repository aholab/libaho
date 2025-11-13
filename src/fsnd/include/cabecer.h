#ifndef __CABECER_H
#define __CABECER_H


/************************************/
/*   1992 (c) Grupo de Voz - DAET   */
/*         ETSII/IT Bilbao          */
/************************************/


/**********************************************************/

#include <stdio.h>

#include "cabdef0.h"
#include "cabdef1.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

void vleecfs( FILE * fp, struct cgsenal * cgs );

void vescribecfs( FILE * fp, struct cgsenal * cgs );

UCHAR1 leecfs( FILE * fp, struct cgsenal * cgs, int tdato );

void escribecfs_ftyp( FILE * fp,
                      struct cgsenal * cgs,
                      UINT2 ftyp,
                      UCHAR1 tdata );

void escribecfs( FILE * fp,
                 struct cgsenal * cgs,
                 UCHAR1 tdata );

void escribe_cab_preenf( FILE * fp,
                         struct cgsenal * cgs,
                         UCHAR1 tdata );

void escribe_cab_deenf( FILE * fp,
                        struct cgsenal * cgs,
                        UCHAR1 tdata );

void escribe_cab_filt( FILE * fp,
                       struct cgsenal * cgs,
                       UCHAR1 tdata );

void escribe_cab_parcor( FILE * fp,
                         struct cgsenal * cgs,
                         struct cgp_parcor * cgpp );

void escribecfslpc( FILE * fp,
                    struct cgsenal * cgs,
                    struct cgp_lpc * cpl,
                    UINT2 ftyp,
                    UINT2 tdat );

UINT2 leecfslpc( FILE * fp,
                struct cgsenal * cgs,
                struct cgp_lpc * cpl,
                int tdato,
                BOOL * hayclpc );

void escribe_cab_lpc( FILE * fp,
                      struct cgsenal * cgs,
                      struct cgp_lpc * cgpl );

void lee_cab_lpc( FILE * fp,
                  struct cgp_lpc * cgpl );

void escribe_cab_err( FILE * fp,
                      struct cgsenal * cgs,
                      struct cgp_lpc * cgpl,
                      UCHAR1 tdata );

void escribe_cab_lpcsint( FILE * fp,
                          struct cgsenal * cgs,
                          struct cgp_lpc * cgpl,
                          UCHAR1 tdata );

void escribecfs_form( FILE * fp,
                      struct cgsenal * cgs,
                      int espera_en_marcas );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif

