/*
 * Header Name:	globals.h
 * Description:	Global Definitions
 */

#ifndef _SDM_GLOBALS_H
#define _SDM_GLOBALS_H

#include <sdm/exception.h>

#define STRING_SIZE		512
#define LARGE_STRING_SIZE	2048

#define SDM_FAILED		-1
#define SDM_FATAL		-2

typedef void (*destroy_t)(void *);
typedef int (*compare_t)(void *, void *);
typedef int (*callback_t)(void *, void *);

#define DECLARE_UNUSED(arg)	((void) arg)

typedef int moo_id_t;
typedef int moo_mode_t;

#define MOO_MODE_X		0001
#define MOO_MODE_W		0002
#define MOO_MODE_R		0004

#define MOO_MODE_EX		0001
#define MOO_MODE_EW		0002
#define MOO_MODE_ER		0004
#define MOO_MODE_OX		0010
#define MOO_MODE_OW		0020
#define MOO_MODE_OR		0040
#define MOO_MODE_SUID		0400
#define MOO_MODE_STICKY		0100

#define MOO_DEFAULT_MODE	0077

#define SBIT(bits, flag)	( (bits) |= (flag) )
#define RBIT(bits, flag)	( (bits) &= ~(flag) )

#endif


