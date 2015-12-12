#include "stdafx.h"

#include "radix40.h"

const char legal_chars[41] = " ,-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char lookup_chars[128] = 
{IL, IL, IL, IL, IL, IL, IL, IL,/* 00 */
IL, IL, IL, IL, IL, IL, IL, IL, /* 08 */
IL, IL, IL, IL, IL, IL, IL, IL, /* 10 */
IL, IL, IL, IL, IL, IL, IL, IL, /* 18 */
 0, IL, IL, IL, IL, IL, IL, IL, /* 20 */
IL, IL, IL, IL,  1,  2,  3, IL, /* 28 */
 4,  5,  6,  7,  8,  9, 10, 11, /* 30 */
12, 13 ,IL, IL, IL, IL, IL, IL, /* 38 */
IL, 14, 15, 16, 17, 18, 19, 20, /* 40 */
21, 22, 23, 24, 25, 26, 27, 28, /* 48 */
29, 30, 31, 32, 33, 34, 35, 36, /* 50 */
37, 38, 39, IL, IL, IL, IL, IL, /* 58 */
IL, 14, 15, 16, 17, 18, 19, 20, /* 60 */
21, 22, 23, 24, 25, 26, 27, 28, /* 68 */
29, 30, 31, 32, 33, 34, 35, 36, /* 70 */
37, 38, 39, IL, IL, IL, IL, IL, /* 78 */

//IL, IL, IL, IL, IL, IL, IL, IL, /* 80 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* 88 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* 90 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* 98 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* A0 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* A8 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* B0 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* B8 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* C0 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* C8 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* D0 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* D8 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* E0 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* E8 */
//IL, IL, IL, IL, IL, IL, IL, IL, /* F0 */
//IL, IL, IL, IL, IL, IL, IL, IL  /* F8 */
};

const int weights[3] = { 1600, 40, 1 };

int ascii_to_radix40(Radix40 *radix40_data, const RADIX40_CHAR *ascii_data, int max_chars)
/* this routine converts a null-terminated ascii character string */
/* to radix 40 representation.  The allowable characters are: */
/* A-Z, 0-9, period, comma, hyphen, and space.  If any illegal characters */
/* are detected, they will be converted to hyphens, and the return value */
/* will be S_ILLEGAL. */
/* Lowercase letters will be upper-cased without error indication. */
/* The radix40 value will be padded with blanks to a multiple of three */
/* characters. If the number of characters in the ascii string is > max_chars */
/* only max_chars will be converted, and the return value will be S_ILLEGAL. */
/* If no error is detected, the return value will be S_OKAY. */
{
//	int i;
	unsigned char j;
//	int ascii_length;
	int result;
//	int words_to_convert;
	int words_to_clear;
	int cycle;
	unsigned current_word_index;

	result = S_OKAY;
//	ascii_length = strlen(ascii_data);
//	if (ascii_length > max_chars)
//		{
//		ascii_length = max_chars;
//		result = S_ILLEGAL;
//		}

//	words_to_convert = ascii_length / 3;
//	if (ascii_length % 3 != 0)
//		words_to_convert ++;

	words_to_clear = max_chars / 3;
	if (max_chars % 3 != 0)
		words_to_clear ++;

	memset(radix40_data,0,words_to_clear*sizeof(Radix40));

	current_word_index = unsigned(-1);
	cycle = 0;
	const RADIX40_CHAR* limit = ascii_data + max_chars;
	for (const RADIX40_CHAR* ptr = ascii_data; *ptr != 0; ++ptr)
		{
		if (ptr == limit)
			{
			result = S_ILLEGAL;
			break;
			}
		if (cycle == 0)
			current_word_index ++;
		j = (*ptr & 0xFF80)? IL : lookup_chars[*ptr];
		if (j & ILLEGAL)
			{
			j = HYPHEN ; /* make it a hyphen */
			result = S_ILLEGAL; /* and remember that it was illegal */
			}
		radix40_data[current_word_index] += Radix40(weights[cycle] * j);
		cycle = (cycle + 1) % 3;
		}

	return(result);
}

int radix40_to_ascii(RADIX40_CHAR *ascii_data, const Radix40 *radix40_data,int max_chars)
/* this routine converts a radix 40 character string */
/* to ascii representation.  Trailing blanks will be deleted. */
{
	int i;
	int ascii_length;
//	int new_ascii_length;
	int words_to_convert;
	int cycle;
	unsigned current_word_index;
	unsigned current_word = 0;
	unsigned current_char;

	ascii_length = max_chars;

	words_to_convert = ascii_length / 3;
	if (ascii_length % 3 != 0)
		words_to_convert ++;

	memset(ascii_data, 0, (max_chars+1) * sizeof(RADIX40_CHAR));

	current_word_index = unsigned(-1);
	cycle = 0;
	for (i = 0; i < ascii_length; i ++)
		{
		if (cycle == 0)
			{
			current_word_index ++;
			current_word = radix40_data[current_word_index];
			}
		current_char = current_word / weights[cycle];
		current_word -= current_char * weights[cycle];
		ascii_data[i] = legal_chars[current_char];
		cycle = (cycle + 1) % 3;
		}

	//new_ascii_length = strlen((char*)ascii_data);
	//for (i = new_ascii_length - 1; i >= 0; i --)
	while (--i >= 0)
		{
		if (ascii_data[i] != ' ')
			break;
		ascii_data[i] = 0;
		}

	return(S_OKAY);
}

