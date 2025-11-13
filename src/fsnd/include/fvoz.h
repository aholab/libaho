#ifndef __FVOZ_H
#define __FVOZ_H


/************************************/
/*   1992 (c) Grupo de Voz - DAET   */
/*         ETSII/IT Bilbao          */
/************************************/


/**********************************************************/

#include <stdio.h>

#include "tdef.h"
#include "cabdef0.h"
#include "cabdef1.h"
#include "uti.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

typedef struct {
  FILE * f;
  struct cgsenal cgs;
  FLOAT4 min, max;
  UCHAR1 tdata;
  } fseni;

typedef struct {
  fntmp * fn;
  FILE * f;
  UINT4 nm;
  UCHAR1 tdata;
  } fseno;

typedef struct {
  fntmp * fn;
  FILE * f;
  UINT4 nm;
  UCHAR1 tdata;
  } fslpco;

typedef struct {
  FILE * f;
  struct cgsenal cgs;
  struct cgp_lpc cgpl;
  BOOL haycgpl;
  FLOAT4 min, max;
  UCHAR1 tdata;
  } fslpci;

typedef struct {
  FILE * f;
  struct cgp_lpc cgpl;
  FLOAT4 * lpcvec;
  } flpci;

typedef struct {
  fntmp * fn;
  FILE * f;
  UINT4 nv;
  UINT2 dimv;
  BOOL incG;
  } flpco;

typedef struct {
  fntmp * fn;
  FILE * f;
  UINT4 nv;
  UINT2 dimv;
  BOOL incG;
  } fparo;

/**********************************************************/

#define get_tdato(sfloat) ( (sfloat) ? TD_FLOAT4 : TD_INT2 )

/**********************************************************/

fseni * fseni_construct( pCHAR fname );

fseni * fseni_v_construct( pCHAR fname );

void fseni_destruct( fseni * fs );

struct cgsenal * fseni_getcgs( fseni * fs );

UINT4 fseni_getnm( fseni * fs );

FLOAT4 fseni_getfm( fseni * fs );

FLOAT4 fseni_getsamp( fseni * fs );

INT2 fseni_geti16samp( fseni * fs );

/**********************************************************/

fseno * fseno_construct( pCHAR fname,
                         struct cgsenal * cgs,
                         UINT2 ftyp,
                         UCHAR1 tdata,
                         pCHAR tmpname );

void fseno_destruct( fseno * fs );

UINT4 fseno_getnm( fseno * fs );

void fseno_setsamp( fseno * fs,
                    FLOAT4 samp );

void fseno_seti16samp( fseno * fs,
                      INT2 samp );

/**********************************************************/

fslpco * fslpco_construct( pCHAR fname,
                           struct cgsenal * cgs,
                           struct cgp_lpc * cgpl,
                           UINT2 ftyp,
                           UCHAR1 tdata,
                           pCHAR tmpname );

void fslpco_destruct( fslpco * fs );

UINT4 fslpco_getnm( fslpco * fs );

void fslpco_setsamp( fslpco * fs,
                     FLOAT4 samp );

/**********************************************************/

fslpci * fslpci_construct( pCHAR fname );

void fslpci_destruct( fslpci * fs );

struct cgsenal * fslpci_getcgs( fslpci * fs );

BOOL fslpci_haycgpl( fslpci * fs );

struct cgp_lpc * fslpci_getcgpl( fslpci * fs );

UINT4 fslpci_getnm( fslpci * fs );

FLOAT4 fslpci_getfm( fslpci * fs );

FLOAT4 fslpci_getsamp( fslpci * fs );

/**********************************************************/

flpci * flpci_construct( pCHAR fname );

void flpci_destruct( flpci * fl );

struct cgp_lpc * flpci_getcgp_lpc( flpci * fl );

UINT4 flpci_getnvec( flpci * fl );

UINT2 flpci_getdimvlpc( flpci * fl );

BOOL flpci_getginc( flpci * fl );

pFLOAT4 flpci_getvlpc( flpci * fl,
                        FLOAT4 * G );

/**********************************************************/

flpco * flpco_construct( pCHAR fname,
                         struct cgsenal * cgs,
                         struct cgp_lpc * cgpl,
                         pCHAR tmpname );

void flpco_destruct( flpco * fl );

UINT4 flpco_getnvec( flpco * fl );

UINT2 flpco_getdimvlpc( flpco * fl );

BOOL flpco_getginc( flpco * fl );

void flpco_setvlpc( flpco * fl,
                    pFLOAT4 lpcvec,
                    FLOAT4 G );

/**********************************************************/

fparo * fparo_construct( pCHAR fname,
                         struct cgsenal * cgs,
                         struct cgp_parcor * cgpp,
                         pCHAR tmpname );

void fparo_destruct( fparo * fp );

UINT4 fparo_getnvec( fparo * fp );

UINT2 fparo_getdimvpar( fparo * fp );

BOOL fparo_getginc( fparo * fp );

void fparo_setvpar( fparo * fp,
                    pFLOAT4 parvec,
                    FLOAT4 G );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
