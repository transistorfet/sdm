/*
 * Module Name:	exception.c
 * Description:	Exception Handling & Reporting Functions
 */

#include <string>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/exception.h>

#define EXCEPTION_STRING	256
#define STRING_MAX_SIZE		1024

MooException moo_mem_error(E_FATAL, "Out of memory");
MooException moo_closed("Connection closed by remote end.");
MooException moo_permissions("Permission denied");
MooException moo_type_error("Invalid type");
MooException moo_method_object("Method called on invalid object");
MooException moo_args_mismatched("The wrong number of arguments were passed.");
MooException moo_thing_not_found("Required thing not found");
MooException moo_evaluate_error("Unable to evaluate object");
MooException moo_unknown(E_FATAL, "Unknown error");

MooException::MooException(const char *msg, ...)
{
	va_list va;
	va_start(va, msg);
	this->init(E_NORMAL, msg, va);
}

MooException::MooException(int type, const char *msg, ...)
{
	va_list va;
	va_start(va, msg);
	this->init(type, msg, va);
}

MooException::MooException(int type, const char *msg, va_list va)
{
	this->init(type, msg, va);
}

void MooException::init(int type, const char *msg, va_list va)
{
	char buffer[EXCEPTION_STRING];

	m_type = type;
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


