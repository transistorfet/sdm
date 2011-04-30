/*
 * Object Name:	channel.h
 * Description:	Channl Object
 */

#ifndef _SDM_THINGS_CHANNEL_H
#define _SDM_THINGS_CHANNEL_H

#include <string>
#include <stdio.h>
#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/things/thing.h>

class MooChannel : public MooThing {
    public:
	static int quit(MooThing *user);

	/// Authentication Methods
	static int exists(const char *name);
	static int valid_channelname(const char *name);
	static MooThing *get(const char *name);
};

#endif

