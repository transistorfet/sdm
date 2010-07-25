/*
 * Header Name:	interface.h
 * Description:	Interface Header
 */

#ifndef _SDM_INTERFACES_INTERFACE_H
#define _SDM_INTERFACES_INTERFACE_H

#include <stdarg.h>

#include <sdm/globals.h>
#include <sdm/objs/object.h>

#define IO_COND_READ			0x0001
#define IO_COND_WRITE			0x0002
#define IO_COND_ERROR			0x0004

#define IO_READY_READ			0x0010
#define IO_READY_WRITE			0x0020
#define IO_READY_ERROR			0x0040

#define SDM_IBF_RELEASING		0x1000

#define SDM_INTERFACE_READ(inter, buffer, max)	\
	SDM_INTERFACE_GET_TYPE(inter)->read(SDM_INTERFACE(inter), (buffer), (max))
#define SDM_INTERFACE_WRITE(inter, str)	\
	SDM_INTERFACE_GET_TYPE(inter)->write(SDM_INTERFACE(inter), (str))

#define SDM_INTERFACE_SET_READY_READ(inter)	( SDM_INTERFACE(inter)->bitflags |= IO_READY_READ )
#define SDM_INTERFACE_SET_NOT_READY_READ(inter)	( SDM_INTERFACE(inter)->bitflags &= ~IO_READY_READ )

class MooInterface : public MooObject {
    protected:
	int bitflags;
	int condition;
	MooCallback callback;
	int read;
	int write;
	int error;
    public:
	MooInterface();
	~MooInterface();

	virtual int read(char *data, int len) = 0;
	virtual int write(const char *data) = 0;

	void set_callback(callback_t func, void *ptr) { this->callback = MooCallback(func, ptr); }
	MooCallback get_callback() { return(this->callback); }
};

extern MooObjectType moo_interface_obj_type;

int init_interface(void);
void release_interface(void);
MooObject *moo_interface_create(void);

int sdm_interface_init(struct sdm_interface *, int, va_list);
void sdm_interface_release(struct sdm_interface *);

int moo_interface_select(float);

#endif

