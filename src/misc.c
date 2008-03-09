/*
 * Module Name:	misc.c
 * Description:	Miscellaneous Functions
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/misc.h>

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


