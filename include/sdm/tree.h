/*
 * Name:	tree.h
 * Description:	Binary Tree Header
 */


#ifndef _SDM_TREE_H
#define _SDM_TREE_H

#include <sdm/globals.h>

#define MOO_TBF_NO_ADD			0x0001		// No entries can be added
#define MOO_TBF_REPLACE			0x0002		// Allow new elements to replace existing elements at the same index
#define MOO_TBF_REMOVE			0x0004		// Allow entries to be removed
#define MOO_TBF_DELETE			0x0010		// Delete elements when removed from list
#define MOO_TBF_DELETEALL		0x0020		// Delete all elements when list is destroyed

template<typename T>
class MooTreeEntry {
    public:
	char *m_key;
	T m_data;
	MooTreeEntry<T> *parent;
	MooTreeEntry<T> *left;
	MooTreeEntry<T> *right;
};

template<typename T>
class MooTree {
    protected:
	int m_bitflags;
	MooTreeEntry<T> *m_traverse_next;
	MooTreeEntry<T> *m_root;
    public:
	MooTree(int bits);
	~MooTree();

	int set(const char *key, T data);
	int remove(const char *key);

	MooTreeEntry<T> *get_entry(const char *key);
	MooTreeEntry<T> *get_entry_partial(const char *key);
	T get(const char *key);
	T get_partial(const char *key);

	void reset();
	MooTreeEntry<T> *next_entry();
	T next();
};

#endif

