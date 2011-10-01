/*
 * Object Name:	func.h
 * Description:	Moo Function
 */

#ifndef _SDM_FUNCS_FUNC_H
#define _SDM_FUNCS_FUNC_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef int (*moo_func_t)(class MooCodeFrame *frame, class MooObjectHash *env, class MooObjectArray *args);

class MooFunc : public MooObject {
    protected:
	std::string *m_name;
	moo_func_t m_func;

    public:
	MooFunc(moo_func_t func);
	virtual ~MooFunc();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooObjectArray *args);
};

extern MooObjectType moo_func_obj_type;

MooObject *make_moo_func(MooDataFile *data);

#endif

