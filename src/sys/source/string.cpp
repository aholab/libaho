/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1998 - Aholab

Nombre fuente................ STRING.HPP
Nombre paquete............... -
Lenguaje fuente.............. C++
Estado....................... -
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Comentario
-------  --------  --------  ----------
1.0.0    17/09/98  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Clase String simplificada, codificada desde 0.

Contiene lo minimo utilizado por otros modulos Aholab.
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <string.h>
#include <ctype.h>
#include "xalloc.h"
#include "string.hpp"

/**********************************************************/

String::String()
{
	buf=xstrdup("");
}

/**********************************************************/

String::String(const String& x)
{
	buf=xstrdup(x);
}

/**********************************************************/

String::String(const char* t)
{
	buf=xstrdup(t);
}

/**********************************************************/

String::String(const char* t, int len)
{
	int l=strlen(t);
	if (l>len) l=len;
	buf=(char*)xmalloc(l+1);
	strncpy(buf,t,l);
	buf[l]='\0';
}

/**********************************************************/

String::String(char c)
{
	buf=(char*)xmalloc(2);
	buf[0]=c;
}

/**********************************************************/

String::~String()
{
	if (buf) xfree(buf);
}

/**********************************************************/

String& String::operator = (const String& y)
{
	char *tmp=xstrdup(y.buf);
	if (buf) xfree(buf);
	buf=tmp;
	return *this;
}

/**********************************************************/

String& String::operator = (const char* y)
{
	char *tmp=xstrdup(y);
	if (buf) xfree(buf);
	buf=tmp;
	return *this;
}

/**********************************************************/

String& String::operator = (char c)
{
	char *tmp=(char*)xmalloc(2);
	tmp[0]=c;
	tmp[1]='\0';
	if (buf) xfree(buf);
	buf=tmp;
	return *this;
}

/**********************************************************/

String& String::operator += (const String& y)
{
	return operator +=((const char*)y);
}

/**********************************************************/

String& String::operator += (const char* t)
{
	char *tmp;
	tmp=(char*)xmalloc((buf?strlen(buf):0)+(t?strlen(t):0)+1);
	tmp[0]='\0';
	if (buf) { strcat(tmp,buf); xfree(buf); }
	if (t) strcat(tmp,t);
	buf=tmp;
	return *this;
}

/**********************************************************/

String& String::operator += (char c)
{
	char *tmp;
	tmp=(char*)xmalloc((buf?strlen(buf):0)+2);
	tmp[0]='\0';
	if (buf) { strcat(tmp,buf); xfree(buf); }
	size_t l=strlen(tmp);
	tmp[l+1]='\0';
	tmp[l]=c;
	buf=tmp;
	return *this;
}

/**********************************************************/

char& String::operator [] (int i)
{
	return buf[i];
}

/**********************************************************/

const char& String::operator [] (int i) const
{
	return buf[i];
}

/**********************************************************/

String::operator const char*() const
{
	return buf;
}

/**********************************************************/

const char* String::chars() const
{
	return buf;
}

/**********************************************************/

unsigned int String::length() const
{
	return strlen(buf);
}

/**********************************************************/

int String::empty() const
{
	return (buf[0]=='\0');
}

/**********************************************************/

int String::OK() const
{
	return buf!=NULL;
}

/**********************************************************/

int String::index(char c, int pos) const
{
	const char *x=strchr(buf+pos,c);
	if (x) return (int)(buf-(char*)x);
	return -1;
}

/**********************************************************/

int String::index(const String& y, int pos) const
{
	return index(y.buf,pos);
}

/**********************************************************/

int String::index(const char* t, int pos) const
{
	const char *x=strstr(buf+pos,t);
	if (x) return (int)(buf-(char*)x);
	return -1;
}

/**********************************************************/

int String::contains(char c) const { return strchr(buf,c)!=NULL; }

/**********************************************************/

int String::contains(const String& y) const { return strstr(buf,y.buf)!=NULL; }

/**********************************************************/

int String::contains(const char* t) const { return strstr(buf,t)!=NULL; }

/**********************************************************/

int String::contains(char c, int pos) const
{
	if (pos>=0) return strchr(buf+pos,c)!=NULL;
	char *x=strchr(buf,c);
	if (!x) return 0;
	return (buf-x <= pos);
}

/**********************************************************/

int String::contains(const String& y, int pos) const
{
	return contains(y.buf,pos);
}

/**********************************************************/

int String::contains(const char* t, int pos) const
{
	if (pos>=0) return strstr(buf+pos,t)!=NULL;
	char *x=strstr(buf,t);
	if (!x) return 0;
	return (buf-x <= pos);
}

/**********************************************************/

String operator + (const String& x, const String& y) { String r(x); r+=y; return r; }
String operator + (const String& x, const char* y) { String r(x); r+=y; return r; }
String operator + (const String& x, char y) { String r(x); r+=y; return r; }
String operator + (const char* x, const String& y) { String r(x); r+=y; return r; }


int compare(const String& x, const String& y) { return strcmp(x,y); }
int compare(const String& x, const char* y) { return strcmp(x,y); }

int operator==(const String& x, const String& y) { return compare(x, y) == 0; }
int operator!=(const String& x, const String& y) { return compare(x, y) != 0; }
int operator>(const String& x, const String& y) { return compare(x, y) > 0; }
int operator>=(const String& x, const String& y) { return compare(x, y) >= 0; }
int operator<(const String& x, const String& y) { return compare(x, y) < 0; }
int operator<=(const String& x, const String& y) { return compare(x, y) <= 0; }
int operator==(const String& x, const char* t) { return compare(x, t) == 0; }
int operator!=(const String& x, const char* t) { return compare(x, t) != 0; }
int operator>(const String& x, const char* t) { return compare(x, t) > 0; }
int operator>=(const String& x, const char* t) { return compare(x, t) >= 0; }
int operator<(const String& x, const char* t) { return compare(x, t) < 0; }
int operator<=(const String& x, const char* t) { return compare(x, t) <= 0; }

/**********************************************************/

void String::upcase()
{
	char *s=buf;
	while (*s)
	{
		*s=toupper(*s);
		s++;
	}
}

/**********************************************************/

String upcase(const String& x) { String s(x); s.upcase(); return s; }

/**********************************************************/

int String::gsub(const char* pat, const char* repl)
{
	size_t l=strlen(pat);
	char *from=buf;
	int n=0;
	char *s;
	String dest;

	while ( (s=strstr(from,pat))!=NULL ) {
		n++;
		String tmp(from,(int)(s-from));
		dest += tmp+repl;
		from = s+l;
	}
	dest += from;
	xfree(buf);
	buf=dest.buf;
	dest.buf=NULL;
	return n;
}

/**********************************************************/
String String::before(int pos)
{
	if (pos<0) return String(buf);
	if (pos>0) return	String(buf,pos-1);
	else return String();
}

String String::before(const String& x, int startpos) { return before(index(x,startpos)); }
String String::before(const char* t, int startpos) { return before(index(t,startpos)); }
String String::before(char c, int startpos) { return before(index(c,startpos)); }
String String::after(int pos)
{
	if (pos<0) return String(buf);
	String tmp(buf+pos+1);
	return tmp;
}

String String::after(const String& x, int startpos)  { return after(index(x,startpos)); }
String String::after(const char* t, int startpos) { return after(index(t,startpos)); }
String String::after(char c, int startpos) { return after(index(c,startpos)); }

/**********************************************************/
