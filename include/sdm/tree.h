/*
 * Name:	tree.h
 * Description:	Binary Tree Header
 */


#ifndef _SDM_TREE_H
#define _SDM_TREE_H

#include <sdm/globals.h>

#define MOO_TBF_NO_ADD			0x0001		/// No entries can be added
#define MOO_TBF_REPLACE			0x0002		/// Allow new elements to replace existing elements at the same index
#define MOO_TBF_REMOVE			0x0004		/// Allow entries to be removed
#define MOO_TBF_DELETE			0x0010		/// Delete elements when removed from list
#define MOO_TBF_DELETEALL		0x0020		/// Delete all elements when list is destroyed

#define MOO_TREE_DEFAULT_BITS		MOO_TBF_REPLACE | MOO_TBF_REMOVE | MOO_TBF_DELETE | MOO_TBF_DELETEALL

template<typename T>
class MooTreeEntry {
    public:
	char *m_key;
	T m_data;
	MooTreeEntry<T> *m_parent;
	MooTreeEntry<T> *m_left;
	MooTreeEntry<T> *m_right;
};

template<typename T>
class MooTree {
    protected:
	int m_bits;
	MooTreeEntry<T> *m_traverse_next;
	MooTreeEntry<T> *m_root;

	void destroy_subtree(MooTreeEntry<T> *entry);
    public:
	MooTree(int bits = MOO_TREE_DEFAULT_BITS);
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


template<typename T>
MooTree<T>::MooTree(int bits)
{
	m_bits = bits;
	m_traverse_next = NULL;
	m_root = NULL;
}

template<typename T>
MooTree<T>::~MooTree()
{
	if (m_root)
		this->destroy_subtree(m_root);
}


template<typename T>
int MooTree<T>::set(const char *key, T data)
{
	int res;
	MooTreeEntry<T> *prev, *entry;

	if (!key || !data || (m_bits & MOO_TBF_NO_ADD))
		return(-1);
	/// Search for an existing entry
	for (entry = m_root, prev = NULL; entry; ) {
		if (!(res = strcmp(key, entry->m_key))) {
			if (!(m_bits & MOO_TBF_REPLACE))
				return(-1);
			if (m_bits & MOO_TBF_DELETE)
				delete entry->m_data;
			entry->m_data = data;
			return(0);
		}
		prev = entry;
		entry = (res < 0) ? entry->m_left : entry->m_right;
	}

	if (!(entry = (MooTreeEntry<T> *) memory_alloc(sizeof(MooTreeEntry<T>) + strlen(key) + 1)))
		return(-1);
	entry->m_key = (char *) (entry + 1);
	strcpy(entry->m_key, key);
	entry->m_data = data;

	entry->m_left = NULL;
	entry->m_right = NULL;
	entry->m_parent = prev;
	if (!prev)
		m_root = entry;
	else if (strcmp(key, prev->m_key) < 0)
		prev->m_left = entry;
	else
		prev->m_right = entry;
	return(0);
}

template<typename T>
int MooTree<T>::remove(const char *key)
{
	int res;
	MooTreeEntry<T> *cur, *tmp;

	if (!key || (m_bits & MOO_TBF_REMOVE))
		return(-1);
	for (cur = m_root; cur; ) {
		if (!(res = strcmp(key, cur->m_key))) {
			if (!cur->m_left && !cur->m_right) {
				if (!cur->m_parent)
					m_root = NULL;
				else if (cur->m_parent->m_left == cur)
					cur->m_parent->m_left = NULL;
				else
					cur->m_parent->m_right = NULL;
			}
			else if (cur->m_left && cur->m_right) {
				tmp = cur->m_left;
				while (tmp->m_right)
					tmp = tmp->m_right;
				if (cur->m_left == tmp) {
					cur->m_left = tmp->m_left;
					if (tmp->m_left)
						tmp->m_left->m_parent = cur;
				}
				else {
					tmp->m_parent->m_right = tmp->m_left;
					if (tmp->m_left)
						tmp->m_left->m_parent = tmp->m_parent;
				}
				tmp->m_left = cur->m_left;
				tmp->m_right = cur->m_right;
				tmp->m_parent = cur->m_parent;
				if (cur == m_root)
					m_root = tmp;
			}
			else {
				tmp = cur->m_left ? cur->m_left : cur->m_right;
				if (!cur->m_parent)
					m_root = tmp;
				else if (cur->m_parent->m_left == cur)
					cur->m_parent->m_left = tmp;
				else
					cur->m_parent->m_right = tmp;
				tmp->m_parent = cur->m_parent;
			}
			// TODO this is incorrect but the correct way depends on how you traverse
			if (m_traverse_next == cur)
				m_traverse_next = NULL;
			if (m_bits & MOO_TBF_DELETE)
				delete cur->m_data;
			memory_free(cur);
			return(0);
		}
		cur = (res < 0) ? cur->m_left : cur->m_right;
	}
	return(-1);
}

template<typename T>
MooTreeEntry<T> *MooTree<T>::get_entry(const char *key)
{
	int res;
	MooTreeEntry<T> *cur;

	for (cur = m_root; cur; ) {
		if (!(res = strcmp(key, cur->m_key)))
			return(cur);
		cur = (res < 0) ? cur->m_left : cur->m_right;
	}
	return(NULL);
}

template<typename T>
MooTreeEntry<T> *MooTree<T>::get_entry_partial(const char *key)
{
	int res, len;
	MooTreeEntry<T> *cur;

	len = strlen(key);
	for (cur = m_root; cur; ) {
		if (!(res = strncmp(key, cur->m_key, len)))
			return(cur);
		cur = (res < 0) ? cur->m_left : cur->m_right;
	}
	return(NULL);
}

template<typename T>
T MooTree<T>::get(const char *key)
{
	MooTreeEntry<T> *entry;

	entry = this->get_entry(key);
	if (!entry)
		return(NULL);
	return(entry->m_data);
}

template<typename T>
T MooTree<T>::get_partial(const char *key)
{
	MooTreeEntry<T> *entry;

	entry = this->get_entry_partial(key);
	if (!entry)
		return(NULL);
	return(entry->m_data);
}

template<typename T>
void MooTree<T>::reset()
{
	m_traverse_next = m_root;
}

template<typename T>
MooTreeEntry<T> *MooTree<T>::next_entry()
{
	MooTreeEntry<T> *entry, *cur, *prev;

	if (!m_traverse_next)
		return(NULL);
	entry = m_traverse_next;
	if (m_traverse_next->m_left)
		m_traverse_next = m_traverse_next->m_left;
	else if (m_traverse_next->m_right)
		m_traverse_next = m_traverse_next->m_right;
	else {
		for (prev = m_traverse_next, cur = m_traverse_next->m_parent; cur; prev = cur, cur = cur->m_parent) {
			if ((cur->m_left == prev) && cur->m_right) {
				m_traverse_next = cur->m_right;
				return(entry);
			}
		}
		m_traverse_next = NULL;
	}
	return(entry);
}

template<typename T>
T MooTree<T>::next()
{
	MooTreeEntry<T> *entry;

	entry = this->next_entry();
	if (!entry)
		return(NULL);
	return(entry->m_data);
}

template<typename T>
void MooTree<T>::destroy_subtree(MooTreeEntry<T> *entry)
{
	if (entry->m_left)
		this->destroy_subtree(entry->m_left);
	if (entry->m_right)
		this->destroy_subtree(entry->m_right);
	if (m_bits & MOO_TBF_DELETEALL)
		delete entry->m_data;
	memory_free(entry);
}

#endif

