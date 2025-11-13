#ifndef __GRAPH_H__
#define __GRAPH_H__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1997 - Grupo de Voz (DAET) ETSII/IT-Bilbao
& CSTR, University of Edinburgh

Nombre fuente................ graph.h
Nombre paquete............... -
Lenguaje fuente.............. C++
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.1    18/07/97  Borja     CSTR: GR_DISPLAY env. management
1.0.0    18/07/97  Borja     inicial.

======================== Contenido ========================
shell para graficos (bgi, gif, X11)
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

/**********************************************************/

class Graph {
	protected:
		int W, H;
		int cpx, cpy, ccolor, cbkcolor;
		int fround( float f ) { return (int)((f<0) ? f-0.5 : f+0.5); }

		virtual void initgraph( int WW, int HH );
		virtual void closegraph( void ) {}

	public:
		Graph( void ) { W = H = 2; cpx=cpy=ccolor=cbkcolor=0; }
    virtual ~Graph( void ) { closegraph(); }
    
		virtual int getw( void ) { return W; }
		virtual int geth( void ) { return H; }

		float x2nx( int x ) { return (float)x/(float)(W-1); }
		int nx2x( float x ) { return fround(x*(W-1)); }
		float x2wx( int x ) { return (float)x/(W-1)*2.0-1; }
		int wx2x( float x ) { return fround((x+1)*0.5*(W-1)); }

		float y2ny( int y ) { return (float)y/(float)(H-1); }
		int ny2y( float y ) { return fround(y*(H-1)); }
		float y2wy( int y ) { return -(float)y/(H-1)*2.0+1; }
		int wy2y( float y ) { return fround((-y+1)*0.5*(H-1)); }

		virtual void clearviewport( int color=-1 ) { clearviewport(0,0,W-1,H-1,color); };
		virtual void clearviewport( int x1, int y1, int x2, int y2, int color=-1 ) { bar(x1,y1,x2,y2,(color<0)?cbkcolor:color,color); };
		virtual void wclearviewport( float x1, float y1, float x2, float y2, int color=-1 ) { clearviewport(wx2x(x1),wy2y(y1),wx2x(x2),wy2y(y2),color); };
		virtual void nclearviewport( float x1, float y1, float x2, float y2, int color=-1 ) { clearviewport(nx2x(x1),ny2y(y1),nx2x(x2),ny2y(y2),color); };

		virtual void setcolor( int color ) { ccolor = color; };
		virtual void setbkcolor( int bkcolor ) { cbkcolor= bkcolor; };
		virtual int getcolor( void ) { return ccolor; };
		virtual int getbkcolor( void ) { return cbkcolor; };

		virtual int getncolors( void ) { return 0; };
		virtual void setcolorrgb( int /*color*/, int /*r*/, int /*g*/, int /*b*/ ) {};
		virtual void nsetcolorrgb( int color, double r, double g, double b ) { setcolorrgb(color,int(r*255),int(g*255),int(b*255)); };
    virtual void setdefpalette( void );

		virtual void putpixel( int /*x*/, int /*y*/, int /*color*/=-1 ) {};
		virtual void wputpixel( float x, float y, int color=-1 ) { putpixel(wx2x(x),wy2y(y),color); };
		virtual void nutpixel( float x, float y, int color=-1 ) { putpixel(nx2x(x),ny2y(y),color); };

		virtual void line( int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/, int /*color*/=-1 ) {};
		virtual void wline( float x1, float y1, float x2, float y2, int color=-1 ) { line(wx2x(x1),wy2y(y1),wx2x(x2),wy2y(y2),color); };
    virtual void nline( float x1, float y1, float x2, float y2, int color=-1 ) { line(nx2x(x1),ny2y(y1),nx2x(x2),ny2y(y2), color); };
    
		virtual void lineto( int x, int y, int color=-1 ) { line(cpx,cpy,x,y,color); cpx=x; cpy=y; };
		virtual void wlineto( float x, float y, int color=-1 ) { lineto(wx2x(x),wy2y(y),color); };
		virtual void nlineto( float x, float y, int color=-1 ) { lineto(nx2x(x),ny2y(y),color); };

		virtual void linerel( int dx, int dy, int color=-1 ) { line(cpx,cpy,cpx+dx,cpy+dy,color); cpx+=dx; cpy+=dy; };
		virtual void wlinerel( float dx, float dy, int color=-1 ) { linerel(wx2x(dx),wy2y(dy),color); };
		virtual void nlinerel( float dx, float dy, int color=-1 ) { linerel(nx2x(dx),ny2y(dy),color); };

		virtual void moveto( int x, int y ) { cpx=x; cpy=y; };
		virtual void wmoveto( float x, float y ) { moveto(wx2x(x),wy2y(y)); };
		virtual void nmoveto( float x, float y ) { moveto(nx2x(x),ny2y(y)); };

		virtual void moverel( int dx, int dy ) { cpx+=dx; cpy+=dy; };
		virtual void wmoverel( float dx, float dy ) { wmoverel(wx2x(dx),wy2y(dy)); };
		virtual void nmoverel( float dx, float dy ) { wmoverel(nx2x(dx),ny2y(dy)); };

		virtual int getx( void ) { return cpx; };
		virtual float wgetx( void ) { return x2wx(getx()); };
		virtual float ngetx( void ) { return x2nx(getx()); };

		virtual int gety( void ) { return cpy; };
		virtual float wgety( void ) { return y2wy(gety()); };
		virtual float ngety( void ) { return y2ny(gety()); };

		virtual void rectangle( int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/, int /*color*/=-1 ) {};
		virtual void wrectangle( float x1, float y1, float x2, float y2, int color=-1 ) { rectangle(wx2x(x1),wy2y(y1),wx2x(x2),wy2y(y2),color); };
    virtual void nrectangle( float x1, float y1, float x2, float y2, int color=-1 ) { rectangle(nx2x(x1),ny2y(y1),nx2x(x2),ny2y(y2),color); };
    
		virtual void bar( int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/, int /*color*/=-1, int /*bkcolor*/=-1 ) {};
		virtual void wbar( float x1, float y1, float x2, float y2, int color=-1, int bkcolor=-1 ) { bar(wx2x(x1),wy2y(y1),wx2x(x2),wy2y(y2),color,bkcolor); };
		virtual void nbar( float x1, float y1, float x2, float y2, int color=-1, int bkcolor=-1 ) { bar(nx2x(x1),ny2y(y1),nx2x(x2),ny2y(y2),color,bkcolor); };

		virtual void ellipse( int /*x*/, int /*y*/, int /*rx*/, int /*ry*/, int /*color*/=-1 ) {};
		virtual void wellipse( float x, float y, float rx, float ry, int color=-1 ) { ellipse(wx2x(x),wy2y(y),wx2x(rx)-wx2x(0),wy2y(0)-wy2y(ry),color); };
		virtual void nellipse( float x, float y, float ry, float rx, int color=-1 ) { ellipse(nx2x(x),ny2y(y),nx2x(rx)-nx2x(0),ny2y(ry)-ny2y(0),color); };

		virtual void circle( int x, int y, int r, int color=-1 ) { ellipse(x,y,r,r,color); };
		virtual void wcircle( float x, float y, float r, int color=-1 ) { wellipse(x,y,r,r,color); };
		virtual void ncircle( float x, float y, float r, int color=-1 ) { nellipse(x,y,r,r,color); };

		virtual void bufferon( void ) {};
		virtual void bufferflush( void ) {};
		virtual void bufferoff( int /*reset*/=0 ) {};

		virtual void setfile( const char */*dest*/ ) {};
    virtual const char* getfile( void ) { return 0; };
    virtual void savefile( const char */*dest*/=0 ) {};

		virtual int kbhit( void ) { return 0; };
		virtual int getkey( void ) { return 0; };

   	friend Graph *newGraph( const char * mode, int W, int H );
    friend Graph *newGraph( int W, int H, const char *mode );
    friend void deleteGraph( Graph *gr );

};


Graph *newGraph( const char *mode="Auto", int W=0, int H=0 );
inline Graph *newGraph( int W, int H, const char *mode="Auto" ) { return newGraph(mode,W,H); }
void deleteGraph( Graph *gr );

/**********************************************************/

enum colors {
	GR_BLACK=0,  GR_BLUE,         GR_GREEN,      GR_CYAN,
	GR_RED,      GR_MAGENTA,      GR_BROWN,      GR_LIGHTGRAY,
	GR_DARKGRAY, GR_LIGHTBLUE,    GR_LIGHTGREEN, GR_LIGHTCYAN,
	GR_LIGHTRED, GR_LIGHTMAGENTA, GR_YELLOW,     GR_WHITE
};

/**********************************************************/

extern Graph *_graph_gr;

void gr_initgraph( const char *mode="Auto", int W=0, int H=0 );
inline void gr_initgraph( int W, int H, const char *mode="Auto" ) { gr_initgraph(mode,W,H); }
void gr_closegraph( void );

#define gr_getw _graph_gr->getw
#define gr_geth _graph_gr->geth

#define gr_x2nx _graph_gr->x2nx
#define gr_nx2x _graph_gr->nx2x
#define gr_x2wx _graph_gr->x2wx
#define gr_wx2x _graph_gr->wx2x

#define gr_y2ny _graph_gr->y2ny
#define gr_ny2y _graph_gr->ny2y
#define gr_y2wy _graph_gr->y2wy
#define gr_wy2y _graph_gr->wy2y

#define gr_clearviewport _graph_gr->clearviewport
#define gr_wclearviewport _graph_gr->wclearviewport
#define gr_nclearviewport _graph_gr->nclearviewport

#define gr_setcolor _graph_gr->setcolor
#define gr_setbkcolor _graph_gr->setbkcolor
#define gr_getcolor _graph_gr->getcolor
#define gr_getbkcolor _graph_gr->getbkcolor

#define gr_getncolors _graph_gr->getncolors
#define gr_setcolorrgb _graph_gr->setcolorrgb
#define gr_nsetcolorrgb _graph_gr->nsetcolorrgb
#define gr_setdefpalette _graph_gr->setdefpalette

#define gr_putpixel _graph_gr->putpixel
#define gr_wputpixel _graph_gr->wputpixel
#define gr_nputpixel _graph_gr->nputpixel

#define gr_line _graph_gr->line
#define gr_wline _graph_gr->wline
#define gr_nline _graph_gr->nline

#define gr_lineto _graph_gr->lineto
#define gr_wlineto _graph_gr->wlineto
#define gr_nlineto _graph_gr->nlineto

#define gr_linerel _graph_gr->linerel
#define gr_wlinerel _graph_gr->wlinerel
#define gr_nlinerel _graph_gr->nlinerel

#define gr_moveto _graph_gr->moveto
#define gr_wmoveto _graph_gr->wmoveto
#define gr_nmoveto _graph_gr->nmoveto

#define gr_moverel _graph_gr->moverel
#define gr_wmoverel _graph_gr->wmoverel
#define gr_nmoverel _graph_gr->nmoverel

#define gr_getx _graph_gr->getx
#define gr_wgetx _graph_gr->wgetx
#define gr_ngetx _graph_gr->ngetx

#define gr_gety _graph_gr->gety
#define gr_wgety _graph_gr->wgety
#define gr_ngety _graph_gr->ngety

#define gr_rectangle _graph_gr->rectangle
#define gr_wrectangle _graph_gr->wrectangle
#define gr_nrectangle _graph_gr->nrectangle

#define gr_bar _graph_gr->bar
#define gr_wbar _graph_gr->wbar
#define gr_nbar _graph_gr->nbar

#define gr_ellipse _graph_gr->ellipse
#define gr_wellipse _graph_gr->wellipse
#define gr_nellipse _graph_gr->nellipse

#define gr_circle _graph_gr->circle
#define gr_wcircle _graph_gr->wcircle
#define gr_ncircle _graph_gr->ncircle

#define gr_bufferon _graph_gr->bufferon
#define gr_bufferflush _graph_gr->bufferflush
#define gr_bufferoff _graph_gr->bufferoff

#define gr_setfile _graph_gr->setfile
#define gr_getfile _graph_gr->getfile
#define gr_savefile _graph_gr->savefile

#define gr_kbhit _graph_gr->kbhit
#define gr_getkey _graph_gr->getkey

/**********************************************************/

#endif
