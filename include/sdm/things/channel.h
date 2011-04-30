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
	int m_bits;
	std::string *m_name;

    public:
	MooChannel(const char *name = NULL, int bits = 0, moo_id_t id = -1, moo_id_t parent = -1);
	virtual ~MooChannel();
	static MooChannel *make_channel(const char *name);

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	static int quit(MooThing *user);

	/// Authentication Methods
	static int exists(const char *name);
	static int valid_channelname(const char *name);
	static MooThing *get(const char *name);

    public:
	/// Accessors
	const char *name() { return(m_name->c_str()); }
};

extern MooObjectType moo_channel_obj_type;

int init_channel(void);
void release_channel(void);
MooObject *make_moo_channel(MooDataFile *data);

#endif

