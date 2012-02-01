/*
 * Object Name:	funcptr.h
 * Description:	Moo Function Pointer
 */

#ifndef _SDM_OBJS_FUNCPTR_H
#define _SDM_OBJS_FUNCPTR_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef int (*moo_func_t)(class MooCodeFrame *frame, class MooObjectArray *args);

class MooFuncPtr : public MooObject {
    protected:
	std::string *m_name;
	moo_func_t m_func;

    public:
	MooFuncPtr(moo_func_t func);
	virtual ~MooFuncPtr();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectArray *args);
};

extern MooObjectType moo_func_ptr_obj_type;

MooObject *load_moo_func_ptr(MooDataFile *data);

#endif

