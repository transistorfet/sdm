/*
 * Module Name:	msg.c
 * Description:	IRC Message Interface
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/irc/msg.h>

#include <sdm/misc.h>
#include <sdm/globals.h>

using namespace MooIRC;

#define IRC_MAX_MSG		512

Msg::Msg()
{

}

Msg::~Msg()
{

}

void Msg::debug_print()
{
	printf("%s\n", m_text);
}


Msg *Msg::read(MooTCP *inter)
{
	int size;
	Msg *msg;
	char buffer[IRC_MAX_MSG];

	if ((size = inter->receive(buffer, IRC_MAX_MSG, '\n')) <= 0)
		return(NULL);
	msg = new Msg();
	if (msg->unmarshal(buffer, size) < 0)
		return(NULL);
	return(msg);
}

int Msg::unmarshal(char *buffer, int max)
{
	strcpy(m_text, buffer);
	return(0);
}

