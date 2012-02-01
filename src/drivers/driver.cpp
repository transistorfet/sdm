/*
 * Module Name:	driver.c
 * Description:	Driver Manager
 */

#include <string.h>
#include <stdarg.h>

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include <sdm/globals.h>

#include <sdm/objs/array.h>
#include <sdm/objs/object.h>
#include <sdm/drivers/driver.h>

#define DRIVER_LIST_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE

static MooArray<MooDriver *> *driver_list = NULL;

int init_driver(void)
{
	if (!driver_list)
		driver_list = new MooArray<MooDriver *>(MOO_ARRAY_DEFAULT_SIZE, -1, DRIVER_LIST_BITS);
	return(0);
}

void release_driver(void)
{
	delete driver_list;
	driver_list = NULL;
}

MooDriver::MooDriver()
{
	m_bits = 0;
	m_rfd = -1;
	m_wfd = -1;
	m_efd = -1;

	// TODO should this instead be set by the function that creates the driver?
	m_owner = MooTask::current_owner();
	driver_list->add(this);
}

MooDriver::~MooDriver()
{
	driver_list->remove(this);
	if (m_rfd > 0)
		close(m_rfd);
	if ((m_wfd > 0) && (m_wfd != m_rfd))
		close(m_wfd);
	if ((m_efd > 0) && (m_efd != m_rfd) && (m_efd != m_wfd))
		close(m_efd);
}


/**
 * Check for activity on all drivers for up to a maximum of t seconds.
 * If new activity is available, the appropriate callback is called using the
 * driver as the parameter.  The number of drivers that were serviced is
 * returned or -1 if an error occurred.
 */
int MooDriver::wait(float t)
{
	int i;
	int state;
	int max, ret = 0;
	MooDriver *cur;
	fd_set rd, wr, err;
	struct timeval timeout;

	/// Check the buffer of each connection to see if any data is waiting
	/// and return when each connection gets a chance to read data so that
	/// we can check other events and remain responsive
	for (i = 0; i < driver_list->size(); i++) {
		if (!(cur = driver_list->get(i)))
			continue;
		if ((state = (cur->m_bits & IO_STATE))) {
			cur->m_bits &= ~IO_STATE;
			cur->handle(state);
			ret++;
		}
	}
	if (ret)
		return(ret);

	/// Check each connection's socket for input using select
	timeout.tv_sec = (int) t;
	timeout.tv_usec = (int) ((t - timeout.tv_sec) * 1000000);

	FD_ZERO(&rd);
	FD_ZERO(&wr);
	FD_ZERO(&err);
	max = 0;
	for (i = 0;i < driver_list->size();i++) {
		if (!(cur = driver_list->get(i)))
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
		/// There was a socket error so we'll just return
		return(-1);
	}

	for (i = 0;i < driver_list->size();i++) {
		/// We check that the driver is not NULL before each condition in case the previous
		/// callback destroyed the driver
		if (!(cur = driver_list->get(i)))
			continue;
		if ((cur->m_rfd != -1) && FD_ISSET(cur->m_rfd, &rd))
			cur->m_bits |= IO_READY_READ;
		if ((cur->m_wfd != -1) && FD_ISSET(cur->m_wfd, &wr))
			cur->m_bits |= IO_READY_WRITE;
		if ((cur->m_efd != -1) && FD_ISSET(cur->m_efd, &err))
			cur->m_bits |= IO_READY_ERROR;

		if ((state = (cur->m_bits & IO_STATE))) {
			cur->m_bits &= ~IO_STATE;
			cur->handle(state);
		}
	}
	return(ret);
}



