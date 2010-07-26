/*
 * Header Name:	telnet.h
 * Description:	Telnet Protocol Interface Manager Header
 */

#ifndef _SDM_INTERFACES_TELNET_H
#define _SDM_INTERFACES_TELNET_H

#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

class MooTelnet {
    public:
	MooTelnet();
	virtual ~MooTelnet();

	virtual int read(char *data, int len);
	virtual int write(const char *data);
};

extern MooObjectType moo_telnet_obj_type;

int init_telnet(void);
void release_telnet(void);
MooObject *moo_telnet_create(void);

void sdm_telnet_echo(struct sdm_telnet *, int);

void sdm_telnet_encrypt_password(const char *, char *, int);

int sdm_telnet_login(struct sdm_telnet *);
int sdm_telnet_register(struct sdm_telnet *, struct sdm_login *);
int sdm_telnet_run(struct sdm_telnet *, struct sdm_user *);

#endif

