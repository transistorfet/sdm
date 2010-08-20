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
#include <sdm/things/world.h>

#include <sdm/misc.h>
#include <sdm/globals.h>

MooObjectType moo_irc_pseudoserv_obj_type = {
	&moo_task_obj_type,
	"irc-pseudoserv",
	typeid(MooIRC::PseudoServ).name(),
	(moo_type_create_t) moo_irc_pseudoserv_create
};

const char *server_name = "moo.jabberwocky.ca";
const char *server_version = "0.1";

int init_irc_pseudoserv(void)
{
	// TODO why wont this compile
	extern int init_irc_msg();
	init_irc_msg();
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

int PseudoServ::notify(int type, MooThing *channel, MooThing *thing, const char *str)
{
	switch (type) {
	    case TNT_STATUS: {
		char buffer[LARGE_STRING_SIZE];
		PseudoServ::format(buffer, LARGE_STRING_SIZE, str);
		return(Msg::send(m_inter, ":TheRealm!realm@%s PRIVMSG #realm :%s\r\n", server_name, buffer));
	    }
	    case TNT_SAY:
	    case TNT_EMOTE: {
		MooString *thing_name;
		MooString *channel_name;
		char buffer[LARGE_STRING_SIZE];

		/// We don't send a message if it was said by our user, since the IRC client will echo that message itself
		if (!thing || thing == m_user)
			return(0);
		thing_name = (MooString *) thing->get_property("name", &moo_string_obj_type);

		if (channel)
			channel_name = (MooString *) thing->get_property("name", &moo_string_obj_type);
		PseudoServ::format(buffer, LARGE_STRING_SIZE, str);
		if (type == TNT_SAY)
			return(Msg::send(m_inter, ":%s!%s@%s PRIVMSG %s :%s\r\n", thing_name ? thing_name->m_str : "Unknown", thing_name ? thing_name->m_str : "realm", server_name, (channel && channel_name) ? channel_name->m_str : "#realm", buffer));
		else
			return(Msg::send(m_inter, ":%s!%s@%s PRIVMSG %s :\x01\x41\x43TION%s\x01\r\n", thing_name ? thing_name->m_str : "Unknown", thing_name ? thing_name->m_str : "realm", server_name, (channel && channel_name) ? channel_name->m_str : "#realm", buffer));
		break;
	    }
	    case TNT_JOIN:
	    case TNT_LEAVE:
	    case TNT_QUIT:
		// TODO implement these later
		break;
	    default:
		break;
	}
	return(0);
}

//int PseudoServ::talk(MooThing *channel, MooThing *thing, const char *str)

//int PseudoServ::print(MooThing *channel, MooThing *thing, const char *str)
//{

//}

int PseudoServ::handle(MooInterface *inter, int ready)
{
	Msg *msg;

	if (!(ready & IO_READY_READ))
		return(-1);
	try {
		msg = Msg::read(dynamic_cast<MooTCP *>(inter));
		if (!msg)
			return(1);
		this->dispatch(msg);
		delete msg;
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
	    case IRC_MSG_PRIVMSG:
		if (!msg->m_last)
			return(Msg::send(m_inter, ":%s %03d :No text to send\r\n", server_name, IRC_ERR_NOTEXTTOSEND));
		else if (moo_is_channel_name(msg->m_params[0])) {
			if (!strcmp(msg->m_params[0], "#realm")) {
				if (!m_user)
					return(Msg::send(m_inter, ":%s NOTICE %s :You aren't logged in yet\r\n", server_name, m_nick->c_str()));
				else if (msg->m_last[0] == '.')
					m_user->command(&msg->m_last[1]);
				else
					m_user->command("say", msg->m_last);
			}
		}
		else {

		}
		return(0);
	    case IRC_MSG_MODE:
		if (moo_is_channel_name(msg->m_params[0])) {
			// TODO do channel mode command processing
		}
		else {
			if (strcmp(m_nick->c_str(), msg->m_params[0]))
				return(Msg::send(m_inter, ":%s %03d :Cannot change mode for other users\r\n", server_name, IRC_ERR_USERSDONTMATCH));

			// TODO check for unknown mode flag
			//return(Msg::send(m_inter, ":%s %03d :Unknown MODE flag\r\n", server_name, IRC_ERR_UMODEUNKNOWNFLAG));

			/// User MODE command reply
			return(Msg::send(m_inter, ":%s %03d %s\r\n", server_name, IRC_RPL_UMODEIS, msg->m_params[0]));
		}
		break;
	    case IRC_MSG_JOIN:
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

		// TODO check for invalid channel name
		//	return(Msg::send(m_inter, ":%s %03d %s :No such channel\r\n", server_name, IRC_ERR_NOSUCHCHANNEL, msg->m_params[0]));

		if (!strcmp(msg->m_params[0], "0"))
			;// TODO leave all channels
		else
			// TODO parse out the possibility of multiple channels
			return(this->join(msg->m_params[0]));
		break;
	    case IRC_MSG_WHOIS:

		return(Msg::send(m_inter, ":%s %03d %s :End of WHOIS list\r\n", server_name, IRC_RPL_ENDOFWHOIS, m_nick->c_str()));
	    case IRC_MSG_QUIT:
		// TODO send quit message around???
		Msg::send(m_inter, "ERROR :Closing Link: %s[%s] (Quit: )\r\n", m_nick->c_str(), m_inter->host());
		delete this;
		return(0);
	    case IRC_MSG_PASS:
	    case IRC_MSG_USER:
		return(Msg::send(m_inter, ":%s %03d :Unauthorized command (already registered)\r\n", server_name, IRC_ERR_ALREADYREGISTERED));
	    default:
		return(Msg::send(m_inter, ":%s %03d %s :Unknown Command\r\n", server_name, IRC_ERR_UNKNOWNCOMMAND, msg->m_params[0]));
	}

	/*
	JOIN
	PART
	MODE
	NICK
	QUIT
	TOPIC
	NAMES
	LIST
	PRIVMSG
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

int PseudoServ::login()
{
	char buffer[STRING_SIZE];

	// TODO overwrite buffers used to store password as a security measure

	/// If we didn't receive a password then we aren't going to login but we wont fail either
	if (m_pass) {
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
		else if (m_user->connect(this) < 0) {
			Msg::send(m_inter, "ERROR :Closing Link: Error when logging in to %s\r\n", m_nick->c_str());
			delete this;
			return(0);
		}
	}
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
		this->join("#realm");
		//m_user->notify(TNT_STATUS, NULL, NULL, "Welcome to The Realm of the Jabberwock, %s", m_nick->c_str());
	}
	return(0);
}

int PseudoServ::join(const char *name)
{
	Msg::send(m_inter, ":%s!~%s@%s JOIN :%s\r\n", m_nick->c_str(), m_nick->c_str(), m_inter->host(), name);
	// TODO send topic
	// TODO send names replies
	return(0);
}

int PseudoServ::part(const char *name)
{

	return(0);
}

// TODO should this be a generic function somewhere else, with a function pointer passed to it for converting a specific tag
//	to the necessary colour escape codes (so that all interface mechanisms can use it)
int PseudoServ::format(char *buffer, int max, const char *str)
{
	int i, j, k;

	for (i = 0, j = 0; (j < max) && (str[i] != '\0'); i++) {
		switch (str[i]) {
		    case '\n': {
			buffer[j++] = '\020';
			buffer[j++] = 'n';
			break;
		    }
		    case '<': {
			k = i + 1;
			if (str[i] == '/')
				k++;
			k = PseudoServ::write_attrib(&buffer[j], max - j - 1, &str[k]);
			/// If a recognized tag was not found, then copy it through to the output
			if (!k)
				buffer[j++] = '<';
			else {
				j = k;
				for (; str[i] != '\0' && str[i] != '>'; i++)
					;
			}
			break;
		    }
		    case '&': {
			if (!strncmp(&str[i + 1], "lt;", 3)) {
				buffer[j++] = '<';
				i += 3;
			}
			else if (!strncmp(&str[i + 1], "gt;", 3)) {
				buffer[j++] = '>';
				i += 3;
			}
			else if (!strncmp(&str[i + 1], "amp;", 4)) {
				buffer[j++] = '&';
				i += 4;
			}
			else if (!strncmp(&str[i + 1], "quot;", 5)) {
				buffer[j++] = '\"';
				i += 5;
			}
			else
				buffer[j++] = '&';
			break;
		    }
		    default: {
			if (str[i] >= 0x20)
				buffer[j++] = str[i];
			break;
		    }
		}
	}
	buffer[j] = '\0';
	return(j);
}

int PseudoServ::write_attrib(char *buffer, int max, const char *attrib)
{
	int j = 0;

	if (attrib[0] == 'b') {
		if (attrib[1] == '>')
			buffer[j++] = '\x02';
		else if (attrib[1] == 'l') {
			if (!strncmp(&attrib[2], "ack>", 4)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x31';
			}
			else if (!strncmp(&attrib[2], "ue>", 3)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x31';
			}
		}
		else if (!strncmp(&attrib[1], "right", 5)) {
			if (!strncmp(&attrib[6], "blue>", 5)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x31';
				buffer[j++] = '\x32';
			}
			else if (!strncmp(&attrib[6], "cyan>", 5)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x31';
				buffer[j++] = '\x31';
			}
			else if (!strncmp(&attrib[6], "green>", 6)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x39';
			}
			else if (!strncmp(&attrib[6], "magenta>", 8)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x31';
				buffer[j++] = '\x33';
			}
			else if (!strncmp(&attrib[6], "red>", 4)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x34';
			}
			else if (!strncmp(&attrib[6], "white>", 6)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x31';
				buffer[j++] = '\x35';
			}
			else if (!strncmp(&attrib[6], "yellow>", 7)) {
				buffer[j++] = '\x03';
				buffer[j++] = '\x38';
			}
		}
	}
	else if (!strncmp(attrib, "blue>", 5)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x32';
	}
	else if (!strncmp(attrib, "cyan>", 5)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x31';
		buffer[j++] = '\x31';
	}
	else if (!strncmp(attrib, "green>", 6)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x33';
	}
	else if (!strncmp(attrib, "magenta>", 8)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x36';
	}
	else if (!strncmp(attrib, "red>", 4)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x35';
	}
	else if (!strncmp(attrib, "white>", 6)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x31';
		buffer[j++] = '\x34';
	}
	else if (!strncmp(attrib, "yellow>", 7)) {
		buffer[j++] = '\x03';
		buffer[j++] = '\x37';
	}
	return(j);
}


