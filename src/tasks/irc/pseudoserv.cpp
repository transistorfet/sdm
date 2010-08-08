/*
 * Module Name:	irc.c
 * Description:	IRC PseudoServ Interface
 */

#include <string.h>
#include <stdarg.h>

#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/task.h>
#include <sdm/tasks/irc/msg.h>
#include <sdm/tasks/irc/pseudoserv.h>

#include <sdm/misc.h>
#include <sdm/globals.h>

MooObjectType moo_irc_pseudoserv_obj_type = {
	&moo_task_obj_type,
	"irc-pseudoserv",
	typeid(MooIRC::PseudoServ).name(),
	(moo_type_create_t) moo_irc_pseudoserv_create
};

MooObject *moo_irc_pseudoserv_create(void)
{
	return(new MooIRC::PseudoServ());
}

using namespace MooIRC;

PseudoServ::PseudoServ()
{
	m_inter = NULL;
	m_user = NULL;
}

PseudoServ::~PseudoServ()
{
	this->set_delete();
	if (m_inter)
		delete m_inter;
	if (m_user)
		m_user->disconnect();
}

int PseudoServ::read_entry(const char *type, MooDataFile *data)
{
/*
	if (!strcmp(type, "port")) {
		m_port = data->read_integer_entry();
		//this->listen(m_port);
	}
	else if (!strcmp(type, "type")) {
		// TODO read in string and find type
	}
	else
		return(MOO_NOT_HANDLED);
*/
	return(MOO_HANDLED);
}


int PseudoServ::write_data(MooDataFile *data)
{
/*
	data->write_integer_entry("port", m_port);
	if (m_itype)
		data->write_string_entry("type", m_itype->m_name);
	if (m_ttype)
		data->write_string_entry("task", m_ttype->m_name);
*/
	return(0);
}


int PseudoServ::initialize()
{
	return(0);
}

int PseudoServ::idle()
{
	return(0);
}

int PseudoServ::release()
{
	return(0);
}

int PseudoServ::handle(MooInterface *inter, int ready)
{
	Msg *msg;

	if (!(ready & IO_READY_READ))
		return(-1);
	try {
		msg = Msg::read(dynamic_cast<MooTCP *>(inter));
		if (!msg)
			return(1);
		this->dispatch(msg);
	}
	catch (MooException e) {
		moo_status("IRC: %s", e.get());
		delete this;
	}
	return(0);
}

int PseudoServ::bestow(MooInterface *inter)
{
	if (m_inter)
		delete m_inter;
	m_inter = dynamic_cast<MooTCP *>(inter);
	m_inter->set_task(this);
	return(0);
}

int PseudoServ::purge(MooInterface *inter)
{
	if (inter != m_inter)
		return(-1);
	// We assume that since we were called because the interface is already being deleted, in which case we don't want to delete too
	m_inter = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::purge(MooUser *user)
{
	if (user != m_user)
		return(-1);
	// We assume that since we were called because the user is already being deleted, in which case we don't want to delete too
	m_user = NULL;
	if (!this->is_deleting())
		delete this;
	return(0);
}

int PseudoServ::dispatch(Msg *msg)
{
	msg->debug_print();
	delete msg;
	return(0);
}

