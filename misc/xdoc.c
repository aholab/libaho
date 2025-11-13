#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 1000
int main( void )
{
	static char s[N];
	int indoc=0;

	while (fgets(s,N,stdin)) {
		if (strstr(s,"<DOC>")) indoc=1;
		else if (strstr(s,"</DOC>")) { indoc=0; fprintf(stdout,"\n"); }
		else if (indoc) fputs(s,stdout);
	}

	return 0;
}