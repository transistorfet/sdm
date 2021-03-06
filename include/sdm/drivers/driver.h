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
#define SDM_IBF_CONNECTED		0x2000

class MooDriver : public MooObject {
    protected:
	int m_bits;
	int m_rfd;
	int m_wfd;
	int m_efd;
	moo_id_t m_owner;	/// This is the object associated with this connection (the user object).  Tasks launched to handle
				/// this connection will be run as this owner.

    public:
	MooDriver();
	~MooDriver();
	int ready() { return(m_bits & IO_STATE); }
	int connected() { return(m_bits & SDM_IBF_CONNECTED); }

	virtual int read(char *data, int len) { return(0); }
	virtual int write(const char *data) { return(0); }
	virtual int handle(int ready) { return(0); }

	static int wait(float t);

    protected:
	void set_ready() { m_bits |= IO_READY_READ; }
	void set_not_ready() { m_bits &= ~IO_READY_READ; }
	void clear_state() { m_bits &= ~IO_STATE; }

	void set_connected(int state) { m_bits = (m_bits & ~SDM_IBF_CONNECTED) | ( state ? SDM_IBF_CONNECTED : 0 ); }
};

int init_driver(void);
void release_driver(void);

#endif

