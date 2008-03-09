/*
 * Header Name:	telnet.h
 * Description:	Telnet Protocol Interface Manager Header
 */

#ifndef _SDM_INTERFACE_TELNET_H
#define _SDM_INTERFACE_TELNET_H

#include <sdm/globals.h>

#include <sdm/interface/interface.h>
#include <sdm/interface/tcp.h>

#include <sdm/objs/login.h>
#include <sdm/objs/user.h>

#define TELNET_CMD_SE			240
#define TELNET_CMD_NOP			241
#define TELNET_CMD_DATA_MARK		242
#define TELNET_CMD_BREAK		243
#define TELNET_CMD_INTERRUPT		244
#define TELNET_CMD_ABORT_OUTPUT		245
#define TELNET_CMD_ARE_YOU_THERE	246
#define TELNET_CMD_ERASE_CHARACTER	247
#define TELNET_CMD_ERASE_LINE		248
#define TELNET_CMD_GO_AHEAD		250
#define TELNET_CMD_WILL			251
#define TELNET_CMD_WONT			252
#define TELNET_CMD_DO			253
#define TELNET_CMD_DONT			254
#define TELNET_CMD_IAC			255


extern struct sdm_interface_type sdm_telnet_type;

int init_telnet(void);
int release_telnet(void);

void sdm_telnet_release(struct sdm_tcp *);

int sdm_telnet_read(struct sdm_tcp *, char *, int);
int sdm_telnet_write(struct sdm_tcp *, const char *);
void sdm_telnet_echo(struct sdm_tcp *, int);

void sdm_telnet_encrypt_password(const char *, char *, int);

int sdm_telnet_login(struct sdm_tcp *);
int sdm_telnet_register(struct sdm_tcp *, struct sdm_login *);
int sdm_telnet_run(struct sdm_tcp *, struct sdm_user *);

#endif

