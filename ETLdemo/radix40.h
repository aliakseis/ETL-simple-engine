#ifndef radix40_h_
#define radix40_h_

/*TITLE include file for Radix40 routines */
//http://www.steveheller.com/opt/prologue.htm
/****keyword-flag*** "%v %f %n" */
/* "5 20-Mar-98,20:29:58 RADIX40.H" */

enum
{
	CPW = 3, /* characters per word in Radix40 code */

	S_OKAY = 0,
	S_ILLEGAL = 1,
};

const char HYPHEN = 2;
const char ILLEGAL = -128;
const char IL = (HYPHEN|ILLEGAL);
/* this is the code for a -, but with the illegal flag set */
/* it is used in the conversion table to indicate that a character is invalid */

typedef unsigned short Radix40;

typedef WCHAR RADIX40_CHAR;

int radix40_to_ascii(RADIX40_CHAR *ascii_data, const Radix40 *radix40_data,int max_chars);

int ascii_to_radix40(Radix40 *radix40_data, const RADIX40_CHAR *ascii_data, int max_chars);

#endif// radix40_h_
