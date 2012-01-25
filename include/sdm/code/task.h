/*
 * Object Name:	task.h
 * Description:	Task Object
 */

#ifndef _SDM_TASKS_TASK_H
#define _SDM_TASKS_TASK_H

#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#define TNT_STATUS	0x01
#define TNT_JOIN	0x02
#define TNT_LEAVE	0x03
#define TNT_SAY		0x04
#define TNT_EMOTE	0x05
#define TNT_QUIT	0x06

#define TNT_FIRST	0x01
#define TNT_LAST	0x06

typedef int moo_tid_t;

class MooUser;
class MooThing;
class MooDriver;
class MooCodeFrame;
class MooObjectArray;

class MooTask : public MooObject {
	moo_tid_t m_tid;
	moo_tid_t m_parent_tid;
	MooCodeFrame *m_frame;

    public:
	MooTask();
	MooTask(MooObject *user, MooObject *channel);
	virtual ~MooTask();

	static int run_idle();
	void schedule(double time);

	int initialize();
	int idle();
	int release();

	int MooTask::push_call(MooObject *func, MooObject *arg1, MooObject *arg2, MooObject *arg3);
	int MooTask::push_code(const char *code);

	/**
	 * Notify the task of an event from the user object.  If channel is NULL, then it's a message from
	 * the realm, otherwise it is from a specific channel.  If thing is NULL, then it's from a status
	 * message from the realm.  Type determines the type of message (eg. say, emote, join, part, quit,
	 * etc).  The str may be used or not depending on type. */
	virtual int notify(int type, MooThing *thing, MooThing *channel, const char *str) { return(-1); }

	virtual int handle(MooDriver *inter, int ready) { throw MooException("Unable to handle interface"); }
	virtual int bestow(MooDriver *inter);
	virtual int purge(MooDriver *inter) { return(-1); }

	virtual int purge(MooUser *user) { return(-1); }

	/// Accessors
	int switch_handle(MooDriver *inter, int ready);
	static MooTask *current_task();
	static moo_id_t current_owner();
	static moo_id_t current_user();

    private:
	friend class MooObject;
	friend class FrameEventRelegate;
	static moo_id_t current_owner(moo_id_t id);
	static moo_id_t current_owner(MooTask *task, moo_id_t id);
	static int switch_task(MooTask *task);
	static int suid(MooObject *obj, MooCodeFrame *frame);
};

int init_task(void);
void release_task(void);

#endif


