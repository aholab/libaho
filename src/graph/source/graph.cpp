/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1997 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ graph.cpp
Nombre paquete............... -
Lenguaje fuente.............. C++
Estado....................... Utilizable
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.1    18/09/97  Borja     bug en initgraph
1.0.0    18/07/97  Borja     inicial.

======================== Contenido ========================
Shell graficos  (BGI, GIF, X11). Funciones graficas
simplificadas para MSDOS (BGI), Unix (X11) y ficheros (GIF, 
solo en Unix).

gr_initgraph() para inicializar graficos. Algunos ejemplos:

gr_initgraph()
gr_initgraph("X11")
gr_initgraph("GIF")
gr_initgraph("BGI")
gr_initgraph(300,200)
gr_initgraph("pepe.gif",600,500)
gr_initgraph("Auto",200,300)

Si no se especifica dispositivo (GIF, BGI, X11, fichero.gif) o 
se pone "Auto", se utiliza la variable de entorno GR_DISPLAY
que puede contener por ejemplo "GIF W=640 H=480" (no importa
el orden de los campos).  

El dispositivo GIF usa un fichero gif por defecto (screen.gif).

Para terminar los graficos, usar gr_closegraph().

Las funciones para pintar funcionan de tres formas:

* coordenadas en pixeles (ej. line): 0 a gr_getw() de izquierda
a derecha y  0 a gr_geth() de arriba a abajo.
* coordenadas normalizadas (ej. nline): como anterior pero
entre 0 y 1.
* coordenadas de mundo (world) (ej. wline): estas van entre
-1 y 1, y en vertical van de abajo a arriba.

Hay funciones para convertir de unas a otras si hace falta (wx2x(), etc).

Colores:  setcolorrgb usa componentes RGB entre 0 y 255, mientras que
nsetcolorrgb usa RGB entre 0 y 1.  Se definen constantes (ej. GR_RED)
para los primeros 16 colores, que se inicializan a la paleta estandar. 
Si los cambias, los nombres de las constantes ya no encajaran.

Todas las funciones graficas son bastante evidentes de usar. Normalmente 
el color es un parametro opcional en todas (si no se indica, se usa el valor 
configurado con setcolor o setbkcolor).

Manejo de buffer. Solo tiene efecto en X11 (aunque se puede usar con
cualquier dispositivo). Cuando se pintan muchas cosas, es mejor activar el 
buffer con bufferon(). al terminar, bufferoff(). Asi va muuuucho mas 
rapido pintando. Puedes enviar lo que esta bufferizado a pantalla con
bufferflush() (bufferoff() ya lo hace, y cuando pides una tecla
con getkey() tambien).

Teclado:  kbhit() indica si hay tecla, no se bloquea. getkey() devuelve 
tecla, bloqueandose hasta que la haya.


Fichero: (solo chuta con los gifs). Con setfile() configuras un nuevo fichero.
Con savefile() salvas lo que tengas pintado a fichero (puedes indicar un
fichero, o se usa el que este previamente configurado). Con getfile() obtienes
el nombre del fichero preconfigurado.


Nota: realmente soporta muchos dispositivos graficos abiertos
simultaneamente (objetos Graph), aunque solo se documentan
las funciones gr_??? para una unica grafica, que es lo mas 
tipico.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.h"
#include "graph.h"
#include "uti.h"

/**********************************************************/

#ifdef __OS_UNIX__
//#define X11
#define GIF
#else
#define BGI
#endif

/**********************************************************/

#define DEFGIF "screen.gif"
#define DEFW 640
#define DEFH 480

/**********************************************************/

void Graph::initgraph( int WW, int HH )
{
	W=WW; H=HH;
	setdefpalette();
	setcolor(GR_WHITE);
	setbkcolor(GR_BLACK);
	moveto(0,0);
	clearviewport();
}

void Graph::setdefpalette( void )
{
	static int c[16][3] = {
		{  0,  0,  0}, {  0,  0,160}, {  0,167,  0}, {  0,167,160},
		{160, 0,   0}, {160,  0,160}, {160, 80,  0}, {160,167,160},
		{ 80, 80, 80}, { 80, 80,255}, { 80,248, 80}, { 80,248,255},
		{255, 80, 80}, {255, 80,255}, {255,248, 80}, {255,255,255}
	};
	int i;

	for (i=0; i<16; i++) setcolorrgb(i,c[i][0],c[i][1],c[i][2]);
}

/**********************************************************/

#ifdef X11

#include "vogle.h"
#include <math.h>

class Vogle: public Graph {
	private:
		float x2vx( int x ) { return (float)x/(float)W*2. - 1.; };
		float y2vy( int y ) { return 1. - (float)y/(float)H*2.; };
		int vx2x( float x ) { return (int)((x+1)*0.5*W+0.5); };
		int vy2y( float y ) { return (int)((1-y)*0.5*H+0.5); };
		int flushstat;
	protected:
		void initgraph( int W, int H );
		void closegraph( void );
	public:
		Vogle() { };

		void setcolor( int color );
		int getncolors( void );
		void setcolorrgb( int color, int r, int g, int b );

		void putpixel( int x, int y, int color=-1 );
		void line( int x1, int y1, int x2, int y2, int color=-1 );
		void rectangle( int x1, int y1, int x2, int y2, int color=-1 );
		void bar( int x1, int y1, int x2, int y2, int color=-1, int bkcolor=-1 );

		void ellipse( int x, int y, int rx, int ry, int color=-1 );

		void bufferon( void );
		void bufferflush( void );
		void bufferoff( int reset=0 );

		int kbhit( void );
		int getkey( void );
};

void Vogle::initgraph( int WW, int HH )
{
	W = WW; if (W<2) W=2;
	H = HH; if (H<2) H=2;

	prefsize(W+16,H+16);
	vinit("X11");

	float w, h;
	getdisplaysize(&w,&h);
	if ((w!=W)||(h!=H)) {
		vexit();
		prefsize(W+16+(W-w),H+16+(H-h));
		vinit("X11");
	}

	expandviewport();
	viewport(-1.,1., -1.,1.);
	frontbuffer();
	font("small");
	flushstat=0;
	polyfill(0);
	Graph::initgraph(W,H);  // dont forget call to base class!
}

void Vogle::closegraph( void ) { vexit(); }

void Vogle::setcolor( int col ) { color(col); Graph::setcolor(col); };
int Vogle::getncolors( void ) { return 1 << getdepth(); };
void Vogle::setcolorrgb( int color, int r, int g, int b ) { mapcolor(color,r,g,b); };

void Vogle::putpixel( int x, int y, int col )
{
	if ((col>=0) && (col!=ccolor)) {
		color(col);
		point2(x2vx(x),y2vy(y));
		color(ccolor);
	}
	else
		point2(x2vx(x),y2vy(y));
}

void Vogle::line( int x1, int y1, int x2, int y2, int col )
{
	if ((col>=0) && (col!=ccolor)) {
		color(col);
		move2(x2vx(x1),y2vy(y1));
		draw2(x2vx(x2),y2vy(y2));
		color(ccolor);
	}
	else {
		move2(x2vx(x1),y2vy(y1));
		draw2(x2vx(x2),y2vy(y2));
	}
}

void Vogle::ellipse( int x, int y, int rx, int ry, int col )
{
#define NSEC 32
	float fx, fy, frx, fry;
	int i;

	color((col<0)?ccolor:col);
	fx = x2vx(x);
	fy = y2vy(y);
	frx =  x2vx(rx)-x2vx(0);
	fry =  y2vy(ry)-y2vy(0);
	bufferon();
	move2(fx+frx,fy);
	for (i=1; i<=NSEC; i++) {
		float a=((2.*M_PI)/NSEC)*i;
		draw2(fx+frx*cos(a),fy+fry*sin(a));
	}
	bufferoff();
	color(ccolor);
#undef NSEC
}

void Vogle::rectangle( int x1, int y1, int x2, int y2, int col )
{
	color((col<0)?ccolor:col);
	rect(x2vx(x1),y2vy(y1),x2vx(x2),y2vy(y2));
	color(ccolor);
}

void Vogle::bar( int x1, int y1, int x2, int y2, int col, int bkcol )
{
	float xx1, yy1, xx2, yy2;
	color((bkcol<0)?cbkcolor:bkcol);
	polyfill(1);
	rect(xx1=x2vx(x1),yy1=y2vy(y1),xx2=x2vx(x2),yy2=y2vy(y2));
	polyfill(0);
	color((col<0)?ccolor:col);
	rect(xx1,yy1,xx2,yy2);
	color(ccolor);
}

void Vogle::bufferon( void ) { if (!flushstat) vsetflush(0); flushstat++; }
void Vogle::bufferflush( void ) { vflush(0); }
void Vogle::bufferoff( int reset ) {
	if (flushstat) flushstat--;
	if (reset) flushstat=0;
	if (!flushstat) { vflush(0); vsetflush(1); }
}
int Vogle::kbhit( void ) {	return checkkey(); };
int Vogle::getkey( void ) {	vflush(0); int k = ::getkey(); return k; };//(k<0)?0:k; };

#endif

/**********************************************************/

#ifdef GIF

#include "gd.h"

class Gif: public Graph {
	private:
		gdImagePtr im;
		char *dest;
	protected:
		void initgraph( int W, int H );
		void closegraph( void );
	public:
		Gif( char *name=0 ) { if (name) dest=strdup(name); else dest=NULL; im=0; };
		~Gif( void ) { closegraph(); setfile(); }

		int getncolors( void );
		void setcolorrgb( int color, int r, int g, int b );

		void putpixel( int x, int y, int color=-1 );
		void line( int x1, int y1, int x2, int y2, int color=-1 );
		void rectangle( int x1, int y1, int x2, int y2, int color=-1 );
		void bar( int x1, int y1, int x2, int y2, int color=-1, int bkcolor=-1 );

		void ellipse( int x, int y, int rx, int ry, int color=-1 );

		void setfile( const char *dest=NULL );
		const char * getfile( void );
		void savefile( const char *dest=NULL );

		int getkey( void );
};

void Gif::initgraph( int WW, int HH )
{
	int i;

	fprintf(stderr,"Rendering GIF image.\n");
	W = WW; if (W<2) W=2;
	H = HH; if (H<2) H=2;

	im = gdImageCreate(WW,HH);
	gdImageInterlace(im,0);

	for (i=0; i<256; i++)
		gdImageColorAllocate(im,i,i,i);

	Graph::initgraph(W,H);  // dont forget call to base class!
}

void Gif::closegraph( void )
{
  if (!im) return;
	savefile();
	gdImageDestroy(im);
  im=0;
	fprintf(stderr,"GIF image closed.\n");
}

int Gif::getncolors( void ) { return 256; };

void Gif::setcolorrgb( int color, int r, int g, int b )
{
	gdImageColorDeallocate(im,color);
	gdImageColorAllocate(im,r,g,b);
};

void Gif::putpixel( int x, int y, int col )
{
	if (col>=0) gdImageSetPixel(im,x,y,col);
	else gdImageSetPixel(im,x,y,ccolor);
}

void Gif::line( int x1, int y1, int x2, int y2, int col )
{
	if (col>=0) gdImageLine(im,x1,y1,x2,y2,col);
	else gdImageLine(im,x1,y1,x2,y2,ccolor);
}

void Gif::rectangle( int x1, int y1, int x2, int y2, int col )
{
	if (col>=0) 		gdImageRectangle(im,x1,y1,x2,y2,col);
	else 		gdImageRectangle(im,x1,y1,x2,y2,ccolor);
}

void Gif::bar( int x1, int y1, int x2, int y2, int col, int bkcol )
{
	if (bkcol>=0) gdImageFilledRectangle(im,x1,y1,x2,y2,bkcol);
	else gdImageFilledRectangle(im,x1,y1,x2,y2,cbkcolor);

	if (col>=0) gdImageRectangle(im,x1,y1,x2,y2,col);
	else gdImageRectangle(im,x1,y1,x2,y2,ccolor);
}

void Gif::ellipse( int x, int y, int rx, int ry, int col )
{
	if (col>=0) gdImageArc(im,x,y,rx,ry,0,360,col);
	else gdImageArc(im,x,y,rx,ry,0,360,ccolor);
}

int Gif::getkey( void )
{
	fprintf(stderr,"Current image (%s) being saved...",getfile());
	savefile();
	fprintf(stderr,"OK!, press KEY and RETURN to continue...");
	return getchar();
};

void Gif::setfile( const char *name )
{
	if (dest) free(dest);
	dest = NULL;
	if (name) dest = strdup(name);
}

void Gif::savefile( const char *name )
{
	if (name) setfile(name);

	FILE * out = fopen((dest?dest:DEFGIF),"wb");
	gdImageGif(im, out);
	fclose(out);
}

const char *Gif::getfile( void )
{
	if (dest) return dest;
	return DEFGIF;
}

#endif

/**********************************************************/

#ifdef BGI

#include <graphics.h>
#include <bgi.h>
#include <conio.h>

class Bgi: public Graph {
	private:
	protected:
		void initgraph( int W, int H );
		void closegraph( void );
	public:
		Bgi() { };
		void clearviewport( int color=-1 ) { Graph::clearviewport(color); }
		void clearviewport( int x1, int y1, int x2, int y2, int color=-1 );

		void setcolor( int color );
		int getncolors( void );
		void setcolorrgb( int color, int r, int g, int b );

		void putpixel( int x, int y, int color=-1 );
		void line( int x1, int y1, int x2, int y2, int color=-1 );
		void rectangle( int x1, int y1, int x2, int y2, int color=-1 );
		void bar( int x1, int y1, int x2, int y2, int color=-1, int bkcolor=-1 );
		void circle( int x, int y, int r, int color=-1 );
		void ellipse( int x, int y, int rx, int ry, int color=-1 );

		int kbhit( void );
		int getkey( void );
};

void Bgi::initgraph( int WW, int HH )
{
	int i;
	int gd = VGA;
	int gm = VGAHI;

	register_VGA();
	::initgraph(&gd,&gm,"");

	W = ::getmaxx()+1;
	H = ::getmaxy()+1;
	if (W>WW) W=WW;
	if (H>HH) H=HH;

	for (i=0; i<16; i++) ::setpalette(i,i);
	
	Graph::initgraph(W,H);  // dont forget call to base class!
}

void Bgi::closegraph( void ) { ::closegraph(); }

void Bgi::clearviewport( int x1, int y1, int x2, int y2, int color )
{
	if (color>=0)	::setbkcolor(color);
	else ::setbkcolor(cbkcolor);

	::setviewport(x1,y1,x2,y2,1);
	::clearviewport();
}

void Bgi::setcolor( int col ) { ::setcolor(col); Graph::setcolor(col); };
int Bgi::getncolors( void ) { return 16; };

void Bgi::setcolorrgb( int color, int r, int g, int b )
{
	r=(r+2)>>2;
	g=(g+2)>>2;
	b=(b+2)>>2;
	r=(r<0?0:r);
	g=(g<0?0:g);
	b=(b<0?0:b);
	r=(r>63?63:r);
	g=(g>63?63:g);
	b=(b>63?63:b);

	setrgbpalette(color,r,g,b);
};

void Bgi::putpixel( int x, int y, int col )
{
	if (col>=0) ::putpixel(x,y,col);
	else ::putpixel(x,y,ccolor);
}

void Bgi::line( int x1, int y1, int x2, int y2, int col )
{
	if ((col>=0) && (col!=ccolor)) {
		::setcolor(col);
		::line(x1,y1,x2,y2);
		::setcolor(ccolor);
	}
	else
		::line(x1,y1,x2,y2);
}

void Bgi::circle( int x, int y, int r, int col )
{
	if ((col>=0) && (col!=ccolor)) {
		::setcolor(col);
		::circle(x,y,r);
		::setcolor(ccolor);
	}
	else
		::circle(x,y,r);
}

void Bgi::ellipse( int x, int y, int rx, int ry, int col )
{
	if ((col>=0) && (col!=ccolor)) {
		::setcolor(col);
		::ellipse(x,y,0,360,rx,ry);
		::setcolor(ccolor);
	}
	else
		::ellipse(x,y,0,360,rx,ry);
}

void Bgi::rectangle( int x1, int y1, int x2, int y2, int col )
{
	if ((col>=0) && (col!=ccolor)) {
		::setcolor(col);
		::rectangle(x1,y1,x2,y2);
		::setcolor(ccolor);
	}
	else
		::rectangle(x1,y1,x2,y2);
}

void Bgi::bar( int x1, int y1, int x2, int y2, int col, int bkcol )
{
	if (bkcol>=0) ::setfillstyle(SOLID_FILL,bkcol);
	else ::setfillstyle(SOLID_FILL,cbkcolor);
	::bar(x1,y1,x2,y2);

	if ((col>=0) && (col!=ccolor)) {
		::setcolor(col);
		::rectangle(x1,y1,x2,y2);
		::setcolor(ccolor);
	}
	else
		::rectangle(x1,y1,x2,y2);
}

int Bgi::getkey( void ) { return ::getch(); }
int Bgi::kbhit( void ) { return ::kbhit(); }

#endif

/**********************************************************/

class NoGraph: public Graph {
	private:
		void Msg(VOID) { die_beep("Graphics not initialized"); }
	protected:
		void closegraph( void ) { Msg(); }
	public:
		NoGraph() { };
		void setcolorrgb( int, int, int, int) { Msg(); }
		void putpixel( int, int, int ) { Msg(); }
		void line( int, int, int, int, int ) { Msg(); }
		void rectangle( int, int, int, int, int ) { Msg(); }
		void bar( int, int, int, int, int, int ) { Msg(); }
		void circle( int, int, int, int ) { Msg(); }
		void ellipse( int, int, int, int, int ) { Msg(); }

		int kbhit( void ) { Msg(); return 0; }
		int getkey( void ) { Msg(); return 0; }
};

/**********************************************************/

Graph *newGraph( const char *mode, int W, int H )
{
	Graph *gr = NULL;
	char *dev = NULL;

	if (getenv("GR_DISPLAY")) {
		char *env=strdup(getenv("GR_DISPLAY"));
		char *s=strtok(env," \t\n");
		while (s) {
			if ((s[0]=='W')&&(s[1]=='=')) { if (!W) W=atoi(s+2); }
			else if ((s[0]=='H')&&(s[1]=='=')) { if (!H) H=atoi(s+2); }
			else { if (dev) free(dev); dev=strdup(s); }
			s=strtok(NULL," \t\n");	
		}
		free(env);
		if (mode && (strcmp(mode,"Auto"))) { if (dev) free(dev); dev=strdup(mode); };
	}
	else if (mode) dev=strdup(mode);

	if (!dev || (!strcmp(dev,"Auto"))) {
#ifdef X11
		if (!gr) gr = new Vogle;
#endif
#ifdef GIF
		if (!gr) gr = new Gif();
#endif
#ifdef BGI
		if (!gr) gr = new Bgi();
#endif
	}

#ifdef X11
	else if (!strcmp(dev,"X11"))
		gr = new Vogle;
#endif
#ifdef GIF
	else if (!strcmp(dev,"GIF"))
		gr = new Gif();
	else if (stristr(dev,".gif"))
		gr = new Gif(dev);
#endif
#ifdef BGI
	else if (!strcmp(dev,"BGI"))
		gr = new Bgi();
#endif

	if (!gr) {
		fprintf(stderr,"Invalid graphic mode (%s)\7\n",dev);
		exit(1);
	}

	if (!H) H=DEFH;
	if (!W) W=DEFW;

	gr->initgraph(W,H);

	return gr;
}

void deleteGraph( Graph *gr )
{
	if (gr) {
		gr->closegraph();
		delete gr;
	};
}

/**********************************************************/

static NoGraph nograph;
Graph *_graph_gr= &nograph;

void gr_initgraph( const char *mode, int W, int H )
{
	gr_closegraph();
	_graph_gr=newGraph(mode,W,H);
}

void gr_closegraph( void ) 
{
	if (_graph_gr!= &nograph)
		deleteGraph(_graph_gr);
	_graph_gr= &nograph;
}

/**********************************************************/

