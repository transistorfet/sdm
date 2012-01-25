/*
 * Module Name:	pseudoserv.c
 * Description:	IRC PseudoServ Interface
 */

#include <string.h>
#include <stdarg.h>

#include <sdm/colours.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>

#include <sdm/drivers/driver.h>
#include <sdm/drivers/tcp.h>
#include <sdm/objs/thing.h>

#include <sdm/lib/users.h>
#include <sdm/lib/irc/msg.h>
#include <sdm/lib/irc/pseudoserv.h>

time_t server_start = 0;
const char *server_name = "moo.jabberwocky.ca";
const char *server_version = "0.1";

int irc_write_attrib(int type, char *buffer, int max);

int init_irc_pseudoserv(void)
{
	server_start = time(NULL);
	return(0);
}

void release_irc_pseudoserv(void)
{

}

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
	m_driver = NULL;
	m_user = NULL;
	m_pass = NULL;
}

PseudoServ::~PseudoServ()
{
	this->set_delete();
	if (m_user)
		m_user->call_method(NULL, "disconnect", NULL);
	if (m_nick)
		delete m_nick;
	if (m_pass)
		delete m_pass;
	if (m_driver)
		delete m_driver;
}

int PseudoServ::handle(MooDriver *driver, int ready)
{
	IRCMsg msg;

	if (!(ready & IO_READY_READ))
		return(-1);
	try {
		if (msg.receive(dynamic_cast<MooTCP *>(driver)))
			return(1);
		this->dispatch(&msg);
	}
	catch (MooException e) {
		moo_status("IRC: %s", e.get());
		delete this;
	}
	return(0);
}

int PseudoServ::bestow(MooDriver *driver)
{
	if (m_driver)
		delete m_driver;
	m_driver = dynamic_cast<MooTCP *>(driver);
	m_driver->set_task(this);
	return(0);
}

int PseudoServ::purge(MooDriver *driver)
{
	if (driver != m_driver)
		return(-1);
	/// We assume that we were called because the interface is already being deleted, in which case we don't want to delete it too
	m_driver = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::purge(MooThing *user)
{
	if (user != m_user)
		return(-1);
	/// We assume that since we were called because the user is already being deleted, in which case we don't want to delete too
	m_user = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::notify(int type, MooThing *thing, MooThing *channel, const char *str)
{
	const char *cmd;
	const char *thing_name = NULL;
	const char *channel_name = NULL;

	if (!m_driver)
		return(0);

	if (thing)
		thing_name = thing->name();
	if (channel)
		channel_name = channel->name();

	switch (type) {
	    case TNT_STATUS: {
		char buffer[LARGE_STRING_SIZE];
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (!channel_name)
			return(IRCMsg::send(m_driver, ":TheRealm!realm@%s NOTICE %s :*** %s\r\n", server_name, m_nick->c_str(), buffer));
		else
			return(IRCMsg::send(m_driver, ":TheRealm!realm@%s PRIVMSG %s :*** %s\r\n", server_name, channel_name, buffer));
	    }
	    case TNT_SAY:
	    case TNT_EMOTE: {
		char buffer[LARGE_STRING_SIZE];

		/// We don't send a message if it was said by our user, since the IRC client will echo that message itself
		if (!thing || thing == m_user || !channel_name)
			return(0);
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (type == TNT_SAY)
			return(IRCMsg::send(m_driver, ":%s!~%s@%s PRIVMSG %s :%s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, channel_name, buffer));
		else
			return(IRCMsg::send(m_driver, ":%s!~%s@%s PRIVMSG %s :\x01\x41\x43TION %s\x01\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, channel_name, buffer));
		break;
	    }
	    case TNT_JOIN:
		cmd = "JOIN";
	    case TNT_LEAVE:
		if (type != TNT_JOIN)
			cmd = "PART";
		if (!thing || !channel_name)
			return(0);
		if (thing == m_user) {
			if (type == TNT_JOIN)
				return(this->send_join(channel_name));
			else
				return(this->send_part(channel_name));
		}
		else
			return(IRCMsg::send(m_driver, ":%s!~%s@%s %s %s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, cmd, channel_name));
	    case TNT_QUIT: {
		char buffer[LARGE_STRING_SIZE];

		if (!thing)
			return(0);
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (thing != m_user)
			return(IRCMsg::send(m_driver, ":%s!~%s@%s QUIT :%s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, buffer));
		break;
	    }
	    default:
		break;
	}
	return(0);
}

int PseudoServ::dispatch(IRCMsg *msg)
{
	if (msg->need_more_params())
		return(IRCMsg::send(m_driver, ":%s %03d %s :Not enough parameters\r\n", server_name, IRC_ERR_NEEDMOREPARAMS, msg->m_cmdtext));

	/// Process messages that are common for pre and post registration
	switch (msg->cmd()) {
	    case IRC_MSG_PING:
		if (msg->m_numparams != 1)
			return(IRCMsg::send(m_driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[0]));
		IRCMsg::send(m_driver, ":%s PONG %s :%s\r\n", server_name, server_name, msg->m_params[0]);
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
			if (!user_valid_username(msg->m_params[0]))
				return(IRCMsg::send(m_driver, ":%s %03d %s :Erroneus nickname\r\n", server_name, IRC_ERR_ERRONEUSNICKNAME, msg->m_params[0]));
			if (user_logged_in(msg->m_params[0]))
				return(IRCMsg::send(m_driver, ":%s %03d %s :Nickname is already in use\r\n", server_name, IRC_ERR_NICKNAMEINUSE, msg->m_params[0]));
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
			return(IRCMsg::send(m_driver, ":%s %03d :You have not registered\r\n", server_name, IRC_ERR_NOTREGISTERED));
		}
		return(0);
	}

	/// Process messages that are only acceptable after registration
	switch (msg->cmd()) {
	    case IRC_MSG_PRIVMSG: {
		int res;

		if (!m_user)
			return(IRCMsg::send(m_driver, ":%s NOTICE %s :You aren't logged in yet\r\n", server_name, m_nick->c_str()));
		else if (!msg->m_last)
			return(IRCMsg::send(m_driver, ":%s %03d :No text to send\r\n", server_name, IRC_ERR_NOTEXTTOSEND));
		else {
			MooThing *channel;

			if (moo_is_channel_name(msg->m_params[0]))
				channel = MooThing::get_channel(msg->m_params[0]);
			// TODO fix/add user-to-user privmsging
			//else
				// TODO i don't think this really makes sense anymore.  It should maybe be a special case
				//channel = MooUser::get(msg->m_params[0]);

			if (!channel)
				return(IRCMsg::send(m_driver, ":%s %03d %s :Cannot send to channel\r\n", server_name, IRC_ERR_CANNOTSENDTOCHAN, msg->m_params[0]));
			if (msg->m_last[0] == '.') {
				//res = channel->call_method(channel, "command", NULL, new MooString("%s", &msg->m_last[1]));
				//if (res == MOO_ACTION_NOT_FOUND)
				//	this->notify(TNT_STATUS, NULL, channel, "Pardon?");
				MooObject *func;
				MooTask *task;

				task = new MooTask(m_user, channel);
				if (!(func = channel->resolve_method("command")))
					return(-1);
				task->push_call(func, channel, new MooString("%s", &msg->m_last[1]));
				task->schedule(0);
			}
			else if (msg->m_last[0] == '\x01')
				this->process_ctcp(msg, channel);
			else
				channel->call_method(channel, "say", NULL, new MooString("%s", msg->m_last));
		}
		return(0);
	    }
	    case IRC_MSG_MODE: {
		if (moo_is_channel_name(msg->m_params[0])) {
			// TODO temporary, to satisfy irssi
			if (msg->m_numparams > 1 && msg->m_params[1][0] == 'b')
				return(IRCMsg::send(m_driver, ":%s %03d %s %s :End of channel ban list\r\n", server_name, IRC_RPL_ENDOFBANLIST, m_nick->c_str(), msg->m_params[0]));
			// TODO do channel mode command processing
			// TODO the +r here is just to send something back and should be removed later when properly implemented
			return(IRCMsg::send(m_driver, ":%s %03d %s %s +\r\n", server_name, IRC_RPL_CHANNELMODEIS, m_nick->c_str(), msg->m_params[0]));
		}
		else {
			if (strcmp(m_nick->c_str(), msg->m_params[0]))
				return(IRCMsg::send(m_driver, ":%s %03d :Cannot change mode for other users\r\n", server_name, IRC_ERR_USERSDONTMATCH));

			// TODO check for unknown mode flag
			//return(IRCMsg::send(m_driver, ":%s %03d :Unknown MODE flag\r\n", server_name, IRC_ERR_UMODEUNKNOWNFLAG));

			/// User MODE command reply
			// TODO the +i here is just to send something back and should be removed later when properly implemented
			return(IRCMsg::send(m_driver, ":%s %03d %s +i\r\n", server_name, IRC_RPL_UMODEIS, msg->m_params[0]));
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
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(IRCMsg::send(m_driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		/// Cycle through the comma-seperated list of channels to leave
		char *name = &msg->m_params[0][0];
		for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
			if (msg->m_params[0][i] == ',') {
				msg->m_params[0][i] = '\0';
				this->send_names(name);
				msg->m_params[0][i] = ',';
				name = &msg->m_params[0][i + 1];
			}
		}
		this->send_names(name);
		return(0);
	    }
	    case IRC_MSG_WHOIS: {
		// TODO do rest of whois
		return(IRCMsg::send(m_driver, ":%s %03d %s :End of WHOIS list\r\n", server_name, IRC_RPL_ENDOFWHOIS, m_nick->c_str()));
	    }
	    case IRC_MSG_QUIT: {
		if (m_user)
			m_user->quit();
		IRCMsg::send(m_driver, "ERROR :Closing Link: %s[%s] (Quit: )\r\n", m_nick->c_str(), m_driver->host());
		delete this;
		return(0);
	    }
	    case IRC_MSG_WHO: {
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(IRCMsg::send(m_driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		this->send_who(msg->m_params[0]);
		return(0);
	    }
	    case IRC_MSG_LIST: {
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(IRCMsg::send(m_driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		/// Cycle through the comma-seperated list of channels to leave
		char *name = &msg->m_params[0][0];
		for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
			if (msg->m_params[0][i] == ',') {
				msg->m_params[0][i] = '\0';
				this->send_list(name);
				msg->m_params[0][i] = ',';
				name = &msg->m_params[0][i + 1];
			}
		}
		this->send_list(name);
		return(0);
	    }
	    case IRC_MSG_PASS:
	    case IRC_MSG_USER:
		return(IRCMsg::send(m_driver, ":%s %03d :Unauthorized command (already registered)\r\n", server_name, IRC_ERR_ALREADYREGISTERED));
	    default:
		return(IRCMsg::send(m_driver, ":%s %03d %s :Unknown Command\r\n", server_name, IRC_ERR_UNKNOWNCOMMAND, msg->m_params[0]));
	}

	/*
	MODE
	NICK
	TOPIC
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
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :Cannot join channel (+i)\r\n", server_name, IRC_ERR_INVITEONLYCHAN, msg->m_params[0]));
	// TODO check for channel limit? is there one?
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :Cannot join channel (+l)\r\n", server_name, IRC_ERR_CHANNELISFULL, msg->m_params[0]));
	// TODO check for too many channels joined?
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :You have joined too many channels\r\n", server_name, IRC_ERR_TOOMANYCHANNELS, msg->m_params[0]));
	// TODO check for banned??
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :Cannot join channel (+b)\r\n", server_name, IRC_ERR_BANNEDFROMCHAN, msg->m_params[0]));

	// TODO what are these messages for??
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :Cannot join channel (+k)\r\n", server_name, IRC_ERR_BADCHANNELKEY, msg->m_params[0]));
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :Bad channel mask\r\n", server_name, IRC_ERR_BADCHANMASK, msg->m_params[0]));
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :<ERRORCODE??> recipients. <ABORTMSG??>\r\n", server_name, IRC_ERR_TOOMANYTARGETS, msg->m_params[0]));
	//	return(IRCMsg::send(m_driver, ":%s %03d %s :Nick/channel is temporarily unavailable\r\n", server_name, IRC_ERR_UNAVAILRESOURCE, msg->m_params[0]));

	MooThing *channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(m_driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	return(channel->call_method(channel, "join", NULL));
}

int PseudoServ::handle_leave(const char *name)
{
	MooObjectArray *users;

	MooThing *channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(m_driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	if ((users = dynamic_cast<MooObjectArray *>(channel->resolve_property("users")))) {
		if (users->search(m_user) < 0)
			return(IRCMsg::send(m_driver, ":%s %03d %s :You're not on that channel\r\n", server_name, IRC_ERR_NOTONCHANNEL, name));
	}
	return(channel->call_method(channel, "leave", NULL));
}

int PseudoServ::login()
{
	char buffer[STRING_SIZE];

	// TODO overwrite buffers used to store password as a security measure

	if (m_pass) {
		/// We received a password so attempt to log in as the user with the network password
		strcpy(buffer, m_pass->c_str());
		user_encrypt_password(m_nick->c_str(), buffer, STRING_SIZE);
		m_user = user_login(m_nick->c_str(), buffer);
		/// Free the password whether it was correct or not so it's not lying around
		delete m_pass;
		m_pass = NULL;
		if (!m_user) {
			IRCMsg::send(m_driver, "ERROR :Closing Link: Invalid password for %s\r\n", m_nick->c_str());
			delete this;
			return(0);
		}
	}
	else {
		/// We received no network password so sign in as a guest
		try {
			if (!(m_user = user_make_guest(m_nick->c_str())))
				throw MooException("Unable to connect as guest");
		}
		catch (MooException e) {
			IRCMsg::send(m_driver, "ERROR :Closing Link: (%s) %s\r\n", m_nick->c_str(), e.get());
			delete this;
			return(0);
		}
	}

	if (m_user->call_method(NULL, "connect", NULL, this) < 0) {
		IRCMsg::send(m_driver, "ERROR :Closing Link: Error when logging in to %s\r\n", m_nick->c_str());
		delete this;
		return(0);
	}
	this->owner(m_user->id());
	this->send_welcome();
	return(1);
}

int PseudoServ::send_welcome()
{
	IRCMsg::send(m_driver, ":%s %03d %s :Welcome to the Moo IRC Portal %s!~%s@%s\r\n", server_name, IRC_RPL_WELCOME, m_nick->c_str(), m_nick->c_str(), m_nick->c_str(), m_driver->host());
	IRCMsg::send(m_driver, ":%s %03d %s :Your host is %s, running version SuperDuperMoo v%s\r\n", server_name, IRC_RPL_YOURHOST, m_nick->c_str(), server_name, server_version);
	IRCMsg::send(m_driver, ":%s %03d %s :This server was created ???\r\n", server_name, IRC_RPL_CREATED, m_nick->c_str());
	IRCMsg::send(m_driver, ":%s %03d %s :%s SuperDuperMoo v%s ? ?\r\n", server_name, IRC_RPL_MYINFO, m_nick->c_str(), server_name, server_version);
	// TODO you can send the 005 ISUPPORT messages as well (which doesn't appear to be defined in the IRC standard)

	this->send_motd();
	m_bits |= IRC_BF_WELCOMED;
	if (m_user) {
		// TODO remove this eventually??  It should be able to work with it in though, so make sure there are no bugs currently
		this->handle_join("#realm");
		//IRCMsg::send(m_driver, ":TheRealm!realm@%s NOTICE %s :Welcome to The Realm of the Jabberwock, %s\r\n", server_name, m_nick->c_str(), m_nick->c_str());
	}
	return(0);
}

int PseudoServ::send_motd()
{
	char ch;
	int len, j = 0;
	char buffer[LARGE_STRING_SIZE];

	IRCMsg::send(m_driver, ":%s %03d %s :- %s Message of the Day -\r\n", server_name, IRC_RPL_MOTDSTART, m_nick->c_str(), server_name);
	len = moo_data_read_file("etc/motd.txt", buffer, LARGE_STRING_SIZE);
	for (int i = 0; i <= len; i++) {
		if (buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == '\0') {
			ch = buffer[i];
			if (i - j > 80)
				buffer[j + 80] = '\0';
			else
				buffer[i] = '\0';
			IRCMsg::send(m_driver, ":%s %03d %s :- %s\r\n", server_name, IRC_RPL_MOTD, m_nick->c_str(), &buffer[j]);
			if (ch == '\r' && buffer[i + 1] == '\n')
				i++;
			j = i + 1;
		}
	}
	IRCMsg::send(m_driver, ":%s %03d %s :End of /MOTD command.\r\n", server_name, IRC_RPL_ENDOFMOTD, m_nick->c_str());
	return(0);
}

int PseudoServ::send_join(const char *name)
{
	IRCMsg::send(m_driver, ":%s!~%s@%s JOIN :%s\r\n", m_nick->c_str(), m_nick->c_str(), m_driver->host(), name);
	// TODO send topic
	this->send_names(name);
	return(0);
}

int PseudoServ::send_part(const char *name)
{
	IRCMsg::send(m_driver, ":%s!~%s@%s PART %s\r\n", m_nick->c_str(), m_nick->c_str(), m_driver->host(), name);
	return(0);
}

int PseudoServ::send_names(const char *name)
{
	MooString *names;
	MooThing *channel;
	MooObject *result = NULL;

	// TODO should we do this like we do WHO instead of having the special 'names' method?
	if (!m_user)
		return(-1);
	channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(m_driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	channel->call_method(channel, "names", &result);
	// TODO break into smaller chunks to guarentee the end message is less than 512 bytes
	// TODO the '=' should be different depending on if it's a secret, private, or public channel
	if (result && (names = dynamic_cast<MooString *>(result)))
		IRCMsg::send(m_driver, ":%s %03d %s = %s :%s\r\n", server_name, IRC_RPL_NAMREPLY, m_nick->c_str(), name, names->get_string());
	IRCMsg::send(m_driver, ":%s %03d %s %s :End of NAMES list.\r\n", server_name, IRC_RPL_ENDOFNAMES, m_nick->c_str(), name);
	MOO_DECREF(result);
	return(0);
}

int PseudoServ::send_who(const char *mask)
{
	MooObject *cur;
	const char *thing_name;
	MooThing *channel;
	MooObjectArray *users;

	// TODO should you make this do more than just list channels? (like work with users and stuff as well)
	channel = MooThing::get_channel(mask);
	if (channel) {
		// TODO we need some way to check if the room we are currently in cannot list members (you don't want to list members if you
		//	are in the cryolocker, for example)
		if ((users = dynamic_cast<MooObjectArray *>(channel->resolve_property("users")))) {
			for (int i = 0; i <= users->last(); i++) {
				cur = users->get(i);
				// TODO we should check that things are invisible, and also add @ for wizards or something
				if ((cur = cur->resolve_property("name")) && (thing_name = cur->get_string())) {
					IRCMsg::send(m_driver, ":%s %03d %s %s %s %s %s %s H :0 %s\r\n", server_name, IRC_RPL_WHOREPLY, m_nick->c_str(), mask, thing_name, server_name, server_name, thing_name, thing_name);
				}
			}
		}
	}
	IRCMsg::send(m_driver, ":%s %03d %s %s :End of WHO list.\r\n", server_name, IRC_RPL_ENDOFWHO, m_nick->c_str(), mask);
	return(0);
}

int PseudoServ::send_list(const char *name)
{
	const char *str;
	MooObjectHash *list;
	MooObject *channels, *cur, *obj;

	// TODO accessing the db directly isn't really correct here, we should either call a method, evaluate direct code (but which
	//	would allow easy use of a method on chanserv), or something to put the actual db access into a method on ChanServ
	if ((channels = MooObject::resolve("ChanServ", global_env))) {
		if ((list = dynamic_cast<MooObjectHash *>(channels->resolve_property("db")))) {
			list->reset();
			while ((cur = list->next())) {
				if ((obj = cur->resolve_property("name")) && (str = obj->get_string()))
					IRCMsg::send(m_driver, ":%s %03d %s %s 1 :\r\n", server_name, IRC_RPL_LIST, m_nick->c_str(), str);
			}
		}
	}

	IRCMsg::send(m_driver, ":%s %03d %s :End of LIST.\r\n", server_name, IRC_RPL_ENDOFLIST, m_nick->c_str());
	return(0);
}

int PseudoServ::process_ctcp(IRCMsg *msg, MooThing *channel)
{
	if (!strncmp(&msg->m_last[1], "ACTION", 6)) {
		char buffer[STRING_SIZE];
		strncpy(buffer, (msg->m_last[7] == ' ') ? &msg->m_last[8] : &msg->m_last[7], STRING_SIZE);
		int len = strlen(buffer);
		buffer[len - 1] = '\0';
		if (channel)
			return(channel->call_method(channel, "emote", NULL, new MooString("%s", buffer)));
		else
			return(m_user->call_method(NULL, "emote", NULL, new MooString("%s", buffer)));
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


