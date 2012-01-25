/*
 * Header Name:	driver.h
 * Description:	Driver Header
 */

#ifndef _SDM_DRIVERS_DRIVER_H
#define _SDM_DRIVERS_DRIVER_H

#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/code/task.h>
#include <sdm/objs/object.h>

#define IO_READY_READ			0x0001
#define IO_READY_WRITE			0x0002
#define IO_READY_ERROR			0x0004
#define IO_STATE			0x0007

#define SDM_IBF_RELEASING		0x1000

class MooDriver : public MooObject {
    protected:
	int m_bits;
	int m_rfd;
	int m_wfd;
	int m_efd;
	MooTask *m_task;	// TODO naturally this would point to the server object in future and the task would only last as
				//	long as the handling (since all tasks will become generic)

	moo_id_t m_owner;	/// This is the object associated with this connection (the user object).  Tasks launched to handle
				/// this connection will be run as this owner.
	// TODO some kind of callback pointer (to replace m_task i guess)  It could even be the server object and then we always
	//	assume the method name to call.  We will be able to only have one method to handle all connections of a specific
	//	server, but we can always just multiplex if needed


	void set_ready() { m_bits |= IO_READY_READ; }
	void set_not_ready() { m_bits &= ~IO_READY_READ; }
	void clear_state() { m_bits &= ~IO_STATE; }

    public:
	MooDriver();
	~MooDriver();
	void set_task(MooTask *task) { m_task = task; }
	int ready() { return(m_bits & IO_STATE); }

	virtual int read(char *data, int len) { return(0); }
	virtual int write(const char *data) { return(0); }

	static int wait(float t);
};

int init_driver(void);
void release_driver(void);

#endif

