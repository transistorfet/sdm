/*
 * Name:	exception.h
 * Description:	Exception Handling
 */

#ifndef _SDM_EXCEPTION_H
#define _SDM_EXCEPTION_H

class MooException {
	const char *m_msg;
    public:
	MooException(const char *msg) { m_msg = msg; }
	const char *get() const { return(m_msg); }
};


#ifdef DEFINE_EXCEPTIONS
#define EXTERN(def, msg)	def(msg);
#else
#define EXTERN(def, msg)	extern def;
#endif

EXTERN(MooException moo_mem_error, "Out of Memory")
EXTERN(MooException moo_whatever, "Whatever")

// TODO and then you can do throw MooException("Some customized Error");


#endif

