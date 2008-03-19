/*
 * Module Name:	ansi.c
 * Description:	ANSI Escape Codes
 */

#include <sdm/hash.h>
#include <sdm/memory.h>

#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/interfaces/telnet.h>
#include "ansi.h"

static struct sdm_hash *telnet_attribs = NULL;

static void sdm_telnet_destroy_attrib(struct sdm_telnet_attrib *);

int init_telnet_ansi(void)
{
	if (telnet_attribs)
		return(1);
	if (!(telnet_attribs = create_sdm_hash(SDM_HBF_CASE_INSENSITIVE, (destroy_t) sdm_telnet_destroy_attrib)))
		return(-1);

	sdm_telnet_add_attrib("b", SDM_ATTR_BOLD, 0, 0);
	sdm_telnet_add_attrib("black", 0, SDM_ATTR_BLACK, 0);
	sdm_telnet_add_attrib("blue", 0, SDM_ATTR_BLUE, 0);
	sdm_telnet_add_attrib("brightblue", SDM_ATTR_BOLD, SDM_ATTR_BLUE, 0);
	sdm_telnet_add_attrib("brightcyan", SDM_ATTR_BOLD, SDM_ATTR_CYAN, 0);
	sdm_telnet_add_attrib("brightgreen", SDM_ATTR_BOLD, SDM_ATTR_GREEN, 0);
	sdm_telnet_add_attrib("brightmagenta", SDM_ATTR_BOLD, SDM_ATTR_MAGENTA, 0);
	sdm_telnet_add_attrib("brightred", SDM_ATTR_BOLD, SDM_ATTR_RED, 0);
	sdm_telnet_add_attrib("brightwhite", SDM_ATTR_BOLD, SDM_ATTR_WHITE, 0);
	sdm_telnet_add_attrib("brightyellow", SDM_ATTR_BOLD, SDM_ATTR_YELLOW, 0);
	sdm_telnet_add_attrib("cyan", 0, SDM_ATTR_CYAN, 0);
	sdm_telnet_add_attrib("f", SDM_ATTR_FLASH, 0, 0);
	sdm_telnet_add_attrib("green", 0, SDM_ATTR_GREEN, 0);
	sdm_telnet_add_attrib("magenta", 0, SDM_ATTR_MAGENTA, 0);
	sdm_telnet_add_attrib("red", 0, SDM_ATTR_RED, 0);
	sdm_telnet_add_attrib("u", SDM_ATTR_UNDERLINE, 0, 0);
	sdm_telnet_add_attrib("white", 0, SDM_ATTR_WHITE, 0);
	sdm_telnet_add_attrib("yellow", 0, SDM_ATTR_YELLOW, 0);
	return(0);
}

int release_telnet_ansi(void)
{
	if (!telnet_attribs)
		return(1);
	destroy_sdm_hash(telnet_attribs);
	telnet_attribs = NULL;
	return(0);
}


int sdm_telnet_add_attrib(const char *name, char style, char fg, char bg)
{
	struct sdm_telnet_attrib *attrib;

	if (!(attrib = (struct sdm_telnet_attrib *) memory_alloc(sizeof(struct sdm_telnet_attrib))))
		return(-1);
	attrib->style = style;
	attrib->fg = fg;
	attrib->bg = bg;

	if (sdm_hash_add(telnet_attribs, name, attrib) < 0) {
		memory_free(attrib);
		return(-1);
	}
	return(0);
}


void sdm_telnet_reset_attribs(struct sdm_telnet *inter)
{
	sdm_tcp_send(SDM_TCP(inter), "\x1b[m", 3);
}

int sdm_telnet_write_attrib(struct sdm_telnet *inter, const char *name, char *buffer, int max, struct sdm_telnet_attrib_stack *stack)
{
	int i = 0;
	int close = 0;
	struct sdm_telnet_attrib *attrib;

	if (name[0] == '/') {
		close = 1;
		name = &name[1];
	}
	if (!(attrib = (struct sdm_telnet_attrib *) sdm_hash_find(telnet_attribs, name)))
		return(0);
	if (!close && stack->sp + 1 >= SDM_TELNET_MAX_ATTRIB)
		return(0);

	strncpy(buffer, "\x1b[", 2);
	i = 2;
	if (close) {
		if (stack->sp)
			stack->sp--;
		if (stack->sp) {
			buffer[i++] = '0';
			buffer[i++] = ';';
			if (stack->attribs[stack->sp - 1].style & SDM_ATTR_BOLD) {
				buffer[i++] = '1';
				buffer[i++] = ';';
			}
			if (stack->attribs[stack->sp - 1].style & SDM_ATTR_FLASH) {
				buffer[i++] = '5';
				buffer[i++] = ';';
			}
			buffer[i++] = '3';
			buffer[i++] = stack->attribs[stack->sp - 1].fg + 0x30;
			buffer[i++] = ';';
			buffer[i++] = '4';
			buffer[i++] = stack->attribs[stack->sp - 1].bg + 0x30;
			buffer[i++] = ';';
		}
	}
	else {
		stack->attribs[stack->sp].style = attrib->style;
		stack->attribs[stack->sp].fg = attrib->fg;
		stack->attribs[stack->sp].bg = attrib->bg;

		if (attrib->style & SDM_ATTR_BOLD) {
			buffer[i++] = '1';
			buffer[i++] = ';';
		}
		if (attrib->style & SDM_ATTR_FLASH) {
			buffer[i++] = '5';
			buffer[i++] = ';';
		}
		if (!stack->sp || attrib->fg != stack->attribs[stack->sp - 1].fg) {
			buffer[i++] = '3';
			buffer[i++] = attrib->fg + 0x30;
			buffer[i++] = ';';
		}
		if (!stack->sp || attrib->bg != stack->attribs[stack->sp - 1].bg) {
			buffer[i++] = '4';
			buffer[i++] = attrib->bg + 0x30;
			buffer[i++] = ';';
		}
		if (stack->sp)
			attrib->style |= stack->attribs[stack->sp - 1].style;
		stack->sp++;
	}
	if (buffer[i - 1] == ';')
		i--;
	buffer[i++] = 'm';
	return(i);
}


/*** Local Functions ***/

static void sdm_telnet_destroy_attrib(struct sdm_telnet_attrib *attrib)
{
	memory_free(attrib);
}












/*
int sdm_telnet_write(struct sdm_telnet *inter, const char *str)
{
	int i, j, k;
	int attr_sp = 0;
	char buffer[STRING_SIZE];
	struct sdm_telnet_attrib attribs[MAX_ATTRIB];

	for (i = 0, j = 0; (j < STRING_SIZE) && (str[i] != '\0'); i++) {
		switch (str[i]) {
		    case '\n': {
			buffer[j++] = '\r';
			buffer[j++] = '\n';
			break;
		    }
		    case '<': {
			k = j;
			buffer[j++] = str[i++];
			if (str[i] == '/')
				buffer[j++] = str[i++];
			for (; (j < STRING_SIZE) && (str[i] != '\0'); i++, j++) {
				// Break at the first non-alphanumeric or '_' character.
				if (!(((str[i] >= '0') && (str[i] <= '9'))
				    || ((str[i] >= 'A') && (str[i] <= 'Z'))
				    || ((str[i] >= 'a') && (str[i] <= 'z')) || (str[i] == '_')))
					break;
				buffer[j] = str[i];
			}
			if (str[i] != '>') {
				// This wasn't a tag so we'll ignore it
				buffer[j++] = str[i];
				break;
			}
			buffer[j] = '\0';
			j = k + sdm_telnet_rewrite_attrib(inter, &buffer[k], attribs, &attr_sp);
			break;
		    }
		    case '&': {
			if (!strncmp(&str[i + 1], "lt;", 3)) {
				buffer[j++] = '<';
				i += 3;
			}
			else if (!strncmp(&str[i + 1], "gt;", 3)) {
				buffer[j++] = '>';
				i += 3;
			}
			else if (!strncmp(&str[i + 1], "amp;", 4)) {
				buffer[j++] = '&';
				i += 4;
			}
			else if (!strncmp(&str[i + 1], "quot;", 5)) {
				buffer[j++] = '\"';
				i += 5;
			}
			else
				buffer[j++] = '&';
			break;
		    }
		    default: {
			if (str[i] >= 0x20)
				buffer[j++] = str[i];
			break;
		    }
		}
	}

	// If we haven't closed all attribute tags then send the reset escape to put us back to normal
	if (attr_sp) {
		if (j + 3 < STRING_SIZE) {
			strncpy(&buffer[j], "\x1b[m", 3);
			j += 3;
		}
		else {
			strncpy(&buffer[STRING_SIZE - 3], "\x1b[m", 3);
			j = STRING_SIZE;
		}
	}
	if (sdm_tcp_send(SDM_TCP(inter), buffer, j) < 0)
		return(-1);
	return(j);
}
*/

