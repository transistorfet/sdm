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

PseudoServ::PseudoServ()
{
	m_bits = 0;
	m_nick = NULL;
	m_inter = NULL;
	m_user = NULL;
}

PseudoServ::~PseudoServ()
{
	this->set_delete();
	if (m_nick)
		delete m_nick;
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
/*
	catch (IRCException e) {
		Msg::send_numeric(inter, e.m_num, e.get());
		if (m_nick)
			moo_status("IRC: %s: %s", m_nick, e.get());
		else
			moo_status("IRC: %s", e.get());
	}
*/
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
	// We assume that we were called because the interface is already being deleted, in which case we don't want to delete it too
	m_inter = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::purge(MooUser *user)
{
	if (user != m_user)
		return(-1);
	// We assume that since we were called because the user is already being deleted, in which case we don't want to delete too
	m_user = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::dispatch(Msg *msg)
{
	if (msg->need_more_params())
		return(Msg::send(m_inter, ":%s %03d %s :Not enough parameters\r\n", server_name, IRC_ERR_NEEDMOREPARAMS, msg->m_cmdtext));

	// Process messages that are common for pre and post registration
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
			// TODO check it
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
				this->send_welcome();
			break;
		    }
		    case IRC_MSG_USER:
			m_bits |= IRC_BF_RECEIVED_USER;
			if (m_nick)
				this->send_welcome();
			break;
		    default:
			return(Msg::send(m_inter, ":%s %03d :You have not registered\r\n", server_name, IRC_ERR_NOTREGISTERED));
		}
		return(0);
	}

	// Process messages that are only acceptable after registration
	switch (msg->cmd()) {
	    case IRC_MSG_PRIVMSG:


		return(0);
	    case IRC_MSG_MODE:
		if (0) { // TODO moo_is_channel_name(msg->m_params[0])) {

		}
		else {
			if (strcmp(m_nick->c_str(), msg->m_params[0]))
				return(Msg::send(m_inter, ":%s %03d :Cannot change mode for other users\r\n", server_name, IRC_ERR_USERSDONTMATCH));

			// TODO check for unknown mode flag
			//return(Msg::send(m_inter, ":%s %03d :Unknown MODE flag\r\n", server_name, IRC_ERR_UMODEUNKNOWNFLAG));

			// User MODE command reply
			return(Msg::send(m_inter, ":%s %03d %s\r\n", server_name, IRC_RPL_UMODEIS, msg->m_params[0]));
		}
		break;
	    case IRC_MSG_WHOIS:

		return(Msg::send(m_inter, ":%s %03d %s :End of WHOIS list\r\n", server_name, IRC_RPL_ENDOFWHOIS, m_nick));
	    case IRC_MSG_QUIT:
		return(0);
	    case IRC_MSG_PASS:
	    case IRC_MSG_USER:
		return(Msg::send(m_inter, ":%s %03d :Unauthorized command (already registered)\r\n", server_name, IRC_ERR_ALREADYREGISTERED));
	    default:
		return(Msg::send(m_inter, ":%s %03d %s :Unknown Command\r\n", server_name, IRC_ERR_UNKNOWNCOMMAND, msg->m_params[0]));
		break;
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

int PseudoServ::send_welcome()
{
	// TODO we need to send the user@host part, do we not?
	Msg::send(m_inter, ":%s %03d %s :Welcome to the Moo IRC Portal %s!?@?\r\n", server_name, IRC_RPL_WELCOME, m_nick->c_str(), m_nick->c_str());
	Msg::send(m_inter, ":%s %03d %s :Your host is %s, running version SuperDuperMoo v%s\r\n", server_name, IRC_RPL_YOURHOST, m_nick->c_str(), server_name, server_version);
	Msg::send(m_inter, ":%s %03d %s :This server was created ???\r\n", server_name, IRC_RPL_CREATED, m_nick->c_str());
	Msg::send(m_inter, ":%s %03d %s :%s SuperDuperMoo v%s ? ?\r\n", server_name, IRC_RPL_MYINFO, m_nick->c_str(), server_name, server_version);
	//Msg::send(m_inter, ":%s %03d %s :Welcome to the Moo IRC Portal %s!?@?\r\n", server_name, IRC_RPL_BOUNCE, m_nick->c_str(), m_nick->c_str());

	Msg::send(m_inter, ":%s %03d %s :- %s Message of the Day -\r\n", server_name, IRC_RPL_MOTDSTART, m_nick->c_str(), server_name);
	Msg::send(m_inter, ":%s %03d %s :End of /MOTD command.\r\n", server_name, IRC_RPL_ENDOFMOTD, m_nick->c_str());
	m_bits |= IRC_BF_WELCOMED;
	return(0);
}

