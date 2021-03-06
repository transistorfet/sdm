/*
 * Name:	hash.h
 * Description:	Hash Table Header
 */


#ifndef _SDM_HASH_H
#define _SDM_HASH_H

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/mutable.h>

#define MOO_HBF_NO_ADD			0x0001		/// No entries can be added
#define MOO_HBF_REPLACE			0x0002		/// Allow new elements to replace existing elements at the same index
#define MOO_HBF_REMOVE			0x0004		/// Allow entries to be removed
#define MOO_HBF_DELETE			0x0010		/// Delete elements when removed from list
#define MOO_HBF_DELETEALL		0x0020		/// Delete all elements when list is destroyed

#define MOO_HASH_DEFAULT_SIZE		16
#define MOO_HASH_DEFAULT_BITS		MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETEALL
#define MOO_HASH_LOAD_FACTOR		0.75
#define MOO_HASH_GROWTH_FACTOR		1.75
#define MOO_OBJECT_HASH_DEFAULT_BITS	MOO_HBF_REPLACE | MOO_HBF_REMOVE | MOO_HBF_DELETE | MOO_HBF_DELETEALL

class MooThing;

template<typename T>
class MooHashEntry {
    public:
	moo_id_t m_owner;
	moo_mode_t m_mode;
	char *m_key;
	T m_data;
	MooHashEntry<T> *m_next;
};

template<typename T>
class MooHash : public MooMutable {
    protected:
	int m_bits;
	int m_traverse_index;
	MooHashEntry<T> *m_traverse_next;
	int m_size;
	int m_entries;
	void (*m_destroy)(T);
	MooHashEntry<T> **m_table;

	int rehash(int newsize);
    public:
	MooHash(int size = MOO_HASH_DEFAULT_SIZE, int bits = MOO_HASH_DEFAULT_BITS, void (*destroy)(T) = MooHash::destroy);
	~MooHash();
	static void destroy(T value) { delete value; }

	void clear();
	int set(const char *key, T data, moo_id_t owner = -2, moo_mode_t mode = -1);
	int remove(const char *key);

	MooHashEntry<T> *get_entry(const char *key);
	T get(const char *key);
	const char *key(T value);

	void reset();
	MooHashEntry<T> *next_entry();
	T next();

	int entries() { return(m_entries); }
};

class MooObjectHash : public MooHash<MooObject *> {
	MooObjectHash *m_parent;

    public:
	MooObjectHash(MooObjectHash *parent = NULL, int size = MOO_HASH_DEFAULT_SIZE, int bits = MOO_OBJECT_HASH_DEFAULT_BITS);

	int read_entry(const char *type, MooDataFile *data);
	int write_object(MooDataFile *data);
	int to_string(char *buffer, int max);

	int mutate(const char *key, MooObject *obj);

	MooObject *get(const char *key);
	MooObject *get_local(const char *key);
	long int get_integer(const char *key);
	double get_float(const char *key);
	const char *get_string(const char *key);
	MooThing *get_thing(const char *key);

    private:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);

	friend class HashEventForeach;
};

extern MooObjectType moo_hash_obj_type;
MooObject *load_moo_hash(MooDataFile *data);

int init_hash(void);
void release_hash(void);

#define LOWERCASE(ch) \
	( (ch >= 0x41 && ch <= 0x5a) ? ch + 0x20 : ch )

static inline unsigned int moo_hash_func(const char *);

template<typename T>
MooHash<T>::MooHash(int size, int bits, void (*destroy)(T))
{
	if (size <= 0)
		size = MOO_HASH_DEFAULT_SIZE;
	m_bits = bits;
	m_traverse_index = 0;
	m_traverse_next = NULL;
	m_size = size;
	m_entries = 0;
	m_destroy = destroy;
	if (!(m_table = (MooHashEntry<T> **) malloc(m_size * sizeof(MooHashEntry<T> *))))
		throw moo_mem_error;
	memset(m_table, '\0', m_size * sizeof(MooHashEntry<T> *));
}

template<typename T>
MooHash<T>::~MooHash()
{
	this->clear();
	free(m_table);
}

template<typename T>
void MooHash<T>::clear()
{
	int i;
	MooHashEntry<T> *cur, *next;

	for (i = 0;i < m_size;i++) {
		cur = m_table[i];
		while (cur) {
			next = cur->m_next;
			if (m_bits & MOO_HBF_DELETEALL && m_destroy)
				m_destroy(cur->m_data);
			delete cur;
			cur = next;
		}
		m_table[i] = NULL;
	}
}

template<typename T>
int MooHash<T>::set(const char *key, T data, moo_id_t owner, moo_mode_t mode)
{
	unsigned int hash;
	MooHashEntry<T> *entry;

	if (!key || !data || (m_bits & MOO_HBF_NO_ADD))
		return(-1);
	/// Search for an existing entry
	hash = moo_hash_func(key) % m_size;
	for (entry = m_table[hash]; entry; entry = entry->m_next) {
		if (!strcasecmp(key, entry->m_key)) {
			if (!(m_bits & MOO_HBF_REPLACE))
				return(-1);
			if (m_bits & MOO_HBF_DELETE && m_destroy)
				m_destroy(entry->m_data);
			if (owner > -2)
				entry->m_owner = owner;
			if (mode > -1)
				entry->m_mode = mode;
			entry->m_data = data;
			return(0);
		}
	}

	if (!(entry =  (MooHashEntry<T> *) malloc(sizeof(MooHashEntry<T>) + strlen(key) + 1)))
		return(-1);
	entry->m_key = (char *) (entry + 1);
	strcpy(entry->m_key, key);
	entry->m_data = data;
	entry->m_owner = (owner > -2) ? owner : -1;
	entry->m_mode = (mode > -1) ? mode : MOO_DEFAULT_MODE;

	entry->m_next = m_table[hash];
	m_table[hash] = entry;
	m_entries++;
	if ((m_entries / m_size) > MOO_HASH_LOAD_FACTOR)
		this->rehash((int) (m_size * MOO_HASH_GROWTH_FACTOR));
	return(0);
}

template<typename T>
int MooHash<T>::remove(const char *key)
{
	unsigned int hash;
	MooHashEntry<T> *cur, *prev;

	if (!key || !(m_bits & MOO_HBF_REMOVE))
		return(-1);
	hash = moo_hash_func(key) % m_size;
	prev = NULL;
	cur = m_table[hash];
	while (cur) {
		if (!strcasecmp(key, cur->m_key)) {
			// TODO this could cause a bug where an entry is skipped
			if (m_traverse_next == cur)
				this->next_entry();
			if (prev)
				prev = cur->m_next;
			else
				m_table[hash] = cur->m_next;
			if (m_bits & MOO_HBF_DELETE && m_destroy)
				m_destroy(cur->m_data);
			free(cur);
			m_entries--;
			return(0);
		}
		prev = cur;
		cur = cur->m_next;
	}
	return(-1);
}

template<typename T>
MooHashEntry<T> *MooHash<T>::get_entry(const char *key)
{
	unsigned int hash;
	MooHashEntry<T> *cur;

	hash = moo_hash_func(key) % m_size;
	for (cur = m_table[hash % m_size]; cur; cur = cur->m_next) {
		if (!strcasecmp(key, cur->m_key))
			return(cur);
	}
	return(NULL);
}

template<typename T>
T MooHash<T>::get(const char *key)
{
	MooHashEntry<T> *entry;

	if (!(entry = this->get_entry(key)))
		return(NULL);
	return(entry->m_data);
}

template<typename T>
const char *MooHash<T>::key(T value)
{
	MooHashEntry<T> *cur;

	for (int i = 0; i < m_size; i++) {
		for (cur = m_table[i]; cur; cur = cur->m_next) {
			if (cur->m_data == value)
				return(cur->m_key);
		}
	}
	return(NULL);
}

template<typename T>
void MooHash<T>::reset()
{
	for (m_traverse_index = 0; m_traverse_index < m_size; m_traverse_index++) {
		if ((m_traverse_next = m_table[m_traverse_index]))
			return;
	}
}

template<typename T>
MooHashEntry<T> *MooHash<T>::next_entry()
{
	MooHashEntry<T> *entry;

	if ((m_traverse_index >= m_size) || !(entry = m_traverse_next))
		return(NULL);
	else if ((m_traverse_next = m_traverse_next->m_next))
		return(entry);

	while (++m_traverse_index < m_size) {
		if ((m_traverse_next = m_table[m_traverse_index]))
			return(entry);
	}
	return(entry);
}

template<typename T>
T MooHash<T>::next()
{
	MooHashEntry<T> *entry;

	entry = this->next_entry();
	if (!entry)
		return(NULL);
	return(entry->m_data);
}

template<typename T>
int MooHash<T>::rehash(int newsize)
{
	unsigned int i, hash, oldsize;
	MooHashEntry<T> **newtable;
	MooHashEntry<T> *cur, *next;

	if (!(newtable = (MooHashEntry<T> **) malloc(newsize * sizeof(MooHashEntry<T> *))))
		return(-1);
	memset(newtable, '\0', newsize * sizeof(MooHashEntry<T> *));
	oldsize = m_size;
	m_size = newsize;
	for (i = 0;i < oldsize;i++) {
		cur = m_table[i];
		while (cur) {
			next = cur->m_next;
			hash = moo_hash_func(cur->m_key) % m_size;
			cur->m_next = newtable[hash];
			newtable[hash] = cur;
			cur = next;
		}
	}
	free(m_table);
	m_table = newtable;
	return(0);
}

static inline unsigned int moo_hash_func(const char *str)
{
	int i;
	unsigned int hash = 0;

	for (i = 0; str[i] != '\0'; i++)
		hash = LOWERCASE(str[i]) + (hash << 6) + (hash << 16) - hash;
	return(hash);
}

#endif

