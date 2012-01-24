/*
 * Name:	pseudoserv.h
 * Description:	IRC PseudoServer Handling
 */

#ifndef _SDM_LIB_IRC_PSEUDO_H
#define _SDM_LIB_IRC_PSEUDO_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/code/task.h>

#include <sdm/lib/irc/msg.h>

class MooChannel;

#define IRC_BF_WELCOMED		0x0001
#define IRC_BF_RECEIVED_USER	0x0002

class PseudoServ {
    protected:
	int m_bits;
	std::string *m_nick;
	std::string *m_pass;
	MooTCP *m_inter;
	MooThing *m_user;

    public:
	PseudoServ();
	~PseudoServ();

	int notify(int type, MooThing *thing, MooThing *channel, const char *str);

	int handle(MooInterface *inter, int ready);
	int bestow(MooInterface *inter);
	int purge(MooInterface *inter);

	int purge(MooThing *user);

	int dispatch(IRCMsg *msg);

    protected:
	int welcomed() { return(m_bits & IRC_BF_WELCOMED); }
	int received_user() { return(m_bits & IRC_BF_RECEIVED_USER); }

	int handle_join(const char *name);
	int handle_leave(const char *name);

	int login();
	int send_welcome();
	int send_motd();
	int send_join(const char *name);
	int send_part(const char *name);
	int send_names(const char *name);
	int send_who(const char *mask);
	int send_list(const char *name);
	int process_ctcp(IRCMsg *msg, MooThing *channel);
};

int init_irc_pseudoserv(void);
void release_irc_pseudoserv(void);

#endif

