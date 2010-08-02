/*
 * Header Name:	interface.h
 * Description:	Interface Header
 */

#ifndef _SDM_INTERFACES_INTERFACE_H
#define _SDM_INTERFACES_INTERFACE_H

#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/tasks/task.h>
#include <sdm/objs/object.h>

#define IO_READY_READ			0x0001
#define IO_READY_WRITE			0x0002
#define IO_READY_ERROR			0x0004
#define IO_STATE			0x0007

#define SDM_IBF_RELEASING		0x1000

class MooInterface : public MooObject {
    protected:
	int m_bits;
	int m_rfd;
	int m_wfd;
	int m_efd;
	MooTask *m_task;

	void set_ready() { m_bits |= IO_READY_READ; }
	void set_not_ready() { m_bits &= ~IO_READY_READ; }

    public:
	MooInterface();
	~MooInterface();
	void set_task(MooTask *task) { m_task = task; }
	int ready() { return(m_bits & IO_STATE); }

	virtual int read(char *data, int len) = 0;
	virtual int write(const char *data) = 0;

	static int wait(float t);
};

extern MooObjectType moo_interface_obj_type;

int init_interface(void);
void release_interface(void);

#endif

