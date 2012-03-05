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


// TODO where would this go? it was in the destructor for pseudoserv
//	if (m_user)
//		m_user->call_method(NULL, "disconnect", NULL);


int irc_pseudoserv_notify(MooCodeFrame *frame, MooObjectArray *args)
//int irc_notify(MooCodeFrame *frame, MooTCP *driver, int type, MooThing *thing, MooThing *channel, const char *str)
{
	MooTCP *driver;
	MooObject *obj;
	const char *cmd;
	const char *nick;
	const char *thing_name = NULL;
	const char *channel_name = NULL;
	MooThing *m_this, *user, *channel;

	if (args->last() != 1)
		throw MooException("Expected 2 arguments; given %d", args->last() + 1);
	if (!(m_this = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (!(user = dynamic_cast<MooThing *>(args->get(1))))
		throw MooException("IRC: arg 1: Invalid user, expected thing");
	if (!(driver = dynamic_cast<MooTCP *>(user->resolve_property("conn"))))
		throw MooException("IRC: Invalid user.conn, expected MooTCP driver");
	if (!(obj = user->resolve_property("name")) || !(nick = obj->get_string()))
		throw MooException("IRC: Invalid user.name, notify failed");

	int type;
	//MooTask *task;
	char buffer[LARGE_STRING_SIZE];
/*

	if (!(m_this = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (!(task = dynamic_cast<MooTask *>(m_this->resolve_property("task"))))
		throw MooException("User not connected; No \"task\" defined.");
	if (args->last() != 4)
		throw moo_args_mismatched;
	type = args->get_integer(1);
	user = args->get_thing(2);
	channel = args->get_thing(3);
	if (!(obj = args->get(4)))
		return(-1);
	obj->to_string(buffer, LARGE_STRING_SIZE);
	if (type < TNT_FIRST || type > TNT_LAST)
		throw MooException("arg 1: Invalid notify() type");
	// TODO permissions check!? I guess on task
	task->notify(type, user, channel, buffer);
*/


/*
	// TODO fix this later
	if (thing)
		thing_name = thing->name();
	if (channel)
		channel_name = channel->name();

	switch (type) {
	    case TNT_STATUS: {
		char buffer[LARGE_STRING_SIZE];
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (!channel_name)
			return(IRCMsg::send(driver, ":TheRealm!realm@%s NOTICE %s :*** %s\r\n", server_name, nick, buffer));
		else
			return(IRCMsg::send(driver, ":TheRealm!realm@%s PRIVMSG %s :*** %s\r\n", server_name, channel_name, buffer));
	    }
	    case TNT_SAY:
	    case TNT_EMOTE: {
		char buffer[LARGE_STRING_SIZE];

		/// We don't send a message if it was said by our user, since the IRC client will echo that message itself
		if (!thing || thing == m_user || !channel_name)
			return(0);
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (type == TNT_SAY)
			return(IRCMsg::send(driver, ":%s!~%s@%s PRIVMSG %s :%s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, channel_name, buffer));
		else
			return(IRCMsg::send(driver, ":%s!~%s@%s PRIVMSG %s :\x01\x41\x43TION %s\x01\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, channel_name, buffer));
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
				return(irc_send_join(frame, driver, nick, channel_name));
			else
				return(irc_send_part(frame, driver, nick, channel_name));
		}
		else
			return(IRCMsg::send(driver, ":%s!~%s@%s %s %s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, cmd, channel_name));
	    case TNT_QUIT: {
		char buffer[LARGE_STRING_SIZE];

		if (!thing)
			return(0);
		moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
		if (thing != m_user)
			return(IRCMsg::send(driver, ":%s!~%s@%s QUIT :%s\r\n", thing_name ? thing_name : "Unknown", thing_name ? thing_name : "realm", server_name, buffer));
		break;
	    }
	    default:
		break;
	}
*/
	return(0);
}


int irc_pseudoserv_send_status(MooCodeFrame *frame, MooObjectArray *args)
{
	MooTCP *driver;
	MooObject *obj;
	char buffer[LARGE_STRING_SIZE];
	const char *nick, *channel_name, *str;
	MooThing *m_this, *user, *channel;

	if (args->last() != 3)
		throw MooException("IRC: Expected 4 arguments; given %d", args->last() + 1);
	if (!(m_this = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (!(user = dynamic_cast<MooThing *>(args->get(1))))
		throw MooException("IRC: arg 1: Invalid user, expected thing type");
	if (!(channel = dynamic_cast<MooThing *>(args->get(2))))
		throw MooException("IRC: arg 2: Invalid channel, expected thing type");
	if (!(str = args->get_string(3)))
		throw MooException("IRC: arg 3: Expected string type");
	if (!(driver = dynamic_cast<MooTCP *>(user->resolve_property("conn"))))
		throw MooException("IRC: Invalid user.conn, expected MooTCP driver");
	if (!(obj = user->resolve_property("name")) || !(nick = obj->get_string()))
		throw MooException("IRC: Invalid user.name");
	if (!(obj = channel->resolve_property("name")) || !(channel_name = obj->get_string()))
		throw MooException("IRC: Invalid channel.name");

	moo_colour_format(&irc_write_attrib, buffer, LARGE_STRING_SIZE, str);
	if (!channel_name)
		return(IRCMsg::send(driver, ":TheRealm!realm@%s NOTICE %s :*** %s\r\n", server_name, nick, buffer));
	else
		return(IRCMsg::send(driver, ":TheRealm!realm@%s PRIVMSG %s :*** %s\r\n", server_name, channel_name, buffer));
}

int irc_pseudoserv_process(MooCodeFrame *frame, MooObjectArray *args)
{
	IRCMsg msg;
	MooTCP *driver;
	MooThing *m_this;

	if (args->last() != 1)
		throw moo_args_mismatched;
	if (!(m_this = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (!(driver = dynamic_cast<MooTCP *>(args->get(1))))
		throw MooException("Expected MooTCP driver");
	//if (!(ready & IO_READY_READ))
	//	return(-1);
	if (msg.receive(driver))
		return(1);
	irc_dispatch(frame, m_this, driver, &msg);
	return(0);
}

int irc_dispatch(MooCodeFrame *frame, MooThing *m_this, MooTCP *driver, IRCMsg *msg)
{
	MooThing *user;
	MooObject *obj;
	const char *nick;
	MooObjectHash *env;

	if (msg->need_more_params())
		return(IRCMsg::send(driver, ":%s %03d %s :Not enough parameters\r\n", server_name, IRC_ERR_NEEDMOREPARAMS, msg->m_cmdtext));

	/// Process messages that are common for pre and post registration
	switch (msg->cmd()) {
	    case IRC_MSG_PING:
		if (msg->m_numparams != 1)
			return(IRCMsg::send(driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[0]));
		IRCMsg::send(driver, ":%s PONG %s :%s\r\n", server_name, server_name, msg->m_params[0]);
		return(0);
	    default:
		break;
	}

	env = frame->env();
	user = dynamic_cast<MooThing *>(frame->resolve("user"));
	if (!user) {
		switch (msg->cmd()) {
		    case IRC_MSG_PASS: {
			char buffer[STRING_SIZE];
			user_encrypt_password(msg->m_params[0], buffer, STRING_SIZE);
			buffer[STRING_SIZE - 1] = '\0';
			env->set("password", new MooString("%s", buffer));
			break;
		    }
		    case IRC_MSG_NICK: {
			if (!user_valid_username(msg->m_params[0]))
				return(IRCMsg::send(driver, ":%s %03d %s :Erroneus nickname\r\n", server_name, IRC_ERR_ERRONEUSNICKNAME, msg->m_params[0]));
			if (user_logged_in(msg->m_params[0]))
				return(IRCMsg::send(driver, ":%s %03d %s :Nickname is already in use\r\n", server_name, IRC_ERR_NICKNAMEINUSE, msg->m_params[0]));
			obj = new MooString("%s", msg->m_params[0]);
			env->set("nick", obj);
			if (env->get("realname"))
				irc_login(frame, driver, env);
			break;
		    }
		    case IRC_MSG_USER:
			env->set("realname", new MooString("%s", msg->m_params[3]));
			if (env->get("nick"))
				irc_login(frame, driver, env);
			break;
		    case IRC_MSG_PRIVMSG: {
			// TODO this is where the restricted unauthenticated stuff gets done (you can only log in and register)
			//	but I guess really we'll just forward all the messages to and from NickServ or else give the error
			break;
		    }
		    default:
			return(IRCMsg::send(driver, ":%s %03d :You have not registered\r\n", server_name, IRC_ERR_NOTREGISTERED));
		}
		return(0);
	}

	// TODO should you change name to username??
	obj = user->resolve_property("name");
	nick = obj->get_string();

	/// Process messages that are only acceptable after registration
	switch (msg->cmd()) {
	    case IRC_MSG_PRIVMSG: {
		int res;

		if (!user)
			return(IRCMsg::send(driver, ":%s NOTICE %s :You aren't logged in yet\r\n", server_name, nick));
		else if (!msg->m_last)
			return(IRCMsg::send(driver, ":%s %03d :No text to send\r\n", server_name, IRC_ERR_NOTEXTTOSEND));
		else {
			MooThing *channel;

			if (moo_is_channel_name(msg->m_params[0]))
				channel = MooThing::get_channel(msg->m_params[0]);
			// TODO fix/add user-to-user privmsging
			//else
				// TODO i don't think this really makes sense anymore.  It should maybe be a special case
				//channel = MooUser::get(msg->m_params[0]);

			if (!channel)
				return(IRCMsg::send(driver, ":%s %03d %s :Cannot send to channel\r\n", server_name, IRC_ERR_CANNOTSENDTOCHAN, msg->m_params[0]));
			if (msg->m_last[0] == '.') {
				//res = channel->call_method(channel, "command", NULL, new MooString("%s", &msg->m_last[1]));
				//if (res == MOO_ACTION_NOT_FOUND)
				//	this->notify(TNT_STATUS, NULL, channel, "Pardon?");
				MooCodeFrame *frame;

				/// The new frame will extend the current environment as it's base environment
				frame = new MooCodeFrame(frame->env());
				frame->push_method_call("command", channel, new MooString("%s", &msg->m_last[1]));
				frame->schedule(0);
				frame = NULL;
			}
			else if (msg->m_last[0] == '\x01')
				irc_process_ctcp(frame, driver, msg, user, channel);
			else
				frame->push_method_call("say", channel, new MooString("%s", msg->m_last));
		}
		return(0);
	    }
	    case IRC_MSG_MODE: {
		if (moo_is_channel_name(msg->m_params[0])) {
			// TODO temporary, to satisfy irssi
			if (msg->m_numparams > 1 && msg->m_params[1][0] == 'b')
				return(IRCMsg::send(driver, ":%s %03d %s %s :End of channel ban list\r\n", server_name, IRC_RPL_ENDOFBANLIST, nick, msg->m_params[0]));
			// TODO do channel mode command processing
			// TODO the +r here is just to send something back and should be removed later when properly implemented
			return(IRCMsg::send(driver, ":%s %03d %s %s +\r\n", server_name, IRC_RPL_CHANNELMODEIS, nick, msg->m_params[0]));
		}
		else {
			if (strcmp(nick, msg->m_params[0]))
				return(IRCMsg::send(driver, ":%s %03d :Cannot change mode for other users\r\n", server_name, IRC_ERR_USERSDONTMATCH));

			// TODO check for unknown mode flag
			//return(IRCMsg::send(driver, ":%s %03d :Unknown MODE flag\r\n", server_name, IRC_ERR_UMODEUNKNOWNFLAG));

			/// User MODE command reply
			// TODO the +i here is just to send something back and should be removed later when properly implemented
			return(IRCMsg::send(driver, ":%s %03d %s +i\r\n", server_name, IRC_RPL_UMODEIS, msg->m_params[0]));
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
					irc_handle_join(frame, driver, name);
					msg->m_params[0][i] = ',';
					name = &msg->m_params[0][i + 1];
				}
			}
			irc_handle_join(frame, driver, name);
		}
		break;
	    }
	    case IRC_MSG_PART: {
		/// Cycle through the comma-seperated list of channels to leave
		char *name = &msg->m_params[0][0];
		for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
			if (msg->m_params[0][i] == ',') {
				msg->m_params[0][i] = '\0';
				irc_handle_leave(frame, driver, user, name);
				msg->m_params[0][i] = ',';
				name = &msg->m_params[0][i + 1];
			}
		}
		irc_handle_leave(frame, driver, user, name);
		break;
	    }
	    case IRC_MSG_NAMES: {
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(IRCMsg::send(driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		/// Cycle through the comma-seperated list of channels to list the names of
		char *name = &msg->m_params[0][0];
		for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
			if (msg->m_params[0][i] == ',') {
				msg->m_params[0][i] = '\0';
				irc_send_names(frame, driver, nick, name);
				msg->m_params[0][i] = ',';
				name = &msg->m_params[0][i + 1];
			}
		}
		irc_send_names(frame, driver, nick, name);
		return(0);
	    }
	    case IRC_MSG_WHOIS: {
		// TODO do rest of whois
		return(IRCMsg::send(driver, ":%s %03d %s :End of WHOIS list\r\n", server_name, IRC_RPL_ENDOFWHOIS, nick));
	    }
	    case IRC_MSG_QUIT: {
		if (user) {
			MooObject *channels;
			if ((channels = frame->resolve("ChanServ")))
				frame->push_method_call("quit", channels);
		}
		// TODO you can't do this now until after you process the quit (right??)
		IRCMsg::send(driver, "ERROR :Closing Link: %s[%s] (Quit: )\r\n", nick, driver->host());
		return(0);
	    }
	    case IRC_MSG_WHO: {
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(IRCMsg::send(driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		irc_send_who(frame, driver, nick, msg->m_params[0]);
		return(0);
	    }
	    case IRC_MSG_LIST: {
		if (msg->m_numparams > 1 && !strcmp(msg->m_params[1], server_name))
			return(IRCMsg::send(driver, ":%s %03d %s :No such server\r\n", server_name, IRC_ERR_NOSUCHSERVER, msg->m_params[1]));
		/// Cycle through the comma-seperated list of channels to leave
		char *name = &msg->m_params[0][0];
		for (int i = 0; msg->m_params[0][i] != '\0'; i++) {
			if (msg->m_params[0][i] == ',') {
				msg->m_params[0][i] = '\0';
				irc_send_list(frame, driver, nick, name);
				msg->m_params[0][i] = ',';
				name = &msg->m_params[0][i + 1];
			}
		}
		irc_send_list(frame, driver, nick, name);
		return(0);
	    }
	    case IRC_MSG_PASS:
	    case IRC_MSG_USER:
		return(IRCMsg::send(driver, ":%s %03d :Unauthorized command (already registered)\r\n", server_name, IRC_ERR_ALREADYREGISTERED));
	    default:
		return(IRCMsg::send(driver, ":%s %03d %s :Unknown Command\r\n", server_name, IRC_ERR_UNKNOWNCOMMAND, msg->m_params[0]));
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

int irc_handle_join(MooCodeFrame *frame, MooTCP *driver, const char *name)
{
	// TODO check for invite only??
	//	return(IRCMsg::send(driver, ":%s %03d %s :Cannot join channel (+i)\r\n", server_name, IRC_ERR_INVITEONLYCHAN, msg->m_params[0]));
	// TODO check for channel limit? is there one?
	//	return(IRCMsg::send(driver, ":%s %03d %s :Cannot join channel (+l)\r\n", server_name, IRC_ERR_CHANNELISFULL, msg->m_params[0]));
	// TODO check for too many channels joined?
	//	return(IRCMsg::send(driver, ":%s %03d %s :You have joined too many channels\r\n", server_name, IRC_ERR_TOOMANYCHANNELS, msg->m_params[0]));
	// TODO check for banned??
	//	return(IRCMsg::send(driver, ":%s %03d %s :Cannot join channel (+b)\r\n", server_name, IRC_ERR_BANNEDFROMCHAN, msg->m_params[0]));

	// TODO what are these messages for??
	//	return(IRCMsg::send(driver, ":%s %03d %s :Cannot join channel (+k)\r\n", server_name, IRC_ERR_BADCHANNELKEY, msg->m_params[0]));
	//	return(IRCMsg::send(driver, ":%s %03d %s :Bad channel mask\r\n", server_name, IRC_ERR_BADCHANMASK, msg->m_params[0]));
	//	return(IRCMsg::send(driver, ":%s %03d %s :<ERRORCODE??> recipients. <ABORTMSG??>\r\n", server_name, IRC_ERR_TOOMANYTARGETS, msg->m_params[0]));
	//	return(IRCMsg::send(driver, ":%s %03d %s :Nick/channel is temporarily unavailable\r\n", server_name, IRC_ERR_UNAVAILRESOURCE, msg->m_params[0]));

	MooThing *channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	frame->push_method_call("join", channel);
	return(0);
}

int irc_handle_leave(MooCodeFrame *frame, MooTCP *driver, MooThing *user, const char *name)
{
	MooObjectArray *users;

	MooThing *channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	if ((users = dynamic_cast<MooObjectArray *>(channel->resolve_property("users")))) {
		if (users->search(user) < 0)
			return(IRCMsg::send(driver, ":%s %03d %s :You're not on that channel\r\n", server_name, IRC_ERR_NOTONCHANNEL, name));
	}
	frame->push_method_call("join", channel);
	return(0);
}

int irc_login(MooCodeFrame *frame, MooTCP *driver, MooObjectHash *env)
{
	MooObject *obj;
	MooThing *user;
	char buffer[STRING_SIZE];
	const char *nick, *pass = NULL;

	if (!(obj = env->get("nick")) || (nick = obj->get_string())) {
		IRCMsg::send(driver, "ERROR :Closing Link: No nick given\r\n");
		return(0);
	}

	if ((obj = env->get("pass")) && (pass = obj->get_string())) {
		/// We received a password so attempt to log in as the user with the network password
		user = user_login(nick, buffer);
		if (!user) {
			IRCMsg::send(driver, "ERROR :Closing Link: Invalid password for %s\r\n", nick);
			return(0);
		}
		// TODO you need to get the server somehow
		if (!user->resolve_property("driver", driver) || !user->resolve_property("server", &moo_nil)) {
			IRCMsg::send(driver, "ERROR :Closing Link: Error when logging in to %s\r\n", nick);
			return(0);
		}
		// TODO we need to move this somewhere else or... something (user_login?)
		frame->owner(user->id());
	}
	irc_send_welcome(frame, driver, nick);
	return(1);
}

int irc_send_welcome(MooCodeFrame *frame, MooTCP *driver, const char *nick)
{
	IRCMsg::send(driver, ":%s %03d %s :Welcome to the Moo IRC Portal %s!~%s@%s\r\n", server_name, IRC_RPL_WELCOME, nick, nick, nick, driver->host());
	IRCMsg::send(driver, ":%s %03d %s :Your host is %s, running version SuperDuperMoo v%s\r\n", server_name, IRC_RPL_YOURHOST, nick, server_name, server_version);
	IRCMsg::send(driver, ":%s %03d %s :This server was created ???\r\n", server_name, IRC_RPL_CREATED, nick);
	IRCMsg::send(driver, ":%s %03d %s :%s SuperDuperMoo v%s ? ?\r\n", server_name, IRC_RPL_MYINFO, nick, server_name, server_version);
	// TODO you can send the 005 ISUPPORT messages as well (which doesn't appear to be defined in the IRC standard)

	irc_send_motd(frame, driver, nick);
	//if (user) {
		// TODO remove this eventually??  It should be able to work with it in though, so make sure there are no bugs currently
	//	irc_handle_join(frame, driver, "#realm");
		//IRCMsg::send(driver, ":TheRealm!realm@%s NOTICE %s :Welcome to The Realm of the Jabberwock, %s\r\n", server_name, nick, nick);
	//}
	return(0);
}

int irc_send_motd(MooCodeFrame *frame, MooTCP *driver, const char *nick)
{
	char ch;
	int len, j = 0;
	char buffer[LARGE_STRING_SIZE];

	IRCMsg::send(driver, ":%s %03d %s :- %s Message of the Day -\r\n", server_name, IRC_RPL_MOTDSTART, nick, server_name);
	len = moo_data_read_file("etc/motd.txt", buffer, LARGE_STRING_SIZE);
	for (int i = 0; i <= len; i++) {
		if (buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == '\0') {
			ch = buffer[i];
			if (i - j > 80)
				buffer[j + 80] = '\0';
			else
				buffer[i] = '\0';
			IRCMsg::send(driver, ":%s %03d %s :- %s\r\n", server_name, IRC_RPL_MOTD, nick, &buffer[j]);
			if (ch == '\r' && buffer[i + 1] == '\n')
				i++;
			j = i + 1;
		}
	}
	IRCMsg::send(driver, ":%s %03d %s :End of /MOTD command.\r\n", server_name, IRC_RPL_ENDOFMOTD, nick);
	return(0);
}

int irc_send_join(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name)
{
	IRCMsg::send(driver, ":%s!~%s@%s JOIN :%s\r\n", nick, nick, driver->host(), name);
	// TODO send topic
	irc_send_names(frame, driver, nick, name);
	return(0);
}

int irc_send_part(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name)
{
	IRCMsg::send(driver, ":%s!~%s@%s PART %s\r\n", nick, nick, driver->host(), name);
	return(0);
}

int irc_send_names(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name)
{
/*
	MooString *names;
	MooThing *channel;
	MooObject *result = NULL;

	// TODO should we do this like we do WHO instead of having the special 'names' method?
	if (!m_user)
		return(-1);
	channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	channel->call_method(channel, "names", &result);
	// TODO break into smaller chunks to guarentee the end message is less than 512 bytes
	// TODO the '=' should be different depending on if it's a secret, private, or public channel
	if (result && (names = dynamic_cast<MooString *>(result)))
		IRCMsg::send(driver, ":%s %03d %s = %s :%s\r\n", server_name, IRC_RPL_NAMREPLY, nick, name, names->get_string());
	IRCMsg::send(driver, ":%s %03d %s %s :End of NAMES list.\r\n", server_name, IRC_RPL_ENDOFNAMES, nick, name);
	MOO_DECREF(result);
	return(0);
*/

	int j = 0;
	MooObject *cur;
	const char *thing_name;
	MooThing *channel;
	MooObjectArray *users;
	char buffer[STRING_SIZE];

	channel = MooThing::get_channel(name);
	if (!channel)
		return(IRCMsg::send(driver, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, name));
	// TODO we need some way to check if the room we are currently in cannot list members (you don't want to list members if you
	//	are in the cryolocker, for example)
	if ((users = dynamic_cast<MooObjectArray *>(channel->resolve_property("users")))) {
		for (int i = 0; i <= users->last(); i++) {
			cur = users->get(i);
			// TODO we should check that things are invisible, and also add @ for wizards or something
			if ((cur = cur->resolve_property("name")) && (thing_name = cur->get_string())) {
				strncpy(&buffer[i], thing_name, STRING_SIZE - j);
				j += strlen(thing_name);
				if (j >= STRING_SIZE)
					break;
				buffer[j++] = ' ';
			}
		}
		buffer[j] = '\0';
		IRCMsg::send(driver, ":%s %03d %s = %s :%s\r\n", server_name, IRC_RPL_NAMREPLY, nick, name, buffer);
	}
	IRCMsg::send(driver, ":%s %03d %s %s :End of NAMES list.\r\n", server_name, IRC_RPL_ENDOFNAMES, nick, name);
	return(0);
}

int irc_send_who(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *mask)
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
					IRCMsg::send(driver, ":%s %03d %s %s %s %s %s %s H :0 %s\r\n", server_name, IRC_RPL_WHOREPLY, nick, mask, thing_name, server_name, server_name, thing_name, thing_name);
				}
			}
		}
	}
	IRCMsg::send(driver, ":%s %03d %s %s :End of WHO list.\r\n", server_name, IRC_RPL_ENDOFWHO, nick, mask);
	return(0);
}

int irc_send_list(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name)
{
	const char *str;
	MooObjectHash *list;
	MooObject *channels, *cur, *obj;

	// TODO accessing the db directly isn't really correct here, we should either call a method, evaluate direct code (but which
	//	would allow easy use of a method on chanserv), or something to put the actual db access into a method on ChanServ
	if ((channels = frame->resolve("ChanServ"))) {
		if ((list = dynamic_cast<MooObjectHash *>(channels->resolve_property("db")))) {
			list->reset();
			while ((cur = list->next())) {
				if ((obj = cur->resolve_property("name")) && (str = obj->get_string()))
					IRCMsg::send(driver, ":%s %03d %s %s 1 :\r\n", server_name, IRC_RPL_LIST, nick, str);
			}
		}
	}

	IRCMsg::send(driver, ":%s %03d %s :End of LIST.\r\n", server_name, IRC_RPL_ENDOFLIST, nick);
	return(0);
}

int irc_process_ctcp(MooCodeFrame *frame, MooTCP *driver, IRCMsg *msg, MooThing *user, MooThing *channel)
{
	if (!strncmp(&msg->m_last[1], "ACTION", 6)) {
		char buffer[STRING_SIZE];
		strncpy(buffer, (msg->m_last[7] == ' ') ? &msg->m_last[8] : &msg->m_last[7], STRING_SIZE);
		int len = strlen(buffer);
		buffer[len - 1] = '\0';
		if (channel)
			frame->push_method_call("emote", channel, new MooString("%s", buffer));
		else
			frame->push_method_call("emote", user, new MooString("%s", buffer));
		return(0);
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

void moo_load_irc_methods(MooObjectHash *env)
{
	env->set("%irc-send-status", new MooFuncPtr(irc_pseudoserv_send_status));

	env->set("%irc-process", new MooFuncPtr(irc_pseudoserv_process));
	env->set("%irc-notify", new MooFuncPtr(irc_pseudoserv_notify));
}


/*

(define irc (root:clone))

(define irc:start (lambda (this port)

))

(define irc:new-connection (lambda (this driver)
	(define *out* this)
	(define conn driver)
	(this:handle-connection driver)
))

(define irc:handle-connection (lambda (this driver)
	(loop
		(driver:wait)
		(this:process driver))
))

(define irc:print (lambda (this text)
	(%irc-send-status this user channel text)
))

(define irc:process %irc-process)
(define irc:notify %irc-notify)

(define irc.server (make-server 6667))
(irc.server:set_callback pseudoserv 'new-connection)

*/


