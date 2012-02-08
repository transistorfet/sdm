/*
 * Name:	users.cpp
 * Description:	User Methods
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/code/code.h>
#include <sdm/objs/thing.h>
#include <sdm/drivers/driver.h>
#include <sdm/drivers/tcp.h>

#include <sdm/lib/users.h>


static int telnet_read_line(MooCodeFrame *frame, MooObjectArray *args)
{
	int size;
	MooTCP *driver;
	MooThing *m_this;
	char buffer[LARGE_STRING_SIZE];

	if (args->last() != 1)
		throw moo_args_mismatched;
	if (!(m_this = dynamic_cast<MooThing *>(args->get(0))))
		throw moo_method_object;
	if (!(driver = dynamic_cast<MooTCP *>(args->get(1))))
		throw MooException("Expected MooTCP driver");
	//if (!(ready & IO_READY_READ))
	//	return(-1);

	if ((size = driver->receive(buffer, LARGE_STRING_SIZE, '\n')) <= 0)
		return(1);
	moo_status("TELNET: DEBUG RECEIVED: %s", buffer);
	while (buffer[size] == '\n' || buffer[size] == '\r')
		buffer[size--] = '\0';
	frame->set_return(new MooString("%s", buffer));
	return(0);
}


int moo_load_telnet_methods(MooObjectHash *env)
{
	env->set("%telnet_read_line", new MooFuncPtr(telnet_read_line));
	return(0);
}

