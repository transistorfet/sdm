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

int MooChannel::quit(MooThing *user)
{
	//MooHashEntry<MooChannel *> *entry;

	//channel_list->reset();
	//while ((entry = channel_list->next_entry())) {
		// TODO quit works except for printing the quit message (only once) to all appropriate users
		//entry->m_data->do_action(user, entry->m_data, "quit");

		// TODO this doesn't work because the leave method expects the current_user to be the one leaving
		//entry->m_data->call_method(user, entry->m_data, "leave");
	//}
	return(0);
}

int MooChannel::valid_channelname(const char *name)
{
	int i;

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


