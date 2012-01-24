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
	MooNumberFormatT m_format;
	union {
		long int m_int;
		double m_float;
	};

    public:
	MooNumber(long int num = 0) { m_format = INT; m_int = num; }
	MooNumber(double num) { m_format = FLOAT; m_float = num; }
	MooNumber(const char *str);
	MooNumber(MooNumber *num);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	virtual long int get_integer() { if (m_format == INT) return(m_int); else if (m_format == FLOAT) return((long int) m_float); else return(0); }
	virtual double get_float() { if (m_format == INT) return((double) m_int); else if (m_format == FLOAT) return(m_float); else return(0); }

	void set_format(MooNumberFormatT format);

	inline int compare(MooNumber *num) {
		// TODO I think this is wrong but the alternative is complicated so I should look into a better solution
		float num1, num2;

		num1 = this->get_float();
		num2 = num->get_float();

		if (num1 > num2)
			return(1);
		else if (num1 < num2)
			return(-1);
		else
			return(0);
	}

	inline void add(MooNumber *num) {
		if (m_format != num->m_format && m_format == INT)
			this->set_format(FLOAT);
		if (m_format == INT)
			m_int += num->get_integer();
		else if (m_format == FLOAT)
			m_float += num->get_float();
	}

	inline void subtract(MooNumber *num) {
		if (m_format != num->m_format && m_format == INT)
			this->set_format(FLOAT);
		if (m_format == INT)
			m_int -= num->get_integer();
		else if (m_format == FLOAT)
			m_float -= num->get_float();
	}

	inline void multiply(MooNumber *num) {
		if (m_format != num->m_format && m_format == INT)
			this->set_format(FLOAT);
		if (m_format == INT)
			m_int *= num->get_integer();
		else if (m_format == FLOAT)
			m_float *= num->get_float();
	}

	inline void divide(MooNumber *num) {
		if (m_format != num->m_format && m_format == INT)
			this->set_format(FLOAT);
		if (m_format == INT) {
			long int div = num->get_integer();
			if (div == 0)
				throw MooException("Attempted to divide by zero");
			m_int /= div;
		}
		else if (m_format == FLOAT) {
			double div = num->get_float();
			if (div == 0)
				throw MooException("Attempted to divide by zero");
			m_float /= div;
		}
	}
};

extern MooObjectType moo_number_obj_type;

MooObject *load_moo_number(MooDataFile *data);

#endif

