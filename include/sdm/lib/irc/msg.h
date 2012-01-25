/*
 * Name:	msg.h
 * Description:	IRC Message Handling
 */

#ifndef _SDM_LIB_IRC_MSG_H
#define _SDM_LIB_IRC_MSG_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/lib/irc/commands.h>
#include <sdm/drivers/tcp.h>

#define IRC_MAX_ARGS		14
#define IRC_MAX_MSG		512

class IRCMsg {
    public:
	char *m_nick;
	char *m_host;
	char *m_cmdtext;
	IRCMsgCommand *m_cmd;
	char *m_params[IRC_MAX_ARGS];
	int m_numparams;
	char *m_last;
	time_t m_time;
	char m_buffer[IRC_MAX_MSG];

    public:
	IRCMsg();
	virtual ~IRCMsg();
	void clear();

	int cmd() { if (m_cmd) return(m_cmd->m_cmd); return(IRC_ERR_UNKNOWNCOMMAND); }
	const char *cmdtext() { return(m_cmdtext); }
	int need_more_params();

	int send(MooTCP *inter);
	int receive(MooTCP *inter);

	static IRCMsg *read(MooTCP *inter);
	static int send(MooTCP *inter, const char *fmt, ...);

    protected:
	int marshal();
	int unmarshal(char *str, int size);
};

#endif

