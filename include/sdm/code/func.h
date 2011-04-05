/*
 * Object Name:	func.h
 * Description:	Moo Code Function
 */

#ifndef _SDM_CODE_FUNC_H
#define _SDM_CODE_FUNC_H

#include <stdarg.h>

#include <sdm/objs/object.h>

typedef int (*moo_code_func_t)(class MooCodeFrame *frame, class MooObjectHash *env, class MooArgs *args);

class MooCodeFunc : public MooObject {
    public:
	moo_code_func_t m_func;

	MooCodeFunc(moo_code_func_t func) { m_func = func; }
	virtual ~MooCodeFunc() { }

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

    private:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectHash *env, MooArgs *args);
};

extern MooObjectType moo_code_func_obj_type;

MooObject *moo_code_func_create(void);

#endif

