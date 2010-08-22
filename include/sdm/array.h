/*
 * Header Name:	array.h
 * Description:	Array Header
 */

#ifndef _SDM_ARRAY_H
#define _SDM_ARRAY_H

#include <sdm/memory.h>
#include <sdm/globals.h>

#define MOO_ABF_DELETE			0x01	/// Delete elements when removed from list
#define MOO_ABF_DELETEALL		0x02	/// Delete all elements when list is destroyed
#define MOO_ABF_RESIZE			0x04	/// Allow resizing of the list when needed
#define MOO_ABF_REPLACE			0x08	/// Allow new elements to replace existing elements at the same index

#define MOO_ARRAY_DEFAULT_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE
#define MOO_ARRAY_DEFAULT_SIZE		32
#define MOO_ARRAY_GROWTH_FACTOR		1.75

template<typename T>
class MooArray {
	int m_bits;
	int m_size;
	int m_max;
	int m_next_space;
	T *m_data;

    public:
	MooArray(int size = MOO_ARRAY_DEFAULT_SIZE, int max = -1, int bits = MOO_ARRAY_DEFAULT_BITS);
	~MooArray();
	int size() { return(m_size); }
	int next_space() { return(m_next_space); }

	T operator[] (int index);
	T get(int index);
	T set(int index, T value);
	int add(T value);
	int remove(T value);

	void resize(int size);
};


template<typename T>
MooArray<T>::MooArray(int size, int max, int bits)
{
	m_bits = bits;
	m_size = 0;
	m_max = max;
	m_next_space = 0;
	m_data = NULL;
	this->resize(size);
}

template<typename T>
MooArray<T>::~MooArray()
{
	int i;

	if (m_bits & MOO_ABF_DELETEALL) {
		for (i = 0; i < m_size; i++) {
			if (m_data[i])
				delete m_data[i];
		}
	}
	memory_free(m_data);
}

template<typename T>
T MooArray<T>::operator[](int index)
{
	return(this->get(index));
}

template<typename T>
T MooArray<T>::get(int index)
{
	if (index < 0 || index > m_size)
		return(NULL);
	return(m_data[index]);
}

template<typename T>
T MooArray<T>::set(int index, T value)
{
	if (index < 0)
		return(NULL);
	if (index > m_size) {
		if ((m_bits & MOO_ABF_RESIZE) && (m_max == -1 || index < m_max))
			this->resize(index + 1);
		else
			return(NULL);
	}

	if (m_data[index]) {
		if (value && !(m_bits & MOO_ABF_REPLACE))
			return(NULL);
		if (m_bits & MOO_ABF_DELETE)
			delete m_data[index];
	}
	m_data[index] = value;

	/// Update the next_space value accordingly
	if (!value) {
		if (index < m_next_space)
			m_next_space = index;
	}
	else {
		for (; m_next_space < m_size; m_next_space++) {
			if (!m_data[m_next_space])
				break;
		}
	}
	return(value);
}

template<typename T>
int MooArray<T>::add(T value)
{
	int space = m_next_space;

	if (space >= m_size && m_bits & MOO_ABF_RESIZE)
		this->resize((int) (m_size * MOO_ARRAY_GROWTH_FACTOR));
	if (this->set(space, value))
		return(space);
	return(-1);
}

template<typename T>
int MooArray<T>::remove(T value)
{
	int i;

	for (i = 0; i < m_size; i++) {
		if (m_data[i] == value) {
			if (m_bits & MOO_ABF_DELETE)
				delete m_data[i];
			m_data[i] = NULL;
			if (i < m_next_space)
				m_next_space = i;
			return(1);
		}
	}
	return(0);
}

template<typename T>
void MooArray<T>::resize(int size)
{
	T *newdata;

	if (m_max > 0 && size > m_max)
		size = m_max;
	if (size == m_size)
		throw -1;

	if (!(newdata = (T *) memory_realloc(m_data, size * sizeof(T))))
		throw -1;
	m_data = newdata;
	memset(&m_data[m_size], '\0', (size - m_size) * sizeof(T));
	m_next_space = m_size;
	m_size = size;
}

#endif

