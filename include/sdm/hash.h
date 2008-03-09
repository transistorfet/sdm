/*
 * Name:	hash.h
 * Description:	Hash Table Header
 */


#ifndef _SDM_HASH_H
#define _SDM_HASH_H

#include <sdm/globals.h>

#define SDM_BBF_NO_ADD			0x0001			/** No entries can be added */
#define SDM_BBF_NO_REPLACE		0x0002			/** No entries can be replaced */
#define SDM_BBF_NO_REMOVE		0x0004			/** No entries can be removed */
#define SDM_BBF_CONSTANT		0x0006			/** No entries can be replaced or removed */
#define SDM_BBF_CASE_INSENSITIVE	0x0008			/** Ignore case when looking up entries */

struct sdm_entry {
	char *name;
	void *data;
	struct sdm_entry *next;
};

struct sdm_hash {
	int bitflags;
	destroy_t destroy;
	int traverse_index;
	struct sdm_entry *traverse_next;
	int size;
	int entries;
	struct sdm_entry **table;
};

struct sdm_hash *create_sdm_hash(short, destroy_t);
void destroy_sdm_hash(struct sdm_hash *);

int sdm_hash_add(struct sdm_hash *, const char *, void *);
int sdm_hash_replace(struct sdm_hash *, const char *, void *);
int sdm_hash_remove(struct sdm_hash *, const char *);

void *sdm_hash_find(struct sdm_hash *, const char *);

void sdm_hash_traverse_reset(struct sdm_hash *);
void *sdm_hash_traverse_next(struct sdm_hash *);

#endif

