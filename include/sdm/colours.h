/*
 * Name:	colours.h
 * Description:	Colour Tag Functions
 */

#ifndef _SDM_COLOURS_H
#define _SDM_COLOURS_H

#define MOO_A_COLOUR		0x0100
#define MOO_A_FLASH		0x0200
#define MOO_A_BOLD		0x0400
#define MOO_A_UNDER		0x0800
#define MOO_A_ITALIC		0x1000
#define MOO_A_REVERSE		0x2000

#define MOO_COLOUR_NUM		0x01ff
#define MOO_C_NORMAL		0x0100
#define MOO_C_BLACK		0x0101
#define MOO_C_BLUE		0x0102
#define MOO_C_GREEN		0x0103
#define MOO_C_LIGHTRED		0x0104
#define MOO_C_RED		0x0105
#define MOO_C_PURPLE		0x0106
#define MOO_C_ORANGE		0x0107
#define MOO_C_YELLOW		0x0108
#define MOO_C_LIGHTGREEN	0x0109
#define MOO_C_TEAL		0x010A
#define MOO_C_CYAN		0x010B
#define MOO_C_LIGHTBLUE		0x010C
#define MOO_C_MAGENTA		0x010D
#define MOO_C_DARKGREY		0x010E
#define MOO_C_GREY		0x010F
#define MOO_C_WHITE		0x0110

typedef int (*moo_colour_func_t)(int type, char *buffer, int max);

int moo_colour_format(moo_colour_func_t func, char *buffer, int max, const char *str);
int moo_colour_read_attrib(const char *attrib);


#endif

