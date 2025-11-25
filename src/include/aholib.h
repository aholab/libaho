#ifndef __AHOLIB_H__
#define __AHOLIB_H__

#include "arch.h"
#include "cabcod.h"
#include "cabctrl.h"
#include "cabdef0.h"
#include "cabdef1.h"
#include "cabecer.h"
#include "chrono.h"
#include "clargs.h"
#include "codec.h"
#include "fvblk.h"
#include "fvoz.h"
#include "lpc10.h"
#include "spl.h"
#include "spltdef.h"
#include "stat.h"
#include "tdef.h"
#include "uti.h"
#define __USE_GRAPHICS__ISC_NO
#if defined(__OS_WINDOWS__) && defined (__USE_GRAPHICS__)
#include "graph.h"
#include "vplot.h"
#endif
#include "xalloc.h"
#include "xfft.h"
#include "wrkbuff.h"

#include "str2win.h"


#ifdef __OS_MSDOS__
#include "aholib.h"
#include "blaster.h"
#include "dma.h"
#include "dmabuff.h"
#include "dt28.h"
#include "intrs.h"
#include "lmem.h"
#include "mouse.h"
#include "xmouse.h"
#include "ports.h"
#include "sb.h"
#include "sb16io.h"
#include "xkbd.h"
#include "xsnd.h"
#include "accel.h"
#endif

#endif

