/*
 * Object Name:	integer.h
 * Description:	Integer Object
 */

#ifndef _SDM_OBJS_INTEGER_H
#define _SDM_OBJS_INTEGER_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef enum {
	INT,
	FLOAT
} MooNumberFormatT;

class MooInteger : public MooObject {
    public:
	MooNumberFormatT m_format;
	union {
		long int m_int;
		double m_float;
	};

	MooInteger(long int num = 0) { m_format = INT; m_int = num; }
	MooInteger(double num) { m_format = FLOAT; m_float = num; }
	MooInteger(const char *str);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	virtual long int get_integer() { if (m_format == INT) return(m_int); else if (m_format == FLOAT) return((long int) m_float); else return(0); }
	virtual double get_float() { if (m_format == INT) return((double) m_int); else if (m_format == FLOAT) return(m_float); else return(0); }

	void set_format(MooNumberFormatT format);
};

extern MooObjectType moo_integer_obj_type;

MooObject *moo_integer_create(void);

#endif

