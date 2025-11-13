#include "vplot.h"
#include "graph.h"
#include "uti.h"

/* SUPPORT_SMOOTHCLEAR:
si definido, atiendo a comando "smoothclear" que
permite repintar sobre grafica anterior borrando
poco a poco. Reduce flicker, pero es mas lento */
#define SUPPORT_SMOOTHCLEAR

template <class T>
void gr_xplot1D( int x1, int y1, int x2, int y2,
		T vec[], int n, KVStrList& opts )
/* cosas del BC31: no va bien "const KVStrList& opts", asi que sin
const, y cast en todos los sitios... */
{
	T min, max;
	int i,clip;
#ifdef SUPPORT_SMOOTHCLEAR
	int smooth;
	int ox=0;
#endif

	gr_bufferon();

	if (opts.contains("max")) max = (T)opts.dval("max");
	else for (i=1, max=vec[0]; i<n; i++) if (max<vec[i]) max=vec[i];
	if (opts.contains("min")) min = (T)opts.dval("min");
	else for (i=1, min=vec[0]; i<n; i++) if (min>vec[i]) min=vec[i];

	if (opts.contains("bg")) gr_setbkcolor(opts.ival("bg"));
	if (opts.contains("fg")) gr_setcolor(opts.ival("fg"));
#ifdef SUPPORT_SMOOTHCLEAR
	smooth = opts.bbval("smoothclear");
#endif
	if (opts.bbval("clear")) {
#ifdef SUPPORT_SMOOTHCLEAR
		if (!smooth) gr_bar(x1,y1,x2,y2,gr_getbkcolor(),gr_getbkcolor());
#else
		gr_bar(x1,y1,x2,y2,gr_getbkcolor(),gr_getbkcolor());
#endif
	}
#ifdef SUPPORT_SMOOTHCLEAR
	else smooth=0;
#endif
	if (opts.bbval("clip")) clip=1; else clip=0;
	if (opts.bbval("frame"))
		gr_rectangle(x1-1,y1-1,x2+1,y2+1);
	if (min==max) return;

	for (i=0; i<n; i++) {
		int x = (int)fround(linterpol(i,0,n-1,x1,x2));
		int y = (int)fround(linterpol(vec[i],min,max,y2,y1));
		if (clip) {
			if (y>y2) y=y2;
			if (y<y1) y=y1;
		}
		if (!i) {
#ifdef SUPPORT_SMOOTHCLEAR
			if (smooth) {
				gr_line(x,y1,x,y2,gr_getbkcolor());
				ox=x;
			};
#endif
			gr_moveto(x,y);
		}
		else {
#ifdef SUPPORT_SMOOTHCLEAR
		if (smooth && (x!=ox)) {
			gr_line(x,y1,x,y2,gr_getbkcolor());
			ox=x;
		};
#endif
		gr_lineto(x,y);
		}
	}

	gr_bufferoff();
}



void gr_plot1D( int x1, int y1, int x2, int y2,
		double vec[], int n, const KVStrList & opts )
{
	gr_xplot1D(x1,y1,x2,y2,vec,n,(KVStrList&)opts);
}

void gr_plot1D( int x1, int y1, int x2, int y2,
		int vec[], int n, const KVStrList & opts )
{
	gr_xplot1D(x1,y1,x2,y2,vec,n,(KVStrList&)opts);
}

void gr_plot1D( int x1, int y1, int x2, int y2,
		short vec[], int n, const KVStrList & opts )
{
	gr_xplot1D(x1,y1,x2,y2,vec,n,(KVStrList&)opts);
}

void gr_plot1D( int x1, int y1, int x2, int y2,
		float vec[], int n, const KVStrList & opts )
{
	gr_xplot1D(x1,y1,x2,y2,vec,n,(KVStrList&)opts);
}

void gr_plot1D( int x1, int y1, int x2, int y2,
		long vec[], int n, const KVStrList & opts )
{
	gr_xplot1D(x1,y1,x2,y2,vec,n,(KVStrList&)opts);
}

// $$$ to_do: falta manejar smoothclear

void gr_xplot1D( int x1, int y1, int x2, int y2,
		double func(double), double xi, double xf, int n, const KVStrList & opts )
{
#define F(i) func(linterpol(i,0,n-1,xi,xf))
	double min, max;
	int i, clip;
#ifdef SUPPORT_SMOOTHCLEAR
	int smooth;
	int ox=0;
#endif

	gr_bufferon();

	if (!n) n=(x2-x1+1);

	if (opts.contains("max")) max = opts.dval("max");
	else for (i=1, max=F(0); i<n; i++) if (max<F(i)) max=F(i);
	if (opts.contains("min")) min = opts.dval("min");
	else for (i=1, min=F(0); i<n; i++) if (min>F(i)) min=F(i);

	if (opts.contains("bg")) gr_setbkcolor(opts.ival("bg"));
	if (opts.contains("fg")) gr_setcolor(opts.ival("fg"));
#ifdef SUPPORT_SMOOTHCLEAR
	smooth = opts.bbval("smoothclear");
#endif
	if (opts.bbval("clear")) {
#ifdef SUPPORT_SMOOTHCLEAR
		if (!smooth) gr_bar(x1,y1,x2,y2,gr_getbkcolor(),gr_getbkcolor());
#else
		gr_bar(x1,y1,x2,y2,gr_getbkcolor(),gr_getbkcolor());
#endif
	}
#ifdef SUPPORT_SMOOTHCLEAR
	else smooth=0;
#endif
	if (opts.bbval("clip")) clip=1; else clip=0;
	if (opts.bbval("frame")) gr_rectangle(x1,y1,x2,y2);
	if (min==max) return;

	for (i=0; i<n; i++) {
		int x = (int)fround(linterpol(i,0,n-1,x1,x2));
		int y = (int)fround(linterpol(F(i),min,max,y2,y1));
		if (clip) {
			if (y>y2) y=y2;
			if (y<y1) y=y1;
		}

		if (!i) {
#ifdef SUPPORT_SMOOTHCLEAR
			if (smooth) {
				gr_line(x,y1,x,y2,gr_getbkcolor());
				ox=x;
			};
#endif
			gr_moveto(x,y);
		}
		else {
#ifdef SUPPORT_SMOOTHCLEAR
		if (smooth && (x!=ox)) {
			gr_line(x,y1,x,y2,gr_getbkcolor());
			ox=x;
		};
#endif
		gr_lineto(x,y);
		}
	}

	gr_bufferoff();
}

