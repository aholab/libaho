/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1996 - Grupo de Voz (DAET) ETSII/IT-Bilbao 
                  & CSTR - Univ. of Edinburgh

Nombre fuente................ -
Nombre paquete............... -
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... desarrollo
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.1    09/04/99  Borja     uso tipos UINT
0.0.2    30/08/98  Borja     split en varios modulos afwav_?.cpp
0.0.1    27/07/97  Borja     adapted to compile on SunOS.
0.0.0    15/07/96  Borja     codificacion inicial.

======================== Contenido ========================
ver afwav.cpp
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "afwav_i.hpp"
#include "xalloc.h"

/**********************************************************/

LONG AFWav::HdrR( FILE *f, KVStrList &, BOOL override )
{
#define ISCK(lbl) (!memcmp(ck.chunkName,lbl,4))
#define ADDSTR(lbl) { char *s=wstr(f); ADDIFNOV(lbl,s); xfree(s); }
	WAVHdr h;
	BOOL iswav=FALSE;
	CueNames cnames;
	BOOL format=FALSE;
	BOOL marks=FALSE;
	BOOL skipmarks=FALSE;

	memset(&h,0,sizeof(h));
	fseek(f,0,SEEK_SET);
	if (fread(&h,sizeof(h),1,f)) {
		if (!memcmp(h.Literal_RIFF,"RIFF",4) &&
			!memcmp(h.Literal_WAVE,"WAVE",4)) iswav=TRUE;
	}
	cdie_beep(!iswav,"%s: not a WAV file",fFormat());
	endian_fromlittle32(&(h.len));

	datapos=-1;
	UINT32 datalen=0;
	UINT32 ssize=2;
	INT nchan=1;

	while (xftell(f)-8<(long)h.len) {
		WAVChunk ck;
		long ckpos=xftell(f);
		xfread(&ck,sizeof(ck),1,f); endian_fromlittle32(&(ck.len));
		if (ISCK("fmt ")) {  // format chunck
			FormatInfo fmt;
			format = TRUE;
			xfread(&fmt,sizeof(fmt),1,f); endian_fromlittle16(&(fmt.wFormatTag)); 
			endian_fromlittle16(&(fmt.wChannels)); endian_fromlittle32(&(fmt.dwSamplesPerSec)); 
			endian_fromlittle32(&(fmt.dwAvgBytesPerSec)); endian_fromlittle16(&(fmt.wBlockAlign));

			cdie_beep(fmt.wFormatTag!=WFORMAT_PCM,"%s: not supported format in WAV file",fFormat());
			ADDIFNOV(CAUDIO_NCHAN,(INT)(fmt.wChannels));
			nchan=(UINT)fa->info().lval(CAUDIO_NCHAN);
			ADDIFNOV(CAUDIO_SRATE,(DOUBLE)fmt.dwSamplesPerSec);
			UINT16 slen;
			xfread(&slen,sizeof(slen),1,f); endian_fromlittle16(&slen);  // PCM sample len
			cdie_beep(slen>16,"%s: sample size too long in WAV file",fFormat());
			if (slen>8) ADD(CAUDIO_SAMPTYPE,SAMPTYPE_STR_PCM16);
			else { ADD(CAUDIO_SAMPTYPE,SAMPTYPE_STR_PCM8U); ssize=1; }
      ADDIFNOV(CAUDIO_BIGENDIAN,"no");  // wav es little endian
		}
		else if (ISCK("data")) { // audio data chunk
			cdie_beep(datapos!=-1,"AFWav error: multiple data blocks in WAV files not supported");
			datapos=xftell(f);
			datalen=ck.len;
		}
		else if (ISCK("LIST")) {
			xfread(&ck,4,1,f);
			if (ISCK("INFO")) {  // misc. information
				while (xftell(f)-ckpos-sizeof(ck)<ck.len) {
					xfread(&ck,4,1,f);
					if (ISCK("IART")) ADDSTR("OriginalArtist")
					else if (ISCK("ICMT")) ADDSTR("Comments")
					else if (ISCK("ICOP")) ADDSTR("Copyright")
					else if (ISCK("ICRD")) ADDSTR("CreationDate")
					else if (ISCK("IENG")) ADDSTR("Engineers")
					else if (ISCK("IGNR")) ADDSTR("Genre")
					else if (ISCK("IKEY")) ADDSTR("KeyWords")
					else if (ISCK("IMED")) ADDSTR("OriginalMedium")
					else if (ISCK("INAM")) ADDSTR("Name")
					else if (ISCK("ISFT")) ADDSTR("SoftwarePackage")
					else if (ISCK("ISRC")) ADDSTR("SourceSupplier")
					else if (ISCK("ITCH")) ADDSTR("Digitizer")
					else if (ISCK("ISBJ")) ADDSTR("Subject")
					else if (ISCK("ISRF")) ADDSTR("DigitizationSource")
					else xfree(wstr(f));
				}
			}
			else if (ISCK("wavl")) die_beep("%s: multi block WAV files not supported",fFormat());
			else if (ISCK("adtl") && (!skipmarks)) {
				cdie_beep(marks==FALSE,"%s: cue assoc. data chunk before cue chunk in WAV file is not supported",fFormat());
				while (xftell(f)-ckpos-sizeof(ck)<ck.len) {
					xfread(&ck,4,1,f);
					if (ISCK("labl")) mrk_labl(fa->marks(),cnames,wstr(f));
					else if (ISCK("note")) mrk_note(fa->marks(),cnames,wstr(f));
					else if (ISCK("ltxt")) mrk_ltxt(fa->marks(),cnames,wstr(f));
					else {
						UINT32 l;
						xfread(&l,sizeof(l),1,f);  endian_fromlittle32(&l);
						xfseek(f,l,SEEK_CUR);
					}
				}
			}
		}
		else if (ISCK("DISP")) {  // display string
			xfseek(f,4,SEEK_CUR);  // line num??
			char *s=wstr(f,ck.len-4); ADDIFNOV("DisplayTitle",s); xfree(s);
		}
		else if (ISCK("cue ")) {  // marks
			skipmarks=!((fa->marks().length()==0) || override);
			if (!skipmarks) {
				fa->marks().clear();
				cnames.clear();
				cdie_beep(marks!=FALSE,"%s: multiple cue chunks in WAV file not supported",fFormat());
				marks=TRUE;
				UINT32 n;
				xfread(&n,sizeof(n),1,f);  endian_fromlittle32(&n);
				for (UINT32 i=0; i<n; i++) {
					CuePoint cp;
					xfread(&cp,sizeof(cp),1,f); endian_fromlittle32(&(cp.dwName)); 	
					endian_fromlittle32(&(cp.dwPosition)); endian_fromlittle32(&(cp.dwChunkStart)); 
					endian_fromlittle32(&(cp.dwBlockStart)); endian_fromlittle32(&(cp.dwSampleOffset));
					fa->marks().append(cp.dwSampleOffset);
					cnames._append(cp.dwName);
				}
			}
		}
		else {
			fprintf(stderr,"%s warning: Unknown chunk %c%c%c%c(%ld)\n",
					fFormat(), ck.chunkName[0],ck.chunkName[1], ck.chunkName[2],ck.chunkName[3], ck.len);
		}
		xfseek(f,ckpos+ck.len+sizeof(ck),SEEK_SET);  // goto next chunk
	}

	cdie_beep(format==FALSE, "%s: can't find format chunk in WAV file",fFormat());
	cdie_beep(datapos==-1, "%s: can't find audio samples in WAV file",fFormat());

	xfseek(f,datapos,SEEK_SET);

	if (fa->info().bval(CAUDIO_BIGENDIAN))
		fprintf(stderr,"%s warning: reading big endian file!.\n",fFormat());

	return datalen/ssize/nchan;
}

/**********************************************************/

BOOL AFWav::testFile( FILE *f )
{
	WAVHdr h;

	memset(&h,0,sizeof(h));
	fseek(f,0,SEEK_SET);
	if (fread(&h,sizeof(h),1,f)) {
		if (!memcmp(h.Literal_RIFF,"RIFF",4) &&
			!memcmp(h.Literal_WAVE,"WAVE",4))
		return TRUE;
	}
	return FALSE;
}

/**********************************************************/

#ifdef __CC_MSVC__
#pragma pack(pop)
#endif

/**********************************************************/
