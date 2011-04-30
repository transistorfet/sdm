/*
 * Name:	pseudo.h
 * Description:	IRC PseudoServer Handling
 */

#ifndef _SDM_INTERFACE_IRC_PSEUDO_H
#define _SDM_INTERFACE_IRC_PSEUDO_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/things/user.h>
#include <sdm/tasks/task.h>

#include <sdm/tasks/irc/msg.h>

class MooChannel;

namespace MooIRC {

#define IRC_BF_WELCOMED		0x0001
#define IRC_BF_RECEIVED_USER	0x0002

class PseudoServ : public MooTask {
    protected:
	int m_bits;
	std::string *m_nick;
	std::string *m_pass;
	MooTCP *m_inter;
	MooUser *m_user;

    public:
	PseudoServ();
	~PseudoServ();

	int read_entry(const char *type, MooDataFile *data);
	int write_data(MooDataFile *data);

	int initialize();
	int idle();
	int release();

	int notify(int type, MooThing *thing, MooThing *channel, const char *str);

	int handle(MooInterface *inter, int ready);
	int bestow(MooInterface *inter);
	int purge(MooInterface *inter);

	int purge(MooUser *user);

	int dispatch(Msg *msg);

    protected:
	int welcomed() { return(m_bits & IRC_BF_WELCOMED); }
	int received_user() { return(m_bits & IRC_BF_RECEIVED_USER); }

	int handle_join(const char *name);
	int handle_leave(const char *name);

	int login();
	int send_welcome();
	int send_join(const char *name);
	int send_part(const char *name);
	int send_names(const char *name);
	int send_who(const char *mask);
	int process_ctcp(Msg *msg, MooThing *channel);
};

}

extern MooObjectType moo_irc_pseudoserv_obj_type;

int init_irc_pseudoserv(void);
void release_irc_pseudoserv(void);
MooObject *make_moo_irc_pseudoserv(MooDataFile *data);

int irc_write_attrib(int type, char *buffer, int max);

#endif

