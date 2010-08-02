/*
 * Module Name:	misc.c
 * Description:	Miscellaneous Functions
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/memory.h>

#define STRING_MAX_SIZE			1024

int sdm_status(const char *fmt, ...)
{
	va_list va;

	// TODO add the date/time to the output
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
	putchar('\n');
	return(0);
}


/**
 * Allocate a string big enough to hold the given string after
 * character expansion (using the given arguments) and return
 * it or NULL on error.
 */
string_t make_string(char *fmt, ...)
{
	va_list va;
	string_t str;
	va_start(va, fmt);
	char buffer[STRING_MAX_SIZE];

	vsnprintf(buffer, STRING_MAX_SIZE, fmt, va);
	if (!(str = (string_t) memory_alloc(strlen(buffer) + 1)))
		return(NULL);
	strcpy(str, buffer);
	return(str);
}

/**
 * Allocate a string and copy the given string to it.
 */
string_t duplicate_string(string_t str)
{
	string_t dup;

	if (!str || !(dup = (string_t) memory_alloc(strlen(str) + 1)))
		return(NULL);
	strcpy(dup, str);
	return(dup);
}
