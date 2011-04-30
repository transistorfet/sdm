/*
 * Object Name:	string.h
 * Description:	String Object
 */

#ifndef _SDM_OBJS_STRING_H
#define _SDM_OBJS_STRING_H

#include <stdarg.h>

#include <sdm/objs/object.h>

class MooString : public MooObject {
    public:
	char *m_str;
	int m_len;

	MooString(const char *fmt, ...);
	virtual ~MooString();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	virtual const char *get_string() { return(m_str); }

	// TODO should all fault conditions throw an exception instead of returning an int?
	int set(const char *str);

	int compare(const char *str);
	int compare(const char *str, int len);
};

extern MooObjectType moo_string_obj_type;

MooObject *make_moo_string(MooDataFile *data);

#endif

