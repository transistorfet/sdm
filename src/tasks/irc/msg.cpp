/*
 * Module Name:	msg.c
 * Description:	IRC Message Interface
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/irc/commands.h>
#include <sdm/tasks/irc/msg.h>

#include <sdm/misc.h>
#include <sdm/hash.h>
#include <sdm/globals.h>

using namespace MooIRC;

#define IRC_MAX_MSG		512

MooException irc_unmarshal_error("Malformed message received");
static MooHash<MsgCommand *> *cmd_list = NULL;

Msg::Msg()
{
	m_msg = NULL;
	this->clear();
}

Msg::~Msg()
{

}

void Msg::clear()
{
	if (m_msg)
		delete m_msg;
	m_nick = NULL;
	m_host = NULL;
	m_cmdtext = NULL;
	m_cmd = 0;
	m_numparams = 0;
	m_last = NULL;
	m_time = 0;

	memset(m_params, '\0', sizeof(char *) * IRC_MAX_ARGS);
}

int Msg::need_more_params()
{
	if (m_cmd && m_numparams < m_cmd->m_min)
		return(1);
	return(0);
}

Msg *Msg::read(MooTCP *inter)
{
	int size;
	Msg *msg;
	char buffer[IRC_MAX_MSG];

	if ((size = inter->receive(buffer, IRC_MAX_MSG, '\n')) <= 0)
		return(NULL);
	//moo_status("IRC: RCVD DEBUG: %s", buffer);
	msg = new Msg();
	if (msg->unmarshal(buffer, size) < 0)
		return(NULL);
	return(msg);
}

int Msg::send(MooTCP *inter, const char *fmt, ...)
{
	va_list va;
	char buffer[STRING_SIZE];

	if (!inter)
		return(-1);
	va_start(va, fmt);
	vsnprintf(buffer, STRING_SIZE, fmt, va);
	//moo_status("IRC: SEND DEBUG: %s", buffer);
	return(inter->send(buffer));
}


int Msg::unmarshal(char *str, int size)
{
	int i = 0;

	if (m_msg)
		this->clear();
	m_msg = new char[size + 1];
	strcpy(m_msg, str);

	// Parse the nick and server if it exists
	if (str[i] == ':') {
		m_nick = &str[++i];
		while ((str[i] != '!') && (str[i] != ' ') && (str[i] != '\0'))
			i++;
		if (str[i] == '\0')
			throw irc_unmarshal_error;
		if (str[i] == '!') {
			str[i] = '\0';
			m_host = &str[++i];
			while ((str[i] != ' ') && (str[i] != '\0'))
				i++;
			if (str[i] == '\0')
				throw irc_unmarshal_error;
		}
		str[i] = '\0';
		i++;
	}

	// Parse the command
	m_cmdtext = &str[i];
	for (; str[i] != ' '; i++) {
		if (str[i] == '\0')
			throw irc_unmarshal_error;
		if (str[i] >= 0x61 && str[i] <= 0x7a)
			str[i] -= 0x20;
	}
	str[i++] = '\0';
	m_cmd = cmd_list->get(m_cmdtext);

	// Parse the parameters
	m_params[m_numparams] = &str[i];
	for (; ((str[i] != '\r') && (str[i] != '\n') && (str[i] != '\0')); i++) {
		if (str[i] == ':') {
			m_params[m_numparams] = &str[++i];
			m_last = &str[i];
			while ((str[i] != 0x0d) && (str[i] != 0x0a) && (str[i] != '\0'))
				i++;
			str[i] = '\0';
			break;
		}
		else if (str[i] == ' ') {
			str[i] = '\0';
			if (++m_numparams >= IRC_MAX_ARGS)
				throw irc_unmarshal_error;
			m_params[m_numparams] = &str[i + 1];
		}
	}
	// If there is a first parameter, then correct the number of parameters
	if (m_params[0][0])
		m_numparams++;
	str[i] = '\0';
	m_time = time(NULL);
	return(0);
}

inline void irc_add_command(const char *name, int cmd, int usetext, int min, int max)
{
	MsgCommand *ptr;
	ptr = new MsgCommand(name, cmd, usetext, min, max);
	cmd_list->set(name, ptr);
}

int init_irc_msg()
{
	if (cmd_list)
		return(1);
	cmd_list = new MooHash<MsgCommand *>(64, MOO_HBF_DELETEALL);
	irc_add_command("PASS", IRC_MSG_PASS, 0, 1, 1);
	irc_add_command("NICK", IRC_MSG_NICK, 0, 0, 1);
	irc_add_command("USER", IRC_MSG_USER, 1, 4, 4);
	irc_add_command("OPER", IRC_MSG_OPER, 0, 2, 2);
	irc_add_command("QUIT", IRC_MSG_QUIT, 1, 0, 1);
	irc_add_command("SQUIT", IRC_MSG_SQUIT, 1, 2, 2);
	irc_add_command("JOIN", IRC_MSG_JOIN, 0, 1, 2);
	irc_add_command("PART", IRC_MSG_PART, 1, 1, 2);
	irc_add_command("MODE", IRC_MSG_MODE, 0, 1, 0);
	irc_add_command("TOPIC", IRC_MSG_TOPIC, 1, 1, 2);
	irc_add_command("NAMES", IRC_MSG_NAMES, 0, 0, 2);
	irc_add_command("LIST", IRC_MSG_LIST, 0, 0, 1);
	irc_add_command("INVITE", IRC_MSG_INVITE, 0, 2, 2);
	irc_add_command("KICK", IRC_MSG_KICK, 1, 2, 3);
	irc_add_command("PRIVMSG", IRC_MSG_PRIVMSG, 1, 0, 2);
	irc_add_command("NOTICE", IRC_MSG_NOTICE, 1, 0, 2);
	irc_add_command("MOTD", IRC_MSG_MOTD, 0, 0, 1);
	irc_add_command("LUSERS", IRC_MSG_LUSERS, 0, 0, 2);
	irc_add_command("VERSION", IRC_MSG_VERSION, 0, 0, 1);
	irc_add_command("STATS", IRC_MSG_STATS, 0, 0, 2);
	irc_add_command("LINKS", IRC_MSG_LINKS, 0, 0, 2);
	irc_add_command("TIME", IRC_MSG_TIME, 0, 0, 1);
	irc_add_command("CONNECT", IRC_MSG_CONNECT, 0, 2, 3);
	irc_add_command("TRACE", IRC_MSG_TRACE, 0, 0, 1);
	irc_add_command("ADMIN", IRC_MSG_ADMIN, 0, 0, 1);
	irc_add_command("INFO", IRC_MSG_INFO, 0, 0, 1);
	irc_add_command("SERVLIST", IRC_MSG_SERVLIST, 0, 0, 2);
	irc_add_command("SQUERY", IRC_MSG_SQUERY, 1, 0, 2);
	irc_add_command("WHO", IRC_MSG_WHO, 0, 0, 2);
	irc_add_command("WHOIS", IRC_MSG_WHOIS, 0, 0, 2);
	irc_add_command("WHOWAS", IRC_MSG_WHOWAS, 0, 0, 3);
	irc_add_command("KILL", IRC_MSG_KILL, 1, 2, 2);
	irc_add_command("PING", IRC_MSG_PING, 1, 0, 2);
	irc_add_command("PONG", IRC_MSG_PONG, 1, 0, 2);
	irc_add_command("ERROR", IRC_MSG_ERROR, 1, 1, 1);
	return(0);
}

/*
IRCException irc_nosuchnick(IRC_ERR_NOSUCHNICK, "No such nick/channel");
IRCException irc_nosuchserver(IRC_ERR_NOSUCHSERVER, "");
IRCException irc_nosuchchannel(IRC_ERR_NOSUCHCHANNEL, "");
IRCException irc_cannotsendtochan(IRC_ERR_CANNOTSENDTOCHAN, "");
IRCException irc_toomanychannels(IRC_ERR_TOOMANYCHANNELS, "");
IRCException irc_wasnosuchnick(IRC_ERR_WASNOSUCHNICK, "");
IRCException irc_toomanytargets(IRC_ERR_TOOMANYTARGETS, "");
IRCException irc_nosuchservice(IRC_ERR_NOSUCHSERVICE, "");
IRCException irc_noorigin(IRC_ERR_NOORIGIN, "");
IRCException irc_norecipient(IRC_ERR_NORECIPIENT, "");
IRCException irc_notexttosend(IRC_ERR_NOTEXTTOSEND, "");
IRCException irc_notoplevel(IRC_ERR_NOTOPLEVEL, "");
IRCException irc_wildtoplevel(IRC_ERR_WILDTOPLEVEL, "");
IRCException irc_badmask(IRC_ERR_BADMASK, "");
IRCException irc_unknowncommand(IRC_ERR_UNKNOWNCOMMAND, "");
IRCException irc_nomotd(IRC_ERR_NOMOTD, "");
IRCException irc_noadmininfo(IRC_ERR_NOADMININFO, "");
IRCException irc_fileerror(IRC_ERR_FILEERROR, "");
IRCException irc_nonicknamegiven(IRC_ERR_NONICKNAMEGIVEN, "");
IRCException irc_erroneusnickname(IRC_ERR_ERRONEUSNICKNAME, "");
IRCException irc_nicknameinuse(IRC_ERR_NICKNAMEINUSE, "");
IRCException irc_nickcollision(IRC_ERR_NICKCOLLISION, "");
IRCException irc_unavailresource(IRC_ERR_UNAVAILRESOURCE, "");
IRCException irc_usernotinchannel(IRC_ERR_USERNOTINCHANNEL, "");
IRCException irc_notonchannel(IRC_ERR_NOTONCHANNEL, "");
IRCException irc_useronchannel(IRC_ERR_USERONCHANNEL, "");
IRCException irc_nologin(IRC_ERR_NOLOGIN, "");
IRCException irc_summondisabled(IRC_ERR_SUMMONDISABLED, "");
IRCException irc_userdisabled(IRC_ERR_USERDISABLED, "");
IRCException irc_notregistered(IRC_ERR_NOTREGISTERED, "");
IRCException irc_needmoreparams(IRC_ERR_NEEDMOREPARAMS, "");
IRCException irc_alreadyregistered(IRC_ERR_ALREADYREGISTERED, "");
IRCException irc_nopermforhost(IRC_ERR_NOPERMFORHOST, "");
IRCException irc_passwdmismatch(IRC_ERR_PASSWDMISMATCH, "");
IRCException irc_yourebannedcreep(IRC_ERR_YOUREBANNEDCREEP, "");
IRCException irc_youwillbebanned(IRC_ERR_YOUWILLBEBANNED, "");
IRCException irc_keyset(IRC_ERR_KEYSET, "");
IRCException irc_channelisfull(IRC_ERR_CHANNELISFULL, "");
IRCException irc_unknownmode(IRC_ERR_UNKNOWNMODE, "");
IRCException irc_inviteonlychan(IRC_ERR_INVITEONLYCHAN, "");
IRCException irc_bannedfromchan(IRC_ERR_BANNEDFROMCHAN, "");
IRCException irc_badchannelkey(IRC_ERR_BADCHANNELKEY, "");
IRCException irc_badchanmask(IRC_ERR_BADCHANMASK, "");
IRCException irc_nochanmodes(IRC_ERR_NOCHANMODES, "");
IRCException irc_banlistfull(IRC_ERR_BANLISTFULL, "");
IRCException irc_noprivileges(IRC_ERR_NOPRIVILEGES, "");
IRCException irc_chanoprivsneeded(IRC_ERR_CHANOPRIVSNEEDED, "");
IRCException irc_cantkillserver(IRC_ERR_CANTKILLSERVER, "");
IRCException irc_restricted(IRC_ERR_RESTRICTED, "");
IRCException irc_uniqopprivsneeded(IRC_ERR_UNIQOPPRIVSNEEDED, "");
IRCException irc_nooperhost(IRC_ERR_NOOPERHOST, "");
IRCException irc_umodeunknownflag(IRC_ERR_UMODEUNKNOWNFLAG, "");
IRCException irc_usersdontmatch(IRC_ERR_USERSDONTMATCH, "");
IRCException irc_erroneusnickname(IRC_ERR_ERRONEUSNICKNAME, "Erroneous nickname");
IRCException irc_nicknameinuse(IRC_ERR_NICKNAMEINUSE, "Nickname is already in use");
*/

