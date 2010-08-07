/*
 * Header Name:	misc.h
 * Description:	Miscellaneous Functions
 */

#ifndef _SDM_MISC_H
#define _SDM_MISC_H

typedef char * string_t;

int moo_status(const char *, ...);

string_t make_string(char *, ...);
string_t duplicate_string(string_t);

#endif

