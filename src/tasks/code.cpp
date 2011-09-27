/*
 * Task Name:	code.cpp
 * Description:	Code Task
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sdm/data.h>
#include <sdm/globals.h>

#include <sdm/objs/hash.h>
#include <sdm/objs/thing.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/tcp.h>
#include <sdm/tasks/task.h>
#include <sdm/tasks/code.h>


MooObjectType moo_code_task_obj_type = {
	&moo_task_obj_type,
	"code-task",
	typeid(MooCodeTask).name(),
	(moo_type_make_t) make_moo_code_task
};

MooObject *make_moo_code_task(MooDataFile *data)
{
	MooCodeTask *obj = new MooCodeTask();
	if (data)
		obj->read_data(data);
	return(obj);
}

MooCodeTask::MooCodeTask()
{
	m_env = new MooObjectHash();
	m_frame = new MooCodeFrame(m_env);
}

MooCodeTask::MooCodeTask(MooThing *user, MooThing *channel)
{
	m_env = new MooObjectHash();
	m_frame = new MooCodeFrame(m_env);
	m_env->set("user", user);
	m_env->set("channel", channel);
}

MooCodeTask::~MooCodeTask()
{
	this->set_delete();

	MOO_DECREF(m_frame);
	MOO_DECREF(m_env);
}

int MooCodeTask::read_entry(const char *type, MooDataFile *data)
{
	char buffer[STRING_SIZE];

/*
	if (!strcmp(type, "port")) {
		m_port = data->read_integer_entry();
		this->listen(m_port);
	}
	else if (!strcmp(type, "type")) {
		data->read_string_entry(buffer, STRING_SIZE);
		if (!(m_itype = moo_object_find_type((*buffer != '\0') ? buffer : "tcp", &moo_tcp_obj_type)))
			throw MooException("No tcp type, %s", buffer);
	}
	else if (!strcmp(type, "task")) {
		data->read_string_entry(buffer, STRING_SIZE);
		if (!(m_ttype = moo_object_find_type(buffer, &moo_task_obj_type)))
			throw MooException("No task type, %s", buffer);
	}
	else
		return(MOO_NOT_HANDLED);
*/
	return(MOO_HANDLED);
}


int MooCodeTask::write_data(MooDataFile *data)
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

int MooCodeTask::initialize()
{

	return(0);
}

int MooCodeTask::idle()
{
	int cycles;

	try {
		cycles = m_frame->run();
	}
	catch (MooException e) {
		// TODO temporary for debugging purposes??
		moo_status("CODE: %s", e.get());
		m_frame->print_stacktrace();
		cycles = -1;
	}
	return(cycles);
}

int MooCodeTask::release()
{

	return(0);
}

int MooCodeTask::handle(MooInterface *inter, int ready)
{
/*
	MooTCP *newtcp;
	MooTask *newtask;

	// TODO should we try to make this handle errors a bit better instead of destroying itself when something goes wrong
	if (!(ready & IO_READY_READ)) {
		delete inter;
		return(-1);
	}
	if (!(newtcp = (MooTCP *) moo_make_object(m_itype, NULL))) {
		delete this;
		return(-1);
	}
	if ((m_inter->accept(newtcp) < 0)
	    || !(newtask = (MooTask *) moo_make_object(m_ttype, NULL))) {
		delete newtcp;
		return(-1);
	}
	newtask->bestow(newtcp);
*/
	return(0);
}

int MooCodeTask::purge(MooInterface *inter)
{
/*
	if (inter != m_inter)
		return(-1);
	/// We assume that since we were called because the interface is already being deleted, in which case we don't want to delete too
	m_inter = NULL;
	if (!this->is_deleting())
		delete this;
*/
	return(0);
}

int MooCodeTask::push_call(MooObject *func, MooObject *arg1, MooObject *arg2, MooObject *arg3)
{
	MooArgs *args;

	args = new MooArgs();
	if (arg1) {
		args->m_args->set(0, arg1);
		if (arg2) {
			args->m_args->set(1, arg2);
			if (arg3)
				args->m_args->set(2, arg3);
		}
	}

	return(m_frame->push_call(m_env, func, args));
}

int MooCodeTask::push_code(const char *code)
{
	return(m_frame->push_code(code));
}


