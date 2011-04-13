/*
 * Name:	exception.h
 * Description:	Exception Handling
 */

#ifndef _SDM_EXCEPTION_H
#define _SDM_EXCEPTION_H

#include <string>

#define E_FATAL		1
#define E_NORMAL	2

class MooException {
	int m_severity;
	std::string m_msg;
    public:
	MooException() { m_severity = 0; m_msg = std::string(""); }
	MooException(const char *msg, ...);
	MooException(int severity, const char *msg, ...);
	MooException(int severity, const char *msg, va_list va);
	void init(int severity, const char *msg, va_list va);
	const char *get() const { return(m_msg.c_str()); }
	inline int severity() { return(m_severity); }
};

extern MooException moo_mem_error;
extern MooException moo_closed;
extern MooException moo_permissions;
extern MooException moo_type_error;
extern MooException moo_method_object;
extern MooException moo_args_mismatched;
extern MooException moo_thing_not_found;
extern MooException moo_evaluate_error;
extern MooException moo_unknown;

int moo_status(const char *fmt, ...);

#endif

