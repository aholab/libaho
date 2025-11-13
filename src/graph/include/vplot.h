#ifndef __vplot_h__
#define __vplot_h__

#include "strl.hpp"
#include "graph.h"

void gr_plot1D( int x1, int y1, int x2, int y2,
		double vec[], int n, const KVStrList & opts=KVStrList("") );
void gr_plot1D( int x1, int y1, int x2, int y2,
		float vec[], int n, const KVStrList & opts=KVStrList("") );
void gr_plot1D( int x1, int y1, int x2, int y2,
		int vec[], int n, const KVStrList & opts=KVStrList("") );
void gr_plot1D( int x1, int y1, int x2, int y2,
		short vec[], int n, const KVStrList & opts=KVStrList("") );
void gr_plot1D( int x1, int y1, int x2, int y2,
		long vec[], int n, const KVStrList & opts=KVStrList("") );

inline void gr_nplot1D( float x1, float y1, float x2, float y2,
		double vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_nx2x(x1),gr_ny2y(y1),gr_nx2x(x2),gr_ny2y(y2),vec,n,opts); }
inline void gr_nplot1D( float x1, float y1, float x2, float y2,
		float vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_nx2x(x1),gr_ny2y(y1),gr_nx2x(x2),gr_ny2y(y2),vec,n,opts); }
inline void gr_nplot1D( float x1, float y1, float x2, float y2,
		int vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_nx2x(x1),gr_ny2y(y1),gr_nx2x(x2),gr_ny2y(y2),vec,n,opts); }
inline void gr_nplot1D( float x1, float y1, float x2, float y2,
		short vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_nx2x(x1),gr_ny2y(y1),gr_nx2x(x2),gr_ny2y(y2),vec,n,opts); }
inline void gr_nplot1D( float x1, float y1, float x2, float y2,
		long vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_nx2x(x1),gr_ny2y(y1),gr_nx2x(x2),gr_ny2y(y2),vec,n,opts); }



inline void gr_wplot1D( float x1, float y1, float x2, float y2,
		double vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_wx2x(x1),gr_wy2y(y2),gr_wx2x(x2),gr_wy2y(y1),vec,n,opts); }
inline void gr_wplot1D( float x1, float y1, float x2, float y2,
		float vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_wx2x(x1),gr_wy2y(y2),gr_wx2x(x2),gr_wy2y(y1),vec,n,opts); }
inline void gr_wplot1D( float x1, float y1, float x2, float y2,
		int vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_wx2x(x1),gr_wy2y(y2),gr_wx2x(x2),gr_wy2y(y1),vec,n,opts); }
inline void gr_wplot1D( float x1, float y1, float x2, float y2,
		short vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_wx2x(x1),gr_wy2y(y2),gr_wx2x(x2),gr_wy2y(y1),vec,n,opts); }
inline void gr_wplot1D( float x1, float y1, float x2, float y2,
		long vec[], int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_wx2x(x1),gr_wy2y(y2),gr_wx2x(x2),gr_wy2y(y1),vec,n,opts); }


void gr_plot1D( int x1, int y1, int x2, int y2,
		double func(double), double xi, double xf, int n, const KVStrList & opts=KVStrList("") );
inline void gr_nplot1D( float x1, float y1, float x2, float y2,
		double func(double), double xi, double xf, int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_nx2x(x1),gr_ny2y(y1),gr_nx2x(x2),gr_ny2y(y2),func,xi,xf,n,opts); }
inline void gr_wplot1D( float x1, float y1, float x2, float y2,
		double func(double), double xi, double xf, int n, const KVStrList & opts=KVStrList("") )
{ gr_plot1D(gr_wx2x(x1),gr_wy2y(y2),gr_wx2x(x2),gr_wy2y(y1),func,xi,xf,n,opts); }






#endif

