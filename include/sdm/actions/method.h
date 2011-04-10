/*
 * Header Name:	method.h
 * Description:	MooCode Action Header
 */

#ifndef _SDM_ACTIONS_METHOD_H
#define _SDM_ACTIONS_METHOD_H

#include <sdm/hash.h>
#include <sdm/globals.h>
#include <sdm/code/code.h>
#include <sdm/objs/object.h>

class MooMethod : public MooObject {
	MooObject *m_obj;
	MooObject *m_func;

    public:
	MooMethod(MooObject *obj, MooObject *func);
	virtual ~MooMethod() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args);
};

extern MooObjectType moo_method_obj_type;

MooObject *moo_method_create(void);

#endif


