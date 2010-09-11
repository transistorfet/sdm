/*
 * Module Name:	irc.c
 * Description:	IRC PseudoServ Interface
 */

#include <string.h>
#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/task.h>
#include <sdm/tasks/irc/msg.h>
#include <sdm/tasks/irc/pseudoserv.h>

#include <sdm/things/user.h>
#include <sdm/things/thing.h>
#include <sdm/things/channel.h>
#include <sdm/things/world.h>

#include <sdm/misc.h>
#include <sdm/colours.h>
#include <sdm/globals.h>

MooObjectType moo_irc_pseudoserv_obj_type = {
	&moo_task_obj_type,
	"irc-pseudoserv",
	typeid(MooIRC::PseudoServ).name(),
	(moo_type_create_t) moo_irc_pseudoserv_create
};

time_t server_start = 0;
const char *server_name = "moo.jabberwocky.ca";
const char *server_version = "0.1";

int init_irc_pseudoserv(void)
{
	// TODO why wont this compile
	extern int init_irc_msg();
	init_irc_msg();

	server_start = time(NULL);
	return(0);
}

void release_irc_pseudoserv(void)
{

}

MooObject *moo_irc_pseudoserv_create(void)
{
	return(new MooIRC::PseudoServ());
}

using namespace MooIRC;

static inline int moo_is_channel_name(const char *name)
{
	if (name[0] == '#' || name[0] == '&' || name[0] == '+' || name[0] == '!')
		return(1);
	return(0);
}

PseudoServ::PseudoServ()
{
	m_bits = 0;
	m_nick = NULL;
	m_inter = NULL;
	m_user = NULL;
	m_pass = NULL;
}

PseudoServ::~PseudoServ()
{
	// TODO do all deletey stuff like log user out, remove from channels, etc
	MooChannel::quit(m_user);

	this->set_delete();
	if (m_nick)
		delete m_nick;
	if (m_pass)
		delete m_pass;
	if (m_inter)
		delete m_inter;
	if (m_user)
		m_user->disconnect();
}

int PseudoServ::read_entry(const char *type, MooDataFile *data)
{
/*
	if (!strcmp(type, "port")) {
		m_port = data->read_integer_entry();
		//this->listen(m_port);
	}
	else if (!strcmp(type, "type")) {
		// TODO read in string and find type
	}
	else
		return(MOO_NOT_HANDLED);
*/
	return(MOO_HANDLED);
}


int PseudoServ::write_data(MooDataFile *data)
{
/*
	data->write_integer_entry("port", m_port);
	if (m_itype)
		data->write_string_entry("type", m_itype->m_name);
	if (m_ttype)
		data->write_string_entry("task", m_ttype->m_name);
*/
	return(0);
}


int PseudoServ::initialize()
{
	return(0);
}

int PseudoServ::idle()
{
	return(0);
}

int PseudoServ::release()
{
	return(0);
}

int PseudoServ::notify(int type, MooThing *thing, MooThing *channel, const char *str)
{
	const char *cmd;
	const char *thing_name;
	const char *channel_name;

	if (thing)
		thing_name = thing->get_string_property("name");
	if (channel)
		channel_name = channel->get_string_property("name");

	switch (type) {
	    case TNT_STATUS: {
		char buffer[LARGE_STRING_SIZE];
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		// TODO if realm became a channel, might this instead be used to send notices to the user?
		//return(Msg::send(m_inter, ":TheRealm!realm@%s PRIVMSG #realm :*** %s\r\n", server_name, buffer));
		if (!channel && !thing)
			return(Msg::send(m_inter, ":TheRealm!realm@%s NOTICE %s :*** %s\r\n", server_name, m_nick->c_str(), buffer));
		return(Msg::send(m_inter, ":TheRealm!realm@%s PRIVMSG #realm :*** %s\r\n", server_name, buffer));
		//Msg *msg = new Msg();
	    }
	    case TNT_SAY:
	    case TNT_EMOTE: {
		char buffer[LARGE_STRING_SIZE];

		/// We don't send a message if it was said by our user, since the IRC client will echo that message itself
		if (!thing || thing == m_user)
			return(0);
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (type == TNT_SAY)
			return(Msg::send(m_inter, ":%s!~%s@%s PRIVMSG %s :%s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, (channel && channel_name) ? channel_name : "#realm", buffer));
		else
			return(Msg::send(m_inter, ":%s!~%s@%s PRIVMSG %s :\x01\x41\x43TION %s\x01\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, (channel && channel_name) ? channel_name : "#realm", buffer));
		break;
	    }
	    case TNT_JOIN:
		cmd = "JOIN";
	    case TNT_LEAVE:
		if (type != TNT_JOIN)
			cmd = "PART";
		if (!thing)
			return(0);
		if (thing == m_user) {
			if (type == TNT_JOIN)
				return(this->send_join((channel && channel_name) ? channel_name : "#realm"));
			else
				return(this->send_part((channel && channel_name) ? channel_name : "#realm"));
		}
		else
			return(Msg::send(m_inter, ":%s!~%s@%s %s %s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, cmd, (channel && channel_name) ? channel_name : "#realm"));
	    case TNT_QUIT: {
		char buffer[LARGE_STRING_SIZE];

		if (!thing)
			return(0);
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (thing != m_user)
			return(Msg::send(m_inter, ":%s!~%s@%s QUIT :%s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, buffer));
		break;
	    }
	    default:
		break;
	}
	return(0);
}

int PseudoServ::handle(MooInterface *inter, int ready)
{
	Msg msg;

	if (!(ready & IO_READY_READ))
		return(-1);
	try {
		if (msg.receive(dynamic_cast<MooTCP *>(inter)))
			return(1);
		this->dispatch(&msg);
	}
	catch (MooException e) {
		moo_status("IRC: %s", e.get());
		delete this;
	}
	return(0);
}

int PseudoServ::bestow(MooInterface *inter)
{
	if (m_inter)
		delete m_inter;
	m_inter = dynamic_cast<MooTCP *>(inter);
	m_inter->set_task(this);
	return(0);
}

int PseudoServ::purge(MooInterface *inter)
{
	if (inter != m_inter)
		return(-1);
	/// We assume that we were called because the interface is already being deleted, in which case we don't want to delete it too
	m_inter = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::purge(MooUser *user)
{
	if (user != m_user)
		return(-1);
	/// We assume that since we were called because the user is already being deleted, in which case we don't want to delete too
	m_user = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::dispatch(Msg *msg)
{
	if (msg->need_more_params())
		return(Msg::send(m_inter, ":%s %03d %s :Not enough parameters\r\n", server_name, IRC_ERR_NEEDMOREPARAMS, msg->m_cmdtext));

	/// Process messages that are common for pre and post registration
	switch (msg->cmd()) {
	    case IRC_MSG_PING:
		if (msg->m_numparams != 1 || strcmp(msg->m_params[0], server_name))
			return(Msg::send(m_inter, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[0]));
		Msg::send(m_inter, ":%s PONG %s :%s\r\n", server_name, server_name, server_name);
		return(0);
	    default:
		break;
	}

	if (!this->welcomed()) {
		switch (msg->cmd()) {
		    case IRC_MSG_PASS:
			m_pass = new std::string(msg->m_params[0]);
			break;
		    case IRC_MSG_NICK: {
			if (!MooUser::valid_username(msg->m_params[0]))
				return(Msg::send(m_inter, ":%s %03d %s :Erroneus nickname\r\n", server_name, IRC_ERR_ERRONEUSNICKNAME, msg->m_params[0]));
			if (MooUser::logged_in(msg->m_params[0]))
				return(Msg::send(m_inter, ":%s %03d %s :Nickname is already in use\r\n", server_name, IRC_ERR_NICKNAMEINUSE, msg->m_params[0]));
			if (m_nick)
				delete m_nick;
			m_nick = new std::string(msg->m_params[0]);
			if (this->received_user())
				this->login();
			break;
		    }
		    case IRC_MSG_USER:
			m_bits |= IRC_BF_RECEIVED_USER;
			if (m_nick)
				this->login();
			break;
		    default:
			return(Msg::send(m_inter, ":%s %03d :You have not registered\r\n", server_name, IRC_ERR_NOTREGISTERED));
		}
		return(0);
	}

	/// Process messages that are only acceptable after registration
	switch (msg->cmd()) {
	    case IRC_MSG_PRIVMSG: {
		int res;

		if (!m_user)
			return(Msg::send(m_inter, ":%s NOTICE %s :You aren't logged in yet\r\n", server_name, m_nick->c_str()));
		else if (!msg->m_last)
			return(Msg::send(m_inter, ":%s %03d :No text to send\r\n", server_name, IRC_ERR_NOTEXTTOSEND));
		else if (moo_is_channel_name(msg->m_params[0])) {
			if (!strcmp(msg->m_params[0], "#realm")) {
				if (this->not_in_realm())
					return(Msg::send(m_inter, ":%s %03d #realm :Cannot send to channel\r\n", server_name, IRC_ERR_CANNOTSENDTOCHAN));
				if (msg->m_last[0] == '.')
					res = m_user->command(m_user, NULL, &msg->m_last[1]);
				else if (msg->m_last[0] == '\x01')
					this->process_ctcp(msg, NULL);
				else
					res = m_user->command(m_user, NULL, "say", msg->m_last);

				if (res == MOO_ACTION_NOT_FOUND)
					this->notify(TNT_STATUS, NULL, NULL, "Pardon?");
			}
			else {
				// TODO will the channel list eventually become part of an object?
				//MooThing *channels = MooThing::reference(MOO_CHANNELS);
				MooChannel *channel = MooChannel::get(msg->m_params[0]);
				if (!channel)
					return(Msg::send(m_inter, ":%s %03d %s :Cannot send to channel\r\n", server_name, IRC_ERR_CANNOTSENDTOCHAN, msg->m_params[0]));
				if (msg->m_last[0] == '.') {
					res = channel->do_action(m_user, channel, "evaluate", &msg->m_last[1]);
					if (res == MOO_ACTION_NOT_FOUND)
						this->notify(TNT_STATUS, NULL, NULL, "Pardon?");
				}
				else if (msg->m_last[0] == '\x01')
					this->process_ctcp(msg, channel);
				else
					channel->do_action(m_user, channel, "say", msg->m_last);
			}	
		}
		else {
			// TODO send a private message (probably by calling an action)
			//m_user->command(m_user, NULL, "tell", ???);
		}
		return(0);
	    }
	    case IRC_MSG_MODE: {
		if (moo_is_channel_name(msg->m_params[0])) {
			// TODO temporary, to satisfy irssi
			if (msg->m_numparams > 1 && msg->m_params[1][0] == 'b')
				return(Msg::send(m_inter, ":%s %03d %s %s :End of channel ban list\r\n", server_name, IRC_RPL_ENDOFBANLIST, m_nick->c_str(), msg->m_params[0]));
			// TODO do channel mode command processing
			// TODO the +r here is just to send something back and should be removed later when properly implemented
			return(Msg::send(m_inter, ":%s %03d %s %s +\r\n", server_name, IRC_RPL_CHANNELMODEIS, m_nick->c_str(), msg->m_params[0]));
		}
		else {
			if (strcmp(m_nick->c_str(), msg->m_params[0]))
				return(Msg::send(m_inter, ":%s %03d :Cannot change mode for other users\r\n", server_name, IRC_ERR_USERSDONTMATCH));

			// TODO check for unknown mode flag
			//return(Msg::send(m_inter, ":%s %03d :Unknown MODE flag\r\n", server_name, IRC_ERR_UMODEUNKNOWNFLAG));

			/// User MODE command reply
			// TODO the +i here is just to send something back and should be removed later when properly implemented
			return(Msg::send(m_inter, ":%s %03d %s +i\r\n", server_name, IRC_RPL_UMODEIS, msg->m_params[0]));
		}
		break;
	    }
	    case IRC_MSG_JOIN: {
		if (!strcmp(msg->m_params[0], "0"))
			;// TODO leave all channels
		else {
			/// Cycle through the comma-seperated list of channels to join
			char *name = &msg->m_params[0][0];
			for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
				if (msg->m_params[0][i] == ',') {
					msg->m_params[0][i] = '\0';
					this->handle_join(name);
					msg->m_params[0][i] = ',';
					name = &msg->m_params[0][i + 1];
				}
			}
			this->handle_join(name);
		}
		break;
	    }
	    case IRC_MSG_PART: {
		/// Cycle through the comma-seperated list of channels to leave
		char *name = &msg->m_params[0][0];
		for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
			if (msg->m_params[0][i] == ',') {
				msg->m_params[0][i] = '\0';
				this->handle_leave(name);
				msg->m_params[0][i] = ',';
				name = &msg->m_params[0][i + 1];
			}
		}
		this->handle_leave(name);
		break;
	    }
	    case IRC_MSG_NAMES: {
		// TODO check for ',' in channels names (a list of channels vs just one channel)
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(Msg::send(m_inter, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		this->send_names(msg->m_params[0]);
		return(0);
	    }
	    case IRC_MSG_WHOIS: {
		// TODO do rest of whois
		return(Msg::send(m_inter, ":%s %03d %s :End of WHOIS list\r\n", server_name, IRC_RPL_ENDOFWHOIS, m_nick->c_str()));
	    }
	    case IRC_MSG_QUIT: {
		Msg::send(m_inter, "ERROR :Closing Link: %s[%s] (Quit: )\r\n", m_nick->c_str(), m_inter->host());
		delete this;
		return(0);
	    }
	    case IRC_MSG_WHO: {
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(Msg::send(m_inter, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		this->send_who(msg->m_params[0]);
		return(0);
	    }
	    case IRC_MSG_PASS:
	    case IRC_MSG_USER:
		return(Msg::send(m_inter, ":%s %03d :Unauthorized command (already registered)\r\n", server_name, IRC_ERR_ALREADYREGISTERED));
	    default:
		return(Msg::send(m_inter, ":%s %03d %s :Unknown Command\r\n", server_name, IRC_ERR_UNKNOWNCOMMAND, msg->m_params[0]));
	}

	/*
	MODE
	NICK
	TOPIC
	LIST
	NOTICE
	WALLOPS

	VERSION

	* Possibly temporarily not implement the following:
	INVITE
	KICK
	MOTD
	LUSERS
	STATS
	LINKS
	TIME
	ADMIN
	INFO
	WHO
	WHOIS
	WHOWAS
	AWAY
	USERHOST
	ISON

	* Possibly error appropriately on the following:
	OPER
	SQUIT
	CONNECT
	TRACE
	SERVLIST
	SQUERY
	KILL
	DIE
	RESTART
	REHASH

	Also check errors for which conditions should be caught and reported on
	*/

	return(0);
}

int PseudoServ::handle_join(const char *name)
{
	// TODO check for invite only??
	//	return(Msg::send(m_inter, ":%s %03d %s :Cannot join channel (+i)\r\n", server_name, IRC_ERR_INVITEONLYCHAN, msg->m_params[0]));
	// TODO check for channel limit? is there one?
	//	return(Msg::send(m_inter, ":%s %03d %s :Cannot join channel (+l)\r\n", server_name, IRC_ERR_CHANNELISFULL, msg->m_params[0]));
	// TODO check for too many channels joined?
	//	return(Msg::send(m_inter, ":%s %03d %s :You have joined too many channels\r\n", server_name, IRC_ERR_TOOMANYCHANNELS, msg->m_params[0]));
	// TODO check for banned??
	//	return(Msg::send(m_inter, ":%s %03d %s :Cannot join channel (+b)\r\n", server_name, IRC_ERR_BANNEDFROMCHAN, msg->m_params[0]));

	// TODO what are these messages for??
	//	return(Msg::send(m_inter, ":%s %03d %s :Cannot join channel (+k)\r\n", server_name, IRC_ERR_BADCHANNELKEY, msg->m_params[0]));
	//	return(Msg::send(m_inter, ":%s %03d %s :Bad channel mask\r\n", server_name, IRC_ERR_BADCHANMASK, msg->m_params[0]));
	//	return(Msg::send(m_inter, ":%s %03d %s :<ERRORCODE??> recipients. <ABORTMSG??>\r\n", server_name, IRC_ERR_TOOMANYTARGETS, msg->m_params[0]));
	//	return(Msg::send(m_inter, ":%s %03d %s :Nick/channel is temporarily unavailable\r\n", server_name, IRC_ERR_UNAVAILRESOURCE, msg->m_params[0]));

	if (!strcmp(name, "#realm")) {
		RBIT(m_bits, IRC_BF_NOT_IN_REALM);
		return(this->send_join(name));
	}
	else {
		MooChannel *channel = MooChannel::get(name);
		if (!channel)
			return(Msg::send(m_inter, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
		return(channel->do_action(m_user, channel, "join"));
	}
}

int PseudoServ::handle_leave(const char *name)
{
	if (!strcmp(name, "#realm")) {
		SBIT(m_bits, IRC_BF_NOT_IN_REALM);
		return(this->send_part(name));
	}
	else {
		// TODO should you check for NOTONCHANNEL?  Or should the 'leave' action send that as an error message via notify
		MooChannel *channel = MooChannel::get(name);
		if (!channel)
			return(Msg::send(m_inter, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
		return(channel->do_action(m_user, channel, "leave"));
	}
}

int PseudoServ::login()
{
	char buffer[STRING_SIZE];

	// TODO overwrite buffers used to store password as a security measure

	if (m_pass) {
		/// We received a password so attempt to log in as the user with the network password
		strcpy(buffer, m_pass->c_str());
		MooUser::encrypt_password(m_nick->c_str(), buffer, STRING_SIZE);
		m_user = MooUser::login(m_nick->c_str(), buffer);
		/// Free the password whether it was correct or not so it's not lying around
		delete m_pass;
		m_pass = NULL;
		if (!m_user) {
			Msg::send(m_inter, "ERROR :Closing Link: Invalid password for %s\r\n", m_nick->c_str());
			delete this;
			return(0);
		}
	}
	else {
		/// We received no network password so sign in as a guest
		try {
			if (!(m_user = MooUser::make_guest(m_nick->c_str())))
				throw -1;
		}
		catch (...) {
			Msg::send(m_inter, "ERROR :Closing Link: Unable to connect as guest, %s\r\n", m_nick->c_str());
			delete this;
			return(0);
		}
	}

	if (m_user->connect(this) < 0) {
		Msg::send(m_inter, "ERROR :Closing Link: Error when logging in to %s\r\n", m_nick->c_str());
		delete this;
		return(0);
	}
	this->owner(m_user->id());
	this->send_welcome();
	return(1);
}

int PseudoServ::send_welcome()
{
	Msg::send(m_inter, ":%s %03d %s :Welcome to the Moo IRC Portal %s!~%s@%s\r\n", server_name, IRC_RPL_WELCOME, m_nick->c_str(), m_nick->c_str(), m_nick->c_str(), m_inter->host());
	Msg::send(m_inter, ":%s %03d %s :Your host is %s, running version SuperDuperMoo v%s\r\n", server_name, IRC_RPL_YOURHOST, m_nick->c_str(), server_name, server_version);
	Msg::send(m_inter, ":%s %03d %s :This server was created ???\r\n", server_name, IRC_RPL_CREATED, m_nick->c_str());
	Msg::send(m_inter, ":%s %03d %s :%s SuperDuperMoo v%s ? ?\r\n", server_name, IRC_RPL_MYINFO, m_nick->c_str(), server_name, server_version);
	// TODO you can send the 005 ISUPPORT messages as well (which doesn't appear to be defined in the IRC standard)

	Msg::send(m_inter, ":%s %03d %s :- %s Message of the Day -\r\n", server_name, IRC_RPL_MOTDSTART, m_nick->c_str(), server_name);
	// TODO send a motd (etc/motd.irc)
	Msg::send(m_inter, ":%s %03d %s :End of /MOTD command.\r\n", server_name, IRC_RPL_ENDOFMOTD, m_nick->c_str());
	m_bits |= IRC_BF_WELCOMED;
	if (m_user) {
		this->send_join("#realm");
		Msg::send(m_inter, ":TheRealm!realm@%s NOTICE %s :Welcome to The Realm of the Jabberwock, %s\r\n", server_name, m_nick->c_str(), m_nick->c_str());
	}
	return(0);
}

int PseudoServ::send_join(const char *name)
{
	Msg::send(m_inter, ":%s!~%s@%s JOIN :%s\r\n", m_nick->c_str(), m_nick->c_str(), m_inter->host(), name);
	// TODO send topic
	this->send_names(name);
	return(0);
}

int PseudoServ::send_part(const char *name)
{
	Msg::send(m_inter, ":%s!~%s@%s PART %s\r\n", m_nick->c_str(), m_nick->c_str(), m_inter->host(), name);
	return(0);
}

int PseudoServ::send_names(const char *name)
{
	int j;
	MooThing *cur;
	const char *thing_name;
	char buffer[STRING_SIZE];

	if (!m_user)
		return(-1);

	if (!strcmp(name, "#realm")) {
		// TODO we need some way to check if the room we are currently in cannot list members (you don't want to list members if you
		//	are in the cryolocker, for example)
		cur = m_user->location();
		if (cur)
			cur = cur->contents();
		do {
			j = 0;
			for (int i = 0; cur && i < 20; i++, cur = cur->next()) {
				// TODO we should check that things are invisible, and also add @ for wizards or something
				if ((thing_name = cur->get_string_property("name"))) {
					strcpy(&buffer[j], thing_name);
					j += strlen(thing_name);
					buffer[j++] = ' ';
				}
			}
			if (buffer[j] == ' ')
				j--;
			buffer[j] = '\0';
			// TODO the '=' should be different depending on if it's a secret, private, or public channel
			Msg::send(m_inter, ":%s %03d %s = %s :%s\r\n", server_name, IRC_RPL_NAMREPLY, m_nick->c_str(), name, buffer);
		} while (cur);
	}
	else {
		MooObject *result = NULL;
		MooChannel *channel = MooChannel::get(name);
		if (!channel)
			return(Msg::send(m_inter, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
		channel->do_action(m_user, channel, "names", NULL, &result);
		if (result && result->is_a(&moo_string_obj_type))
			Msg::send(m_inter, ":%s %03d %s = %s :%s\r\n", server_name, IRC_RPL_NAMREPLY, m_nick->c_str(), name, ((MooString *) result)->m_str);

	}
	Msg::send(m_inter, ":%s %03d %s %s :End of NAMES list.\r\n", server_name, IRC_RPL_ENDOFNAMES, m_nick->c_str(), name);
	return(0);
}

int PseudoServ::send_who(const char *mask)
{
	MooThing *cur;
	const char *thing_name;

	if (!strcmp(mask, "#realm")) {
		// TODO we need some way to check if the room we are currently in cannot list members (you don't want to list members if you
		//	are in the cryolocker, for example)
		cur = m_user->location();
		if (cur)
			cur = cur->contents();
		for (; cur; cur = cur->next()) {
			// TODO we should check that things are invisible, and also add @ for wizards or something
			if ((thing_name = cur->get_string_property("name"))) {
				Msg::send(m_inter, ":%s %03d %s %s %s %s %s %s H :0 %s\r\n", server_name, IRC_RPL_WHOREPLY, m_nick->c_str(), mask, thing_name, server_name, server_name, thing_name, thing_name);
			}
		}
	}
	Msg::send(m_inter, ":%s %03d %s %s :End of WHO list.\r\n", server_name, IRC_RPL_ENDOFWHO, m_nick->c_str(), mask);
	return(0);
}

int PseudoServ::process_ctcp(Msg *msg, MooChannel *channel)
{
	if (!strncmp(&msg->m_last[1], "ACTION", 6)) {
		char buffer[STRING_SIZE];
		strncpy(buffer, &msg->m_last[7], STRING_SIZE);
		int len = strlen(buffer);
		buffer[len - 1] = '\0';
		if (channel)
			return(channel->do_action(m_user, channel, "emote", buffer));
		else
			return(m_user->command(m_user, NULL, "emote", buffer));
	}
	// TODO process others?? return error??
	return(0);
}

int irc_write_attrib(int type, char *buffer, int max)
{
	int j = 0;

	if (type & MOO_A_FLASH)
		buffer[j++] = '\x06';
	if (type & MOO_A_BOLD)
		buffer[j++] = '\x02';
	if (type & MOO_A_UNDER)
		buffer[j++] = '\x1F';
	if (type & MOO_A_ITALIC)
		buffer[j++] = '\x09';
	if (type & MOO_A_REVERSE)
		buffer[j++] = '\x22';
	if (type & MOO_A_COLOUR) {
		buffer[j++] = '\x03';
		switch (type & MOO_COLOUR_NUM) {
		    case MOO_C_NORMAL:
			buffer[j++] = '\x30';
			break;
		    case MOO_C_BLACK:
			buffer[j++] = '\x31';
			break;
		    case MOO_C_BLUE:
			buffer[j++] = '\x32';
			break;
		    case MOO_C_GREEN:
			buffer[j++] = '\x33';
			break;
		    case MOO_C_LIGHTRED:
			buffer[j++] = '\x34';
			break;
		    case MOO_C_RED:
			buffer[j++] = '\x35';
			break;
		    case MOO_C_PURPLE:
			buffer[j++] = '\x36';
			break;
		    case MOO_C_ORANGE:
			buffer[j++] = '\x37';
			break;
		    case MOO_C_YELLOW:
			buffer[j++] = '\x38';
			break;
		    case MOO_C_LIGHTGREEN:
			buffer[j++] = '\x39';
			break;
		    case MOO_C_TEAL:
			buffer[j++] = '\x31';
			buffer[j++] = '\x30';
			break;
		    case MOO_C_CYAN:
			buffer[j++] = '\x31';
			buffer[j++] = '\x31';
			break;
		    case MOO_C_LIGHTBLUE:
			buffer[j++] = '\x31';
			buffer[j++] = '\x32';
			break;
		    case MOO_C_MAGENTA:
			buffer[j++] = '\x31';
			buffer[j++] = '\x33';
			break;
		    case MOO_C_DARKGREY:
			buffer[j++] = '\x31';
			buffer[j++] = '\x34';
			break;
		    case MOO_C_GREY:
			buffer[j++] = '\x31';
			buffer[j++] = '\x35';
			break;
		    case MOO_C_WHITE:
			buffer[j++] = '\x31';
			buffer[j++] = '\x36';
			break;
		    default:
			break;
		}
	}
	return(j);
}


