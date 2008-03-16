/*
 * Header Name:	ansi.h
 * Description:	ANSI Escape Codes Header
 */

#ifndef _SDM_INTERFACE_TELNET_ANSI_H
#define _SDM_INTERFACE_TELNET_ANSI_H

#include <sdm/globals.h>

#define SDM_TELNET_MAX_ATTRIB	100

#define SDM_ATTR_BOLD		0x01
#define SDM_ATTR_FLASH		0x02
#define SDM_ATTR_UNDERLINE	0x04

#define SDM_ATTR_BLACK		0
#define SDM_ATTR_RED		1
#define SDM_ATTR_GREEN		2
#define SDM_ATTR_YELLOW		3
#define SDM_ATTR_BLUE		4
#define SDM_ATTR_MAGENTA	5
#define SDM_ATTR_CYAN		6
#define SDM_ATTR_WHITE		7

struct sdm_telnet_attrib {
	char style;
	char fg;
	char bg;
};

struct sdm_telnet_attrib_stack {
	int sp;
	struct sdm_telnet_attrib attribs[SDM_TELNET_MAX_ATTRIB];
};

int init_telnet_ansi(void);
int release_telnet_ansi(void);

int sdm_telnet_add_attrib(const char *, char, char, char);
void sdm_telnet_reset_attribs(struct sdm_telnet *);
int sdm_telnet_write_attrib(struct sdm_telnet *, const char *, char *, int, struct sdm_telnet_attrib_stack *);

#endif

