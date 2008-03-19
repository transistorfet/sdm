/*
 * Header Name:	telnet.h
 * Description:	Telnet Protocol Interface Manager Header
 */

#ifndef _SDM_INTERFACE_TELNET_H
#define _SDM_INTERFACE_TELNET_H

#include <sdm/globals.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>

struct sdm_user;
struct sdm_login;

#define SDM_TELNET(ptr)		( (struct sdm_telnet *) (ptr) )

struct sdm_telnet {
	struct sdm_tcp tcp;

};

extern struct sdm_interface_type sdm_telnet_obj_type;

int init_telnet(void);
void release_telnet(void);

void sdm_telnet_release(struct sdm_telnet *);

int sdm_telnet_read(struct sdm_telnet *, char *, int);
int sdm_telnet_write(struct sdm_telnet *, const char *);
void sdm_telnet_echo(struct sdm_telnet *, int);

void sdm_telnet_encrypt_password(const char *, char *, int);

int sdm_telnet_login(struct sdm_telnet *);
int sdm_telnet_register(struct sdm_telnet *, struct sdm_login *);
int sdm_telnet_run(struct sdm_telnet *, struct sdm_user *);

#endif

