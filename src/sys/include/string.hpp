#ifndef __STRING_HPP__
#define __STRING_HPP__

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


#include <iostream>
#include <string.h>

class String {
private:
	char *buf;
protected:
public:
	String();
	String(const String& x);
	String(const char* t);
	String(const char* t, int len);
	String(char c);

	~String();

	String& operator = (const String& y);
	String& operator = (const char* y);
	String& operator = (char c);

	String& operator += (const String& y);
	String& operator += (const char* t);
	String& operator += (char c);

	char& operator [] (int i);
	const char& operator [] (int i) const;

	operator const char*() const;
	const char* chars() const;

	unsigned int length() const;
	int empty() const;

	int index(char c, int startpos = 0) const;
	int index(const String& y, int startpos = 0) const;
	int index(const char* t, int startpos = 0) const;

	int contains(char c) const;
	int contains(const String& y) const;
	int contains(const char* t) const;

	int contains(char c, int pos) const;
	int contains(const String& y, int pos) const;
	int contains(const char* t, int pos) const;

	void upcase();
	friend String upcase(const String& x);

	int gsub(const char* pat, const char* repl);

	String before(int pos);
	String before(const String& x, int startpos = 0);
	String before(const char* t, int startpos = 0);
	String before(char c, int startpos = 0);
	String after(int pos);
	String after(const String& x, int startpos = 0);
	String after(const char* t, int startpos = 0);
	String after(char c, int startpos = 0);

	int OK() const;
};


String operator + (const String& x, const String& y);
String operator + (const String& x, const char* y);
String operator + (const String& x, char y);
String operator + (const char* x, const String& y);

int compare(const String& x, const String& y);
int compare(const String& x, const char* y);

int operator==(const String& x, const String& y);
int operator!=(const String& x, const String& y);
int operator>(const String& x, const String& y);
int operator>=(const String& x, const String& y);
int operator<(const String& x, const String& y);
int operator<=(const String& x, const String& y);
int operator==(const String& x, const char* t);
int operator!=(const String& x, const char* t);
int operator>(const String& x, const char* t);
int operator>=(const String& x, const char* t);
int operator<(const String& x, const char* t);
int operator<=(const String& x, const char* t);

String upcase(const String& x);

#endif
