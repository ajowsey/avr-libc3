/* $Id$	*/

#ifndef __AVR__
# define _GNU_SOURCE		/* to include memmem()	*/
# include <stdio.h>
# define PRINTFLN(line, fmt, ...)	\
    printf("\nLine %d: " fmt "\n", line, ##__VA_ARGS__)
# define EXIT(code)	exit ((code) < 255 ? (code) : 255)
# define memcmp_P	memcmp
#else
# define PRINTFLN(args...)
# define EXIT	exit
#endif

#include <stdlib.h>
#include <string.h>
#include "progmem.h"

void Check (int line,
	    const void *s1, size_t n1, const char *s2, size_t n2, int expect)
{
    char t1[300];
    char t2[100];
    char *p;

    if (n1 > sizeof(t1) || n2 > sizeof(t2))
	exit (1);
    memcpy_P (t1, s1, n1);
    memcpy_P (t2, s2, n2);
    p = memmem (t1, n1, t2, n2);

    if (expect < 0) {
	if (p) {
	    PRINTFLN (line, "return nonzero");
	    EXIT (line);
	}
    } else {
	if (p != t1 + expect) {
	    PRINTFLN (line, "expect= %d  result= %d", expect, p - t1);
	    EXIT (1000 + line);
	}
    }
    if (memcmp_P (t1, s1, n1) || memcmp_P (t2, s2, n2)) {
	PRINTFLN (line, "string(s) is changed");
	EXIT (2000 + line);
    }
}

#define CHECK(s1, n1, s2, n2, expect)	do {			\
    Check (__LINE__, PSTR(s1), n1, PSTR(s2), n2, expect);	\
} while (0)

/* Very long s2[]. Separate function: we are restricted in stack usage. */
void Check_2 (void)
{
    char s1[8];
    char s2[260];
	
    memset (s1, '.', sizeof(s1));
    memset (s2, '.', sizeof(s2));
    if (memmem (s1, sizeof(s1), s2, sizeof(s2)))
	EXIT (__LINE__);
}

int main ()
{
    /* Empty 'needle'.	*/
    CHECK ("", 0, "", 0, 0);
    CHECK ("12345", 5, "1", 0, 0);

    /* 'needle' of 1 byte long	*/
    CHECK ("", 0, "a", 1, -1);
    CHECK ("b", 1, "a", 1, -1);
    CHECK ("a", 1, "a", 1, 0);
    CHECK ("abcbef", 6, "a", 1, 0);
    CHECK (".a", 2, "a", 1, 1);
    CHECK (".a.", 3, "a", 1, 1);
    CHECK ("ABCDEFGH", 8, "H", 1, 7);
    CHECK ("", 0, "\000", 1, -1);
    CHECK ("", 1, "\000", 1, 0);
    CHECK (".", 2, "\000", 1, 1);
    CHECK ("\000\001", 2, "\001", 1, 1);
    
    /* 'needle' of 2 bytes long	*/
    CHECK ("", 0, "12", 2, -1);
    CHECK ("1", 1, "12", 2, -1);
    CHECK ("13", 2, "12", 2, -1);
    CHECK ("32", 2, "12", 2, -1);
    CHECK ("12", 2, "12", 2, 0);
    CHECK ("123", 3, "12", 2, 0);
    CHECK ("012", 3, "12", 2, 1);
    CHECK ("01200", 5, "12", 2, 1);
    CHECK ("\000", 1, "\000\000", 2, -1);
    CHECK ("\000\000", 2, "\000\000", 2, 0);
    
    /* partially mathing	*/
    CHECK ("a_ab_abc_abcd_abcde", 19, "abcdef", 6, -1);
    CHECK ("a_ab_abc_abcd_abcde_abcdef", 26, "abcdef", 6, 20);
    CHECK ("aababcabcdabcde", 15, "abcdef", 6, -1);
    CHECK ("aababcabcdabcdeabcdef", 21, "abcdef", 6, 15);
    
    /* repeated chars	*/
    CHECK ("abaabaaabaaaab", 14, "aaaaab", 6, -1);
    CHECK ("abaabaaabaaaabaaaaab", 20, "aaaaab", 6, 14);
    
    /* A first match is returned.	*/
    CHECK ("_foo_foo", 8, "foo", 3, 1);
    
    /* Case is not ignored.	*/
    CHECK ("A", 1, "a", 1, -1);
    CHECK (".z", 2, ".Z", 2, -1);

    /* Very long s1	*/
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "...............................................................*",
	   256, "*", 1, 255);
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "................................................................"
	   "*", 257, "*", 1, 256);
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "................................................................"
	   ".*.", 258, "*", 1, 257);
    CHECK ("................................................................"
	   "................................................................"
	   "................................................................"
	   "................................................................"
	   "...*..", 260, ".*", 2, 258);

    Check_2 ();

    return 0;
}
