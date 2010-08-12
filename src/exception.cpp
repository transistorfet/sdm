/*
 * Module Name:	exception.c
 * Description:	Exception Handling & Reporting Functions
 */

#include <string>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/misc.h>
#include <sdm/memory.h>
#include <sdm/exception.h>

#define EXCEPTION_STRING	256
#define STRING_MAX_SIZE		1024

MooException moo_mem_error("Out of Memory");
MooException moo_closed("Connection closed by remote end.");
MooException moo_unknown("Unknown error");

MooException::MooException(const char *msg, ...)
{
	va_list va;
	char buffer[EXCEPTION_STRING];

	va_start(va, msg);
	vsnprintf(buffer, EXCEPTION_STRING, msg, va);
	m_msg = std::string(buffer);
}

int moo_status(const char *fmt, ...)
{
	va_list va;

	// TODO add the date/time to the output
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
	putchar('\n');
	return(0);
}


