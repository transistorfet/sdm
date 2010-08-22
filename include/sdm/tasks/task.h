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

typedef int moo_tid_t;

class MooUser;
class MooThing;
class MooInterface;

class MooTask : public MooObject {
	moo_id_t m_owner;
	moo_tid_t m_tid;
	moo_tid_t m_parent_tid;

    public:
	MooTask();
	virtual ~MooTask();

	virtual int initialize() = 0;
	virtual int idle() = 0;
	virtual int release() = 0;

	/**
	 * Notify the task of an event from the user object.  If channel is NULL, then it's a message from
	 * the realm, otherwise it is from a specific channel.  If thing is NULL, then it's from a status
	 * message from the realm.  Type determines the type of message (eg. say, emote, join, part, quit,
	 * etc).  The str may be used or not depending on type. */
	virtual int notify(int type, MooThing *channel, MooThing *thing, const char *str) { return(-1); }

	virtual int handle(MooInterface *inter, int ready) = 0;
	virtual int bestow(MooInterface *inter);
	virtual int purge(MooInterface *inter) { return(-1); }

	virtual int purge(MooUser *user) { return(-1); }

	moo_id_t owner(moo_id_t id = -1);
	int switch_handle(MooInterface *inter, int ready);
	static MooTask *current_task();
	static moo_id_t current_owner();

    private:
	static int switch_task(MooTask *task);
};

extern MooObjectType moo_task_obj_type;

int init_task(void);
void release_task(void);

#endif

