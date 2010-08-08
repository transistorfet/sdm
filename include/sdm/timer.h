/*
 * Header Name:		timer.h
 * Description:		Timer Manager Header
 */

#ifndef _SDM_TIMER_H
#define _SDM_TIMER_H

#include <sdm/globals.h>

#define MOO_TBF_ONE_TIME	0x00
#define MOO_TBF_PERIODIC	0x01
#define MOO_TBF_EXPIRED		0x80

class MooTimerHandler {
    public:
	virtual int timer_close() = 0;
	virtual int handle_timer(class MooTimer *timer) = 0;
};

class MooTimer {
	int m_bits;
	float m_interval;
	time_t m_start;
	MooTimerHandler *m_handler;
	MooTimer *m_next;
	MooTimer *m_prev;

	void insert();
	void remove();
    public:
	MooTimer(int bits, float interval, MooTimerHandler *handler);
	~MooTimer();
	MooTimerHandler *handler() { return(m_handler); }
	MooTimerHandler *handler(MooTimerHandler *handler);

	int reset();
	int expire();
	int set(float interval);

	static int check();

	static void delete_all();
};

int init_timer(void);
void release_timer(void);

#endif

