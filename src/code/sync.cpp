/*
 * Object Name:	sync.cpp
 * Description:	Moo Sync
 */

#include <pthread.h>

#include <stdio.h>
#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/code/code.h>

#include <sdm/code/sync.h>

struct MooObjectType moo_sync_obj_type = {
	"sync",
	typeid(MooSync).name(),
	(moo_type_load_t) load_moo_sync
};

MooObject *load_moo_sync(MooDataFile *data)
{
	MooSync *obj = new MooSync(NULL);
	if (data)
		obj->read_data(data);
	return(obj);
}

MooSync::MooSync(MooObject *func)
{
	pthread_mutex_init(&m_mutex, NULL);
	MOO_INCREF(m_func = func);
}

MooSync::~MooSync()
{
	if (!pthread_mutex_trylock(&m_mutex))
		pthread_mutex_unlock(&m_mutex);
	pthread_mutex_destroy(&m_mutex);
	MOO_DECREF(m_func);
}

int MooSync::read_entry(const char *type, MooDataFile *data)
{
	int res;
	char buffer[STRING_SIZE];
	const MooObjectType *objtype;

	// TODO this doesn't really work at all because if you use (synchronize (lambda () ...)), you expect a lambda object and
	//	thus want the (lambda ...) code expr to be evaluated (synchronize is a function then), but if you want
	//	something like (synchronize ...) then you'd need a form, but you need somewhere to store the mutex in this case,
	//	most likely on the code expr itself, since there would be no sync value returned which gets assigned as a function
/*
	if (!strcmp(type, "object")) {
		data->read_attrib_string("type", buffer, STRING_SIZE);
		if (!(objtype = moo_object_find_type(buffer, NULL))) {
			moo_status("SYNC: Unable to find object type, %s", buffer);
			return(-1);
		}
		res = data->read_children();
		if (!(m_func = moo_make_object(objtype, res ? data : NULL))) {
			moo_status("SYNC: Error loading object");
			return(-1);
		}
		if (res)
			data->read_parent();
	}
		return(MooObject::read_entry(type, data));
*/
	return(MOO_HANDLED);
}

int MooSync::write_data(MooDataFile *data)
{
	data->write_begin_entry("object");
	data->write_attrib_string("type", m_func->objtype_name());
	m_func->write_data(data);
	data->write_end_entry();
	return(0);
}

int MooSync::to_string(char *buffer, int max)
{
	int i = 0;

	strcpy(buffer, "(synchronize ");
	i += 13;
	i += m_func->to_string(&buffer[i], STRING_SIZE - i);
	buffer[i] = ')';
	return(0);
}

int MooSync::lock()
{
	// TODO you could look into recursive mutexes and allow multiple entries
	if (pthread_mutex_lock(&m_mutex))
		throw MooException("SYNC: Error occurred while attempting to lock");
	return(0);
}

int MooSync::unlock()
{
	if (pthread_mutex_unlock(&m_mutex))
		throw MooException("SYNC: Error occurred while attempting to unlock");
	return(0);
}


class SyncUnlock : public MooCodeEvent {
	MooSync *m_sync;
    public:
	SyncUnlock(MooObjectHash *env, MooSync *sync) : MooCodeEvent(NULL, env, NULL, NULL) { m_sync = sync; };

	/// If we evaluate this during the normal run loop then an exception hasn't occurred and we therefore do nothing
	int do_event(MooCodeFrame *frame);
	int handle(MooCodeFrame *frame);
};

int SyncUnlock::do_event(MooCodeFrame *frame)
{
	return(m_sync->unlock());
}

int SyncUnlock::handle(MooCodeFrame *frame)
{
	return(m_sync->unlock());
}

int MooSync::do_evaluate(MooCodeFrame *frame, MooObjectArray *args)
{
	// TODO do we need something like this? so that if an exception or return occurs which jumps over this point, we unlock the
	//	resource??? No no no, we need to either way unlock the return point, so we need both an event which unlocks the sync
	//	which executes normally, and a special handler function for that event which also unlocks the sync if an exception or
	//	return occurs.  Perhaps all exceptions and returns should be the same kind of thing such that you can make handling of
	//	all such needs by some generic mechanism in the event (to handle return points, breakpoints, exception handling, etc, and
	//	now also lockpoints.

	// TODO do a search on the stack for an unlock event which is waiting for this sync object and cause an exception about
	//	a possible deadlock detected.  This would prevent deadlocks, even though we should still anyways be pretty deadlock
	//	protected by the use of multiple short tasks (a deadlock would likely occur during a command which we are going to make
	//	a dedicated task for
	this->lock();
/*
	frame->push_event(frame->env(), new SyncUnlock(this));
	return(frame->push_block(frame->env(), m_func));
*/
	return(0);
}


