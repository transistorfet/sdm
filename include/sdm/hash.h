/*
 * Name:	hash.h
 * Description:	Hash Table Header
 */


#ifndef _SDM_HASH_H
#define _SDM_HASH_H

#include <sdm/globals.h>

#define MOO_HBF_NO_ADD			0x0001		// No entries can be added
#define MOO_HBF_REPLACE			0x0002		// Allow new elements to replace existing elements at the same index
#define MOO_HBF_REMOVE			0x0004		// Allow entries to be removed
#define MOO_HBF_DELETE			0x0010		// Delete elements when removed from list
#define MOO_HBF_DELETEALL		0x0020		// Delete all elements when list is destroyed

#define MOO_HASH_DEFAULT_SIZE		32
#define MOO_HASH_DEFAULT_BITS		MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL

template<typename T>
class MooHashEntry {
    public:
	char *m_key;
	T m_data;
	MooHashEntry<T> *m_next;
};

template<typename T>
class MooHash {
    protected:
	int m_bits;
	int m_traverse_index;
	MooHashEntry<T> *m_traverse_next;
	int m_size;
	int m_entries;
	MooHashEntry<T> **m_table;

	int rehash(int newsize);
    public:
	MooHash(int size = MOO_HASH_DEFAULT_SIZE, int bits = MOO_HASH_DEFAULT_BITS);
	~MooHash();

	int set(const char *key, T data);
	int remove(const char *key);

	MooHashEntry<T> *get_entry(const char *key);
	T get(const char *key);

	void reset();
	MooHashEntry<T> *next_entry();
	T next();
};


#endif

