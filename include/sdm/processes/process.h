/*
 * Object Name:	process.h
 * Description:	Process Object
 */

#ifndef _SDM_PROCESSES_PROCESS_H
#define _SDM_PROCESSES_PROCESS_H

#include <stdarg.h>

#include <sdm/objs/object.h>

class MooProcess : public MooObject {
    public:
	MooProcess() { }
	virtual ~MooProcess() { }

	virtual int initialize(MooUser *user) = 0;
	virtual int loop(MooUser *user, char *buffer) = 0;
	virtual int release(MooUser *user) = 0;
};


extern MooObjectType moo_process_obj_type;

MooObject *moo_process_create(void);

#endif

