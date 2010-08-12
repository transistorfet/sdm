/*
 * Name:	msg.h
 * Description:	IRC Message Handling
 */

#ifndef _SDM_INTERFACE_IRC_MSG_H
#define _SDM_INTERFACE_IRC_MSG_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/irc/commands.h>

namespace MooIRC {

#define IRC_MAX_ARGS		14

class Msg {
    public:
	char *m_msg;
	const char *m_nick;
	const char *m_host;
	const char *m_cmdtext;
	MsgCommand *m_cmd;
	const char *m_params[IRC_MAX_ARGS];
	int m_numparams;
	const char *m_last;
	time_t m_time;

    public:
	Msg();
	virtual ~Msg();
	void clear();

	int cmd() { if (m_cmd) return(m_cmd->m_cmd); return(IRC_ERR_UNKNOWNCOMMAND); }
	const char *cmdtext() { return(m_cmdtext); }
	int need_more_params();

	int send(MooTCP *inter);

	static Msg *read(MooTCP *inter);
	static int send(MooTCP *inter, const char *fmt, ...);

    protected:
	int marshal();
	int unmarshal(char *str, int size);
};

int init_irc_msg();

}

#endif

