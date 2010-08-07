/*
 * Module Name:	interface.c
 * Description:	Interface Manager
 */

#include <string.h>
#include <stdarg.h>

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include <sdm/array.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/tasks/task.h>
#include <sdm/objs/object.h>
#include <sdm/interfaces/interface.h>

static MooArray<MooInterface *> *interface_list = NULL;

MooObjectType moo_interface_obj_type = {
	NULL,
	"interface",
	typeid(MooInterface).name(),
	(moo_type_create_t) NULL
};

int init_interface(void)
{
	if (!interface_list)
		interface_list = new MooArray<MooInterface *>();
	return(0);
}

void release_interface(void)
{
	delete interface_list;
	interface_list = NULL;
}

MooInterface::MooInterface()
{
	m_bits = 0;
	m_rfd = -1;
	m_wfd = -1;
	m_efd = -1;
	m_task = NULL;

	interface_list->add(this);
}

MooInterface::~MooInterface()
{
	interface_list->remove(this);
	if (m_rfd > 0)
		close(m_rfd);
	if ((m_wfd > 0) && (m_wfd != m_rfd))
		close(m_wfd);
	if ((m_efd > 0) && (m_efd != m_rfd) && (m_efd != m_wfd))
		close(m_efd);
}


/**
 * Check for activity on all interfaces for up to a maximum of t seconds.
 * If new activity is available, the appropriate callback is called using the
 * interface as the parameter.  The number of interfaces that were serviced is
 * returned or -1 if an error occurred.
 */
int MooInterface::wait(float t)
{
	int i;
	int state;
	int max, ret = 0;
	MooInterface *cur;
	fd_set rd, wr, err;
	struct timeval timeout;

	/** Check the buffer of each connection to see if any data is waiting
	    and return when each connection gets a chance to read data so that
	    we can check other events and remain responsive */
	for (i = 0; i < interface_list->size(); i++) {
		if (!(cur = interface_list->get(i)) || !cur->m_task)
			continue;
		if ((state = (cur->m_bits & IO_STATE))) {
			cur->m_bits &= ~IO_STATE;
			cur->m_task->handle(cur, state);
			ret++;
		}
	}
	if (ret)
		return(ret);

	/** Check each connection's socket for input using select */
	timeout.tv_sec = (int) t;
	timeout.tv_usec = (int) ((t - timeout.tv_sec) * 1000000);

	FD_ZERO(&rd);
	FD_ZERO(&wr);
	FD_ZERO(&err);
	max = 0;
	for (i = 0;i < interface_list->size();i++) {
		if (!(cur = interface_list->get(i)))
			continue;
		if (cur->m_rfd != -1) {
			FD_SET(cur->m_rfd, &rd);
			if (cur->m_rfd > max)
				max = cur->m_rfd;
		}
		if (cur->m_wfd != -1) {
			FD_SET(cur->m_wfd, &wr);
			if (cur->m_wfd > max)
				max = cur->m_wfd;
		}
		if (cur->m_efd != -1) {
			FD_SET(cur->m_efd, &err);
			if (cur->m_efd > max)
				max = cur->m_efd;
		}
	}

	if ((ret = ::select(max + 1, &rd, &wr, &err, &timeout)) == -1) {
		/** There was a socket error so we'll just return */
		return(-1);
	}

	for (i = 0;i < interface_list->size();i++) {
		/** We check that the interface is not NULL before each condition in case the previous
		    callback destroyed the interface */
		if (!(cur = interface_list->get(i)) || !cur->m_task)
			continue;
		if ((cur->m_rfd != -1) && FD_ISSET(cur->m_rfd, &rd))
			cur->m_bits |= IO_READY_READ;
		if ((cur->m_wfd != -1) && FD_ISSET(cur->m_wfd, &wr))
			cur->m_bits |= IO_READY_WRITE;
		if ((cur->m_efd != -1) && FD_ISSET(cur->m_efd, &err))
			cur->m_bits |= IO_READY_ERROR;

		if ((state = (cur->m_bits & IO_STATE))) {
			cur->m_bits &= ~IO_STATE;
			cur->m_task->handle(cur, state);
		}
	}
	return(ret);
}



