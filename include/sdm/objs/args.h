/*
 * Object Name:	args.h
 * Description:	Args Object
 */

#ifndef _SDM_OBJS_ARGS_H
#define _SDM_OBJS_ARGS_H

#include <stdarg.h>

#include <sdm/objs/array.h>
#include <sdm/objs/object.h>

#define DEFAULT_ARGS	5

class MooUser;
class MooThing;
class MooAction;
class MooChannel;
class MooCodeExpr;

class MooArgs : public MooObject {
    public:
	MooObject *m_this;
	MooObject *m_result;
	MooObjectArray *m_args;		// TODO better name!??

	MooArgs(int init_size = DEFAULT_ARGS);
	MooArgs(MooObjectArray *args);
	virtual ~MooArgs();
	void set_args(MooObjectArray *&args);

    private:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
};

extern MooObjectType moo_args_obj_type;

MooObject *moo_args_create(void);

#endif

