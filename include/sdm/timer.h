/*
 * Header Name:		timer.h
 * Description:		Timer Manager Header
 */

#ifndef _SDM_TIMER_H
#define _SDM_TIMER_H

#include <sdm/globals.h>

#define SDM_TBF_ONE_TIME	0x00
#define SDM_TBF_PERIODIC	0x01
#define SDM_TBF_EXPIRED		0x80

struct sdm_timer;

int init_timer(void);
int release_timer(void);

struct sdm_timer *create_sdm_timer(int, float, callback_t, void *);
void destroy_sdm_timer(struct sdm_timer *);

struct callback_s sdm_timer_get_callback(struct sdm_timer *);
void sdm_timer_set_callback(struct sdm_timer *, callback_t, void *);

int sdm_timer_reset(struct sdm_timer *);
int sdm_timer_expire(struct sdm_timer *);
int sdm_timer_set_interval(struct sdm_timer *, float);

int sdm_timer_check(void);

#endif

