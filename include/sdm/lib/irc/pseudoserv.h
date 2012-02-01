/*
 * Name:	pseudoserv.h
 * Description:	IRC PseudoServer Handling
 */

#ifndef _SDM_LIB_IRC_PSEUDO_H
#define _SDM_LIB_IRC_PSEUDO_H

#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/thing.h>
#include <sdm/drivers/tcp.h>
#include <sdm/code/task.h>

#include <sdm/lib/irc/msg.h>

class MooChannel;

#define IRC_BF_WELCOMED		0x0001
#define IRC_BF_RECEIVED_USER	0x0002

/*
class PseudoServ {
    protected:
	int m_bits;
	std::string *m_nick;
	std::string *m_pass;
	MooTCP *m_driver;
	MooThing *m_user;

    public:
	PseudoServ();
	~PseudoServ();

	int notify(int type, MooThing *thing, MooThing *channel, const char *str);

	int handle(MooDriver *driver, int ready);
	int bestow(MooDriver *driver);
	int purge(MooDriver *driver);

	int purge(MooThing *user);

	int dispatch(MooCodeFrame *frame, MooObjectHash *env, IRCMsg *msg);

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
*/

int init_irc_pseudoserv(void);
void release_irc_pseudoserv(void);

void moo_load_irc_methods(MooObjectHash *env);

int irc_pseudoserv_notify(MooCodeFrame *frame, MooObjectArray *args);
int irc_pseudoserv_process(MooCodeFrame *frame, MooObjectArray *args);
int irc_dispatch(MooCodeFrame *frame, MooThing *m_this, MooTCP *driver, IRCMsg *msg);
int irc_handle_join(MooCodeFrame *frame, MooTCP *driver, const char *name);
int irc_handle_leave(MooCodeFrame *frame, MooTCP *driver, MooThing *user, const char *name);
int irc_login(MooCodeFrame *frame, MooTCP *driver, MooObjectHash *env);
int irc_send_welcome(MooCodeFrame *frame, MooTCP *driver, const char *nick);
int irc_send_motd(MooCodeFrame *frame, MooTCP *driver, const char *nick);
int irc_send_join(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name);
int irc_send_part(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name);
int irc_send_names(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name);
int irc_send_who(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *mask);
int irc_send_list(MooCodeFrame *frame, MooTCP *driver, const char *nick, const char *name);
int irc_process_ctcp(MooCodeFrame *frame, MooTCP *driver, IRCMsg *msg, MooThing *user, MooThing *channel);
int irc_write_attrib(int type, char *buffer, int max);

#endif

