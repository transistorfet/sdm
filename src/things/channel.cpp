/*
 * Object Name:	channel.cpp
 * Description:	Channel Object
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/tasks/task.h>
#include <sdm/objs/hash.h>
#include <sdm/objs/object.h>
#include <sdm/things/thing.h>
#include <sdm/things/channel.h>

/*
#define USER_INIT_SIZE		64

MooObjectType moo_channel_obj_type = {
	&moo_thing_obj_type,
	"channel",
	typeid(MooChannel).name(),
	(moo_type_make_t) make_moo_channel
};

static MooHash<MooChannel *> *channel_list = NULL;

int init_channel(void)
{
	if (channel_list)
		return(1);
	// TODO should this be DELETEALL if the things are also in the thing database, which is also DELETEALL?
	channel_list = new MooHash<MooChannel *>(USER_INIT_SIZE, MOO_HBF_REMOVE | MOO_HBF_DELETEALL);
	moo_object_register_type(&moo_channel_obj_type);

	/// Pre-cache channels
	MooThing::reference(MOO_CHANNELS);
	return(0);
}

void release_channel(void)
{
	if (!channel_list)
		return;
	delete channel_list;
	channel_list = NULL;
}

MooObject *make_moo_channel(MooDataFile *data)
{
	MooChannel *obj = new MooChannel();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooChannel::MooChannel(const char *name, int bits, moo_id_t id, moo_id_t parent) : MooThing(id, parent)
{
	m_bits = bits;
	m_name = NULL;

	if (name) {
		if (!MooChannel::valid_channelname(name))
			throw MooException("Channel name error");
		m_name = new std::string(name);
		if (channel_list->set(name, this))
			throw MooException("Unable to add channel to list: %s", name);
	}
}

MooChannel::~MooChannel()
{
	/// Release the channel's other resources
	if (m_name) {
		channel_list->remove(m_name->c_str());
		delete m_name;
	}
}

MooChannel *MooChannel::make_channel(const char *name)
{
	MooChannel *channel;

//	MooThing *parent;
//
//	if (MooChannel::exists(name))
//		throw MooException("User already exists.  Cannot make guest.");
//	// TODO should these references perhaps be stored in a hash table of some kind which is searched with $thing references
//	if (!(parent = MooThing::reference(MOO_GENERIC_USER)))
//		throw moo_thing_not_found;
//	channel = new MooChannel(name, MOO_UBF_GUEST, MOO_NEW_ID, parent->id());
//	channel->set_property("name", name);
//	channel->init();
//	channel->owner(channel->m_id);
//	channel->set_property("description", "You see a new person who looks rather out-of-place.");
//	// TODO is this the correct way to moving a channel to the starting location?
//	channel->moveto(MooThing::reference(MOO_START_ROOM));

	return(channel);
}

int MooChannel::read_entry(const char *type, MooDataFile *data)
{
	if (!strcmp(type, "name")) {
		if (!m_name) {
			char name[STRING_SIZE];

			data->read_string_entry(name, STRING_SIZE);
			m_name = new std::string(name);
			if (channel_list->set(name, this))
				throw MooException("Unable to add channel to list: %s", name);
		}
		// TODO else error????
	}
	else
		return(MooThing::read_entry(type, data));
	return(0);
}

int MooChannel::write_data(MooDataFile *data)
{
	MooThing::write_data(data);
	data->write_string_entry("name", m_name->c_str());
	return(0);
}

int MooChannel::quit(MooThing *user)
{
	MooHashEntry<MooChannel *> *entry;

	channel_list->reset();
	while ((entry = channel_list->next_entry())) {
		// TODO quit works except for printing the quit message (only once) to all appropriate users
		//entry->m_data->do_action(user, entry->m_data, "quit");

		// TODO this doesn't work because the leave method expects the current_user to be the one leaving
		//entry->m_data->call_method(user, entry->m_data, "leave");
	}
	return(0);
}
*/

int MooChannel::exists(const char *name)
{
	if (MooChannel::get(name))
		return(1);
	return(0);
}

int MooChannel::valid_channelname(const char *name)
{
	int i;

	// TODO should channel names be bare or should they start with #.  Should we require them to have a non-number to
	//	distinguish them from objects?
	if (name[0] != '#')
		return(0);
	for (i = 1; name[i] != '\0'; i++) {
		if (!(((name[i] >= 0x30) && (name[i] <= 0x39))
		    || ((name[i] >= 0x41) && (name[i] <= 0x5a))
		    || ((name[i] >= 0x61) && (name[i] <= 0x7a))
		    || (name[i] == '-') || (name[i] == '.') || (name[i] == '_')))
			return(0);
	}
	return(1);
}

MooThing *MooChannel::get(const char *name)
{
	MooThing *channels;
	MooObjectHash *list;

	if (!(channels = MooThing::reference(MOO_CHANNELS))) {
		moo_status("CHANNEL: channels object not found");
		return(NULL);
	}
	if (!(list = dynamic_cast<MooObjectHash *>(channels->resolve_property("list")))) {
		moo_status("CHANNEL: channel list not found on channels object");
		return(NULL);
	}
	return(dynamic_cast<MooThing *>(list->get(name)));
}


