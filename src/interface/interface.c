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

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/interface/interface.h>

#define INTERFACE_INIT_SIZE		4
#define INTERFACE_GROWTH_FACTOR		2

struct sdm_interface_list {
	int size;
	int next_space;
	struct sdm_interface **table;
};

static struct sdm_interface_list interface_list;

int init_interface(void)
{
	interface_list.size = INTERFACE_INIT_SIZE;
	interface_list.next_space = 0;
	if (!(interface_list.table = (struct sdm_interface **) memory_alloc(interface_list.size * sizeof(struct sdm_interface *))))
		return(-1);
	memset(interface_list.table, '\0', interface_list.size * sizeof(struct sdm_interface *));
	return(0);
}

int release_interface(void)
{
	int i;

	for (i = 0;i < interface_list.size;i++) {
		if (interface_list.table[i]) {
			if (interface_list.table[i]->type->release)
				interface_list.table[i]->type->release(interface_list.table[i]);
			memory_free(interface_list.table[i]);
		}
	}
	memory_free(interface_list.table);
	return(0);
}

/**
 * Create a new interface of the given type and with the given arguments.  A
 * pointer to the interface is returned or NULL on error.
 */
struct sdm_interface *create_sdm_interface(struct sdm_interface_type *type, ...)
{
	va_list va;
	int newsize;
	struct sdm_interface *inter;
	struct sdm_interface **newtable;

	if (!(inter = (struct sdm_interface *) memory_alloc(type->size)))
		return(NULL);
	inter->type = type;
	inter->bitflags = 0;
	inter->condition = 0;
	inter->callback.func = NULL;
	inter->read = -1;
	inter->write = -1;
	inter->error = -1;

	if (interface_list.next_space == interface_list.size) {
		newsize = interface_list.size * INTERFACE_GROWTH_FACTOR;
		if (!(newtable = (struct sdm_interface **) memory_realloc(interface_list.table, newsize * sizeof(struct sdm_interface *)))) {
			memory_free(inter);
			return(NULL);
		}
		interface_list.table = newtable;
		memset(&interface_list.table[interface_list.size], '\0', (newsize - interface_list.size) * sizeof(struct sdm_interface *));
		interface_list.next_space = interface_list.size;
		interface_list.size = newsize;
	}
	interface_list.table[interface_list.next_space] = inter;
	for (;interface_list.next_space < interface_list.size;interface_list.next_space++) {
		if (!interface_list.table[interface_list.next_space])
			break;
	}

	va_start(va, type);
	if (type->init && type->init(inter, va)) {
		destroy_sdm_interface(inter);
		return(NULL);
	}
	va_end(va);
	return(inter);
}

/**
 * Destroy the given interface.  A 0 is returned on success or a negative
 * number on error.
 */
void destroy_sdm_interface(struct sdm_interface *inter)
{
	int i;

	/** We don't want to attempt to free this object twice */
	if (!inter || (inter->bitflags & SDM_IBF_RELEASING))
		return;
	inter->bitflags |= SDM_IBF_RELEASING;
	if (inter->type->release)
		inter->type->release(inter);
	memory_free(inter);

	for (i = 0;i < interface_list.size;i++) {
		if (interface_list.table[i] == inter) {
			interface_list.table[i] = NULL;
			if (i < interface_list.next_space)
				interface_list.next_space = i;
		}
	}	
}


/**
 * Sets the callback that occurs under the given condition for the given
 * descriptior.
 */
void sdm_interface_set_callback(struct sdm_interface *inter, int condition, callback_t func, void *ptr)
{
	inter->condition = condition;
	inter->callback.func = func;
	inter->callback.ptr = ptr;
}

/**
 * Returns the callback for the given interface.
 */
struct callback_s sdm_interface_get_callback(struct sdm_interface *inter)
{
	return(inter->callback);
}


/**
 * Check for activity on all interfaces for up to a maximum of t seconds.
 * If new activity is available, the appropriate callback is called using the
 * interface as the parameter.  The number of interfaces that were serviced is
 * returned or -1 if an error occurred.
 */
int sdm_interface_select(float t)
{
	int i;
	int max, ret = 0;
	fd_set rd, wr, err;
	struct timeval timeout;

	/** Check the buffer of each connection to see if any data is waiting
	    and return when each connection gets a chance to read data so that
	    we can check other events and remain responsive */
	for (i = 0;i < interface_list.size;i++) {
		if (!interface_list.table[i])
			continue;
		if ((interface_list.table[i]->condition & IO_COND_READ)
		  && (interface_list.table[i]->bitflags & IO_WAIT_READ)) {
			EXECUTE_CALLBACK(interface_list.table[i]->callback, interface_list.table[i]);
			ret++;
		}
		if (!interface_list.table[i])
			continue;
		if ((interface_list.table[i]->condition & IO_COND_WRITE)
		  && (interface_list.table[i]->bitflags & IO_WAIT_WRITE)) {
			EXECUTE_CALLBACK(interface_list.table[i]->callback, interface_list.table[i]);
			ret++;
		}
		if (!interface_list.table[i])
			continue;
		if ((interface_list.table[i]->condition & IO_COND_ERROR)
		  && (interface_list.table[i]->bitflags & IO_WAIT_ERROR)) {
			EXECUTE_CALLBACK(interface_list.table[i]->callback, interface_list.table[i]);
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
	for (i = 0;i < interface_list.size;i++) {
		if (!interface_list.table[i])
			continue;
		if (interface_list.table[i]->read != -1) {
			FD_SET(interface_list.table[i]->read, &rd);
			if (interface_list.table[i]->read > max)
				max = interface_list.table[i]->read;
		}
		if (interface_list.table[i]->write != -1) {
			FD_SET(interface_list.table[i]->write, &wr);
			if (interface_list.table[i]->write > max)
				max = interface_list.table[i]->write;
		}
		if (interface_list.table[i]->error != -1) {
			FD_SET(interface_list.table[i]->error, &err);
			if (interface_list.table[i]->error > max)
				max = interface_list.table[i]->error;
		}
	}

	if ((ret = select(max + 1, &rd, &wr, &err, &timeout)) == -1) {
		/** There was a socket error so we'll just return */
		return(-1);
	}

	for (i = 0;i < interface_list.size;i++) {
		/** We check that the interface is not NULL before each condition in case the previous
		    callback destroyed the interface */
		if (!interface_list.table[i])
			continue;
		if ((interface_list.table[i]->condition & IO_COND_READ)
		    && ((interface_list.table[i]->read != -1) && FD_ISSET(interface_list.table[i]->read, &rd)))
			EXECUTE_CALLBACK(interface_list.table[i]->callback, interface_list.table[i]);

		if (!interface_list.table[i])
			continue;
		if ((interface_list.table[i]->condition & IO_COND_WRITE)
		    && ((interface_list.table[i]->write != -1) && FD_ISSET(interface_list.table[i]->write, &wr)))
			EXECUTE_CALLBACK(interface_list.table[i]->callback, interface_list.table[i]);

		if (!interface_list.table[i])
			continue;
		if ((interface_list.table[i]->condition & IO_COND_ERROR)
		    && ((interface_list.table[i]->error != -1) && FD_ISSET(interface_list.table[i]->error, &err)))
			EXECUTE_CALLBACK(interface_list.table[i]->callback, interface_list.table[i]);
	}
	return(ret);
}



