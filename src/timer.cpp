/*
 * Name:	timer.c
 * Description:	Timer Manager
 */

#include <time.h>

#include <sdm/timer.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

class MooTimer *timer_list = NULL;

int init_timer(void)
{
	return(0);
}

void release_timer(void)
{
	MooTimer::delete_all();
}

MooTimer::MooTimer(int bits, float interval, MooTimerHandler *handler)
{
	m_bits = bits;
	m_interval = interval;
	m_start = time(NULL);
	m_handler = handler;
	m_prev = NULL;
	m_next = NULL;
	this->insert();
}

MooTimer::~MooTimer()
{
	this->remove();
	if (m_handler)
		m_handler->timer_close();
}

MooTimerHandler *MooTimer::handler(MooTimerHandler *handler)
{
	if (m_handler)
		m_handler->timer_close();
	m_handler = handler;
	return(m_handler);
}

int MooTimer::reset()
{
	this->remove();
	m_bits &= ~MOO_TBF_EXPIRED;
	m_start = time(NULL);
	this->insert();
	return(0);
}


int MooTimer::expire()
{
	// TODO does this actually work?
	m_bits |= MOO_TBF_EXPIRED;
	return(0);
}


int MooTimer::set(float interval)
{
	this->remove();
	m_interval = interval;
	this->insert();
	return(0);
}


int MooTimer::check()
{
	time_t current_time;
	MooTimer *cur;

	current_time = time(NULL);
	for (cur = timer_list; cur; cur = cur->m_next) {
		if (cur->m_bits & MOO_TBF_EXPIRED)
			continue;
		else if ((current_time - cur->m_start) >= cur->m_interval) {
			cur->m_bits |= MOO_TBF_EXPIRED;
			// TODO switch to the owner/task saved
			if (cur->m_handler)
				cur->m_handler->handle_timer(cur);
			if (cur->m_bits & MOO_TBF_PERIODIC)
				cur->reset();
		}
		else
			break;
	}
	return(0);
}

void MooTimer::delete_all()
{
	MooTimer *cur, *tmp;

	for (cur = timer_list;cur;) {
		tmp = cur->m_next;
		delete cur;
		cur = tmp;
	}
	timer_list = NULL;
}

void MooTimer::insert()
{
	double expiration;
	MooTimer *cur, *prev;

	if (!timer_list) {
		timer_list = this;
		m_next = NULL;
		m_prev = NULL;
	}
	else {
		expiration = m_start + m_interval;
		for (prev = NULL, cur = timer_list; cur; prev = cur, cur = cur->m_next) {
			if (expiration <= (cur->m_start + cur->m_interval))
				break;
		}
		m_prev = prev;
		m_next = cur;
		if (prev)
			prev->m_next = this;
		else
			timer_list = this;
		if (cur)
			cur->m_prev = this;
	}
}

void MooTimer::remove()
{
	if (m_prev)
		m_prev->m_next = m_next;
	else
		timer_list = m_next;
	if (m_next)
		m_next->m_prev = m_prev;
	m_prev = NULL;
	m_next = NULL;
}


