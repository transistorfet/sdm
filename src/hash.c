/*
 * Name:	hash.c
 * Description:	Hash Table
 */


#include <stdlib.h>
#include <string.h>

#include <sdm/hash.h>
#include <sdm/memory.h>
#include <sdm/globals.h>

#define SDM_HASH_INIT_SIZE		32
#define SDM_HASH_LOAD_FACTOR		0.75
#define SDM_HASH_GROWTH_FACTOR		1.75

#define HASH_ENTRY_COMPARE(env, str1, str2) \
	( ((env)->bitflags & SDM_HBF_CASE_INSENSITIVE) ? !sdm_stricmp((str1), (str2)) : !strcmp((str1), (str2)) )

#define LOWERCASE(ch) \
	( (ch >= 0x41 && ch <= 0x5a) ? ch + 0x20 : ch )

static inline int sdm_entries_rehash(struct sdm_hash *, int);
static int sdm_stricmp(const char *, const char *);
static inline unsigned int sdm_hash_func(const char *);

/**
 * Allocate a hash table generic data.
 */
struct sdm_hash *create_sdm_hash(short bitflags, int init_size, destroy_t destroy)
{
	struct sdm_hash_entry **table;
	struct sdm_hash *env;

	if (init_size <= 0)
		init_size = SDM_HASH_INIT_SIZE;
	if (!(table = (struct sdm_hash_entry **) memory_alloc(init_size * sizeof(struct sdm_hash_entry *))))
		return(NULL);
	if (!(env = (struct sdm_hash *) memory_alloc(sizeof(struct sdm_hash)))) {
		memory_free(table);
		return(NULL);
	}
	env->bitflags = bitflags;
	env->destroy = destroy;
	env->traverse_index = 0;
	env->traverse_next = NULL;
	env->size = init_size;
	env->entries = 0;
	env->table = table;
	memset(env->table, '\0', env->size * sizeof(struct sdm_hash_entry *));
	return(env);
}

/**
 * Free resources allocated by the hash table.
 */
void destroy_sdm_hash(struct sdm_hash *env)
{
	unsigned int i;
	struct sdm_hash_entry *cur, *next;

	for (i = 0;i < env->size;i++) {
		cur = env->table[i];
		while (cur) {
			next = cur->next;
			if (env->destroy)
				env->destroy(cur->data);
			memory_free(cur);
			cur = next;
		}
	}
	memory_free(env->table);
	memory_free(env);
}


/**
 * Add an entry of "name" to the hash table.
 */
int sdm_hash_add(struct sdm_hash *env, const char *name, void *data)
{
	unsigned int hash;
	struct sdm_hash_entry *entry;

	if (!name || !data || (env->bitflags & SDM_HBF_NO_ADD))
		return(-1);
	/** Search for an existing entry */
	hash = sdm_hash_func(name) % env->size;
	for (entry = env->table[hash]; entry; entry = entry->next) {
		if (HASH_ENTRY_COMPARE(env, name, entry->name))
			return(-1);
	}

	if (!(entry = (struct sdm_hash_entry *) memory_alloc(sizeof(struct sdm_hash_entry) + strlen(name) + 1)))
		return(-1);
	entry->name = (char *) (entry + 1);
	strcpy(entry->name, name);
	entry->data = data;

	entry->next = env->table[hash];
	env->table[hash] = entry;
	env->entries++;
	if ((env->entries / env->size) > SDM_HASH_LOAD_FACTOR)
		sdm_entries_rehash(env, env->size * SDM_HASH_GROWTH_FACTOR);
	return(0);
}

/**
 * Replace the entry's data with "data".
 */
int sdm_hash_replace(struct sdm_hash *env, const char *name, void *data)
{
	unsigned int hash;
	struct sdm_hash_entry *cur;

	if (!name || !data || (env->bitflags & SDM_HBF_NO_REPLACE))
		return(-1);

	hash = sdm_hash_func(name) % env->size;
	for (cur = env->table[hash % env->size]; cur; cur = cur->next) {
		if (HASH_ENTRY_COMPARE(env, name, cur->name)) {
			if (env->destroy)
				env->destroy(cur->data);
			cur->data = data;
			return(0);
		}
	}
	return(-1);
}

/**
 * Remove an entry of "name" from hash table
 */
int sdm_hash_remove(struct sdm_hash *env, const char *name)
{
	unsigned int hash;
	struct sdm_hash_entry *cur, *prev;

	if (!name || (env->bitflags & SDM_HBF_NO_REMOVE))
		return(-1);
	hash = sdm_hash_func(name) % env->size;
	prev = NULL;
	cur = env->table[hash];
	while (cur) {
		if (HASH_ENTRY_COMPARE(env, name, cur->name)) {
			if (env->traverse_next == cur)
				sdm_hash_traverse_next(env);
			if (prev)
				prev = cur->next;
			else
				env->table[hash] = cur->next;
			if (env->destroy)
				env->destroy(cur->data);
			memory_free(cur);
			env->entries--;
			return(0);
		}
		prev = cur;
		cur = cur->next;
	}
	return(-1);
}

/**
 * Find the value bound to name in table.
 */
struct sdm_hash_entry *sdm_hash_find_entry(struct sdm_hash *env, const char *name)
{
	unsigned int hash;
	struct sdm_hash_entry *cur;

	hash = sdm_hash_func(name) % env->size;
	for (cur = env->table[hash % env->size]; cur; cur = cur->next) {
		if (HASH_ENTRY_COMPARE(env, name, cur->name))
			return(cur);
	}
	return(NULL);
}

void sdm_hash_traverse_reset(struct sdm_hash *env)
{
	for (env->traverse_index = 0; env->traverse_index < env->size; env->traverse_index++) {
		if ((env->traverse_next = env->table[env->traverse_index]))
			return;
	}
}

struct sdm_hash_entry *sdm_hash_traverse_next_entry(struct sdm_hash *env)
{
	struct sdm_hash_entry *entry;

	if ((env->traverse_index >= env->size) || !(entry = env->traverse_next))
		return(NULL);
	else if ((env->traverse_next = env->traverse_next->next))
		return(entry);

	while (++env->traverse_index < env->size) {
		if ((env->traverse_next = env->table[env->traverse_index]))
			return(entry);
	}
	return(entry);
}


/*** Local Functions ***/

/**
 * Increases the size of the hashtable and reinserts all of the elements.
 */
static inline int sdm_entries_rehash(struct sdm_hash *env, int newsize)
{
	unsigned int i, hash, oldsize;
	struct sdm_hash_entry **newtable;
	struct sdm_hash_entry *cur, *next;

	if (!(newtable = (struct sdm_hash_entry **) memory_alloc(newsize * sizeof(struct sdm_hash_entry *))))
		return(-1);
	memset(newtable, '\0', newsize * sizeof(struct sdm_hash_entry *));
	oldsize = env->size;
	env->size = newsize;
	for (i = 0;i < oldsize;i++) {
		cur = env->table[i];
		while (cur) {
			next = cur->next;
			hash = sdm_hash_func(cur->name) % env->size;
			cur->next = newtable[hash];
			newtable[hash] = cur;
			cur = next;
		}
	}
	memory_free(env->table);
	env->table = newtable;
	return(0);
}

static int sdm_stricmp(const char *str1, const char *str2)
{
	int i = 0;

	while ((str1[i] != '\0') && (str2[i] != '\0')) {
		if (LOWERCASE(str1[i]) != LOWERCASE(str2[i]))
			return(1);
		i++;
	}
	return(0);
}

static inline unsigned int sdm_hash_func(const char *str)
{
	int i;
	unsigned int hash = 0;

	for (i = 0;str[i] != '\0';i++)
		hash = LOWERCASE(str[i]) + (hash << 6) + (hash << 16) - hash;
	return(hash);
}

