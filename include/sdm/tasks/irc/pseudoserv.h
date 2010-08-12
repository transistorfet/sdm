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

namespace MooIRC {

#define IRC_BF_WELCOMED		0x0001
#define IRC_BF_RECEIVED_USER	0x0002


class PseudoServ : public MooTask {
    protected:
	int m_bits;
	std::string *m_nick;
	MooTCP *m_inter;
	MooUser *m_user;

    public:
	PseudoServ();
	virtual ~PseudoServ();

	virtual int read_entry(const char *type, MooDataFile *data);
	virtual int write_data(MooDataFile *data);

	virtual int initialize();
	virtual int idle();
	virtual int release();

	virtual int handle(MooInterface *inter, int ready);
	virtual int bestow(MooInterface *inter);
	int purge(MooInterface *inter);

	int purge(MooUser *user);

	int dispatch(Msg *msg);

    protected:
	int welcomed() { return(m_bits & IRC_BF_WELCOMED); }
	int received_user() { return(m_bits & IRC_BF_RECEIVED_USER); }
	int send_welcome();
};

}

extern MooObjectType moo_irc_pseudoserv_obj_type;

int init_irc_pseudoserv(void);
void release_irc_pseudoserv(void);
MooObject *moo_irc_pseudoserv_create(void);

#endif

