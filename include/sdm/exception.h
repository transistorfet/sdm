/*
 * Name:	exception.h
 * Description:	Exception Handling
 */

#ifndef _SDM_EXCEPTION_H
#define _SDM_EXCEPTION_H

#include <string>

class MooException {
	std::string m_msg;
    public:
	MooException(const char *msg, ...);
	const char *get() const { return(m_msg.c_str()); }
};

extern MooException moo_mem_error;
extern MooException moo_closed;
extern MooException moo_permissions;
extern MooException moo_thing_not_found;
extern MooException moo_unknown;

int moo_status(const char *fmt, ...);

#endif

