/*
 * Header Name:	interface.h
 * Description:	Interface Header
 */

#ifndef _SDM_INTERFACE_INTERFACE_H
#define _SDM_INTERFACE_INTERFACE_H

#include <stdarg.h>

#include <sdm/globals.h>

#define IO_COND_READ			0x0001
#define IO_COND_WRITE			0x0002
#define IO_COND_ERROR			0x0004

#define IO_READY_READ			0x0010
#define IO_READY_WRITE			0x0020
#define IO_READY_ERROR			0x0040

#define SDM_IBF_RELEASING		0x1000

#define SDM_INTERFACE(ptr)		( (struct sdm_interface *) (ptr) )

struct sdm_interface;
struct sdm_interface_type;

typedef int (*sdm_int_init_t)(struct sdm_interface *, va_list);
typedef void (*sdm_int_release_t)(struct sdm_interface *);
typedef int (*sdm_int_read_t)(struct sdm_interface *, char *, int);
typedef int (*sdm_int_write_t)(struct sdm_interface *, const char *);

#define SDM_INTERFACE_READ(inter, buffer, max)	\
	SDM_INTERFACE(inter)->type->read(SDM_INTERFACE(inter), (buffer), (max))
#define SDM_INTERFACE_WRITE(inter, str)	\
	SDM_INTERFACE(inter)->type->write(SDM_INTERFACE(inter), (str))

#define SDM_INTERFACE_SET_READY_READ(inter)	( SDM_INTERFACE(inter)->bitflags |= IO_READY_READ )
#define SDM_INTERFACE_SET_NOT_READY_READ(inter)	( SDM_INTERFACE(inter)->bitflags &= ~IO_READY_READ )

struct sdm_interface {
	struct sdm_interface_type *type;
	int bitflags;
	int condition;
	struct callback_s callback;
	int read;
	int write;
	int error;
};

struct sdm_interface_type {
	int size;
	sdm_int_init_t init;
	sdm_int_release_t release;
	sdm_int_read_t read;
	sdm_int_write_t write;
};

int init_interface(void);
void release_interface(void);

struct sdm_interface *create_sdm_interface(struct sdm_interface_type *, ...);
void destroy_sdm_interface(struct sdm_interface *);

void sdm_interface_set_callback(struct sdm_interface *, int, callback_t, void *);
struct callback_s sdm_interface_get_callback(struct sdm_interface *);

int sdm_interface_select(float);

#endif

