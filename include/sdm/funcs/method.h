/*
 * Header Name:	method.h
 * Description:	Methodifying Object Header
 */

#ifndef _SDM_FUNCS_METHOD_H
#define _SDM_FUNCS_METHOD_H

#include <sdm/globals.h>
#include <sdm/code/code.h>
#include <sdm/objs/object.h>

class MooMethod : public MooObject {
    public:
	MooObject *m_obj;
	MooObject *m_func;

	MooMethod(MooObject *obj, MooObject *func);
	virtual ~MooMethod() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args);
};

extern MooObjectType moo_method_obj_type;

MooObject *make_moo_method(MooDataFile *data);

#endif


