/*
 * Name:	colours.cpp
 * Description:	Colour Tag Functions
 */

#include <string.h>

#include <sdm/colours.h>

int moo_colour_format(moo_colour_func_t func, char *buffer, int max, const char *str)
{
	int i, j, k, type, close;

	for (i = 0, j = 0; (j < max) && (str[i] != '\0'); i++) {
		switch (str[i]) {
		    case '\n': {
			buffer[j++] = '\020';
			buffer[j++] = 'n';
			break;
		    }
		    case '<': {
			k = i + 1;
			close = 0;
			if (str[k] == '/') {
				close = 1;
				k++;
			}
			// TODO add support for closing tags
			type = moo_colour_read_attrib(&str[k]);
			/// If a recognized tag was not found, then copy it through to the output
			if (!type)
				buffer[j++] = '<';
			else {
				j += func(type, &buffer[j], max - j - 1);
				for (; str[i] != '\0' && str[i] != '>'; i++)
					;
			}
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
	buffer[j] = '\0';
	return(j);
}

int moo_colour_read_attrib(const char *attrib)
{
	if (attrib[0] == 'b') {
		if (attrib[1] == '>')
			return(MOO_A_BOLD);
		else if (attrib[1] == 'l') {
			if (!strncmp(&attrib[2], "ack>", 4))
				return(MOO_C_BLACK);
			else if (!strncmp(&attrib[2], "ue>", 3))
				return(MOO_C_BLUE);
		}
	}
	else if (!strncmp(attrib, "cyan>", 5))
		return(MOO_C_CYAN);
	else if (!strncmp(attrib, "darkgrey>", 9))
		return(MOO_C_DARKGREY);
	else if (!strncmp(attrib, "f>", 2))
		return(MOO_A_FLASH);
	else if (!strncmp(attrib, "gre", 3)) {
		if (!strncmp(&attrib[3], "y>", 2))
			return(MOO_C_GREY);
		else if (!strncmp(&attrib[3], "en>", 3))
			return(MOO_C_GREEN);
	}
	else if (!strncmp(attrib, "i>", 2))
		return(MOO_A_ITALIC);
	else if (!strncmp(attrib, "light", 5)) {
		if (!strncmp(&attrib[5], "blue>", 5))
			return(MOO_C_LIGHTBLUE);
		else if (!strncmp(&attrib[5], "green>", 6))
			return(MOO_C_LIGHTGREEN);
		else if (!strncmp(&attrib[5], "red>", 4))
			return(MOO_C_LIGHTRED);
	}
	else if (!strncmp(attrib, "magenta>", 8))
		return(MOO_C_MAGENTA);
	else if (!strncmp(attrib, "orange>", 7))
		return(MOO_C_ORANGE);
	else if (!strncmp(attrib, "purple>", 7))
		return(MOO_C_PURPLE);
	else if (!strncmp(attrib, "r>", 2))
		return(MOO_A_REVERSE);
	else if (!strncmp(attrib, "red>", 4))
		return(MOO_C_RED);
	else if (!strncmp(attrib, "teal>", 5))
		return(MOO_C_TEAL);
	else if (!strncmp(attrib, "u>", 2))
		return(MOO_A_UNDER);
	else if (!strncmp(attrib, "white>", 6))
		return(MOO_C_WHITE);
	else if (!strncmp(attrib, "yellow>", 7))
		return(MOO_C_YELLOW);
	return(0);
}


