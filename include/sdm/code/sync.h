/*
 * Object Name:	sync.h
 * Description:	Moo Sync
 */

#ifndef _SDM_CODE_SYNC_H
#define _SDM_CODE_SYNC_H

#include <pthread.h>

#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/code/expr.h>

class MooSync : public MooObject {
    public:
	pthread_mutex_t m_mutex;
	MooObject *m_func;

	MooSync(MooObject *func);
	virtual ~MooSync();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);
	virtual int to_string(char *buffer, int max);

	int lock();
	int unlock();

    protected:
	virtual int do_evaluate(MooCodeFrame *frame, MooObjectArray *args);
};

extern MooObjectType moo_sync_obj_type;

MooObject *load_moo_sync(MooDataFile *data);

#endif

