/*
 * Name:	timer.c
 * Description:	Timer Manager
 */

#include <time.h>

#include <sdm/timer.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

struct sdm_timer {
	int bitflags;
	struct callback_s callback;
	float interval;
	time_t start;
	struct sdm_timer *prev;
	struct sdm_timer *next;
};

struct sdm_timer *timer_list = NULL;

static void sdm_timer_insert(struct sdm_timer *);
static void sdm_timer_remove(struct sdm_timer *);

int init_timer(void)
{
	return(0);
}

void release_timer(void)
{
	struct sdm_timer *cur, *tmp;

	for (cur = timer_list;cur;) {
		tmp = cur->next;
		memory_free(cur);
		cur = tmp;
	}
}

/**
 * Execute the given command and return a reference to the running program.
 */
struct sdm_timer *create_sdm_timer(int bitflags, float interval, callback_t func, void *ptr)
{
	struct sdm_timer *timer;

	if (!(timer = (struct sdm_timer *) memory_alloc(sizeof(struct sdm_timer))))
		return(NULL);
	timer->bitflags = bitflags;
	timer->callback.func = func;
	timer->callback.ptr = ptr;
	timer->interval = interval;
	timer->start = time(NULL);
	timer->prev = NULL;
	timer->next = NULL;

	sdm_timer_insert(timer);
	return(timer);
}

/**
 * Close the given program reference.
 */
void destroy_sdm_timer(struct sdm_timer *timer)
{
	sdm_timer_remove(timer);
	memory_free(timer);
}


/**
 * Returns the callback for the given timer.
 */
struct callback_s sdm_timer_get_callback(struct sdm_timer *timer)
{
	return(timer->callback);
}

/**
 * Sets the callback of the given timer.
 */
void sdm_timer_set_callback(struct sdm_timer *timer, callback_t func, void *ptr)
{
	timer->callback.func = func;
	timer->callback.ptr = ptr;
}


/**
 * Reset the given timer's start time to the current time.  If the timer has
 * already expired, it will start counting again.  If an error occurs, -1 is
 * returned; otherwise 0 is returned.
 */
int sdm_timer_reset(struct sdm_timer *timer)
{
	sdm_timer_remove(timer);
	timer->bitflags &= ~SDM_TBF_EXPIRED;
	timer->start = time(NULL);
	sdm_timer_insert(timer);
	return(0);
}

/**
 * Expires the timer.  If and error occurs, -1 is returned; otherwise 0 is
 * returned.
 */
int sdm_timer_expire(struct sdm_timer *timer)
{
	// TODO does this actually work?
	timer->bitflags |= SDM_TBF_EXPIRED;
	return(0);
}

/**
 * Set the given timer's interval to the given interval without resetting
 * the timer's time so far.  If the timer has already expired, 1 will be
 * returned.  If an error occurs, -1 will be returned.  Otherwise 0 is
 * returned.
 */
int sdm_timer_set_interval(struct sdm_timer *timer, float interval)
{
	sdm_timer_remove(timer);
	timer->interval = interval;
	sdm_timer_insert(timer);
	return(0);
}

/**
 * Check all timers for expiration.
 */
int sdm_timer_check(void)
{
	time_t current_time;
	struct sdm_timer *cur;

	current_time = time(NULL);
	for (cur = timer_list;cur;cur = cur->next) {
		if (cur->bitflags & SDM_TBF_EXPIRED)
			continue;
		else if ((current_time - cur->start) >= cur->interval) {
			cur->bitflags |= SDM_TBF_EXPIRED;
			EXECUTE_CALLBACK(cur->callback, cur);
			if (cur->bitflags & SDM_TBF_PERIODIC)
				sdm_timer_reset(cur);
		}
		else
			break;
	}
	return(0);
}

/*** Local Functions ***/

static void sdm_timer_insert(struct sdm_timer *timer)
{
	double expiration;
	struct sdm_timer *cur, *prev;

	if (!timer_list) {
		timer_list = timer;
		timer->next = NULL;
		timer->prev = NULL;
	}
	else {
		expiration = timer->start + timer->interval;
		for (prev = NULL, cur = timer_list;cur;prev = cur, cur = cur->next) {
			if (expiration <= (cur->start + cur->interval))
				break;
		}
		timer->prev = prev;
		timer->next = cur;
		if (prev)
			prev->next = timer;
		else
			timer_list = timer;
		if (cur)
			cur->prev = timer;
	}
}

static void sdm_timer_remove(struct sdm_timer *timer)
{
	if (timer->prev)
		timer->prev->next = timer->next;
	else
		timer_list = timer->next;
	if (timer->next)
		timer->next->prev = timer->prev;
	timer->prev = NULL;
	timer->next = NULL;
}

