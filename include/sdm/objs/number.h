/*
 * Object Name:	number.h
 * Description:	Number Object
 */

#ifndef _SDM_OBJS_NUMBER_H
#define _SDM_OBJS_NUMBER_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef enum {
	INT,
	FLOAT
} MooNumberFormatT;

// TODO We could instead implement this as 2 subclasses of MooNumber, one for each type, and virtual functions for all the operations.
//	This, however, would not allow the number to mutate into a different kind, and thus all operation non-idempotent functions would
//	have to create a new number and return it (we wouldn't allow any it, even if the type doesn't have to change, in order to keep
//	behaviour consistent)
class MooNumber : public MooObject {
    public:
	MooNumberFormatT m_format;
	union {
		long int m_int;
		double m_float;
	};

	MooNumber(long int num = 0) { m_format = INT; m_int = num; }
	MooNumber(double num) { m_format = FLOAT; m_float = num; }
	MooNumber(const char *str);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	virtual long int get_integer() { if (m_format == INT) return(m_int); else if (m_format == FLOAT) return((long int) m_float); else return(0); }
	virtual double get_float() { if (m_format == INT) return((double) m_int); else if (m_format == FLOAT) return(m_float); else return(0); }

	void set_format(MooNumberFormatT format);

	inline int equals(MooNumber *num) {
		// TODO I think this is wrong but the alternative is complicated so I should look into a better solution
		return( this->get_float() == num->get_float() );
	}

	inline void add(MooNumber *num) {
		if (m_format != num->m_format && m_format == INT)
			this->set_format(FLOAT);
		if (m_format == INT)
			m_int += num->get_integer();
		else if (m_format == FLOAT)
			m_float += num->get_float();
	}
};

extern MooObjectType moo_number_obj_type;

MooObject *make_moo_number(MooDataFile *data);

#endif

