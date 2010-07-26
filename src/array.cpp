/*
 * Name:	array.c
 * Description:	Array
 */

#include <stdio.h>
#include <stdarg.h>

#include <sdm/memory.h>
#include <sdm/globals.h>

#include <sdm/array.h>

template<typename T>
MooArray<T>::MooArray(int size, int max)
{
	m_size = size;
	m_max = max;
	m_next_space = 0;
	m_data = NULL;
	this->resize(size);
}

template<typename T>
MooArray<T>::~MooArray()
{
	int i;

	for (i = 0; i < m_size; i++) {
		if (m_data[i])
			delete m_data[i];
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
	if (index > m_size)
		return(NULL);
	return(m_data[index]);
}

template<typename T>
T MooArray<T>::set(int index, T value)
{
	if (index > m_size)
		return(NULL);
	/** For now, if there is already something in that position, do not replace or delet it */
	if (m_data[index])
		return(NULL);
	m_data[index] = value;

	/** Update the next_space value accordingly */
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

	if (space >= m_size)
		this->resize(m_size * MOO_ARRAY_GROWTH_FACTOR);
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
	T **newdata;

	if (m_max > 0 && size > m_max)
		size = m_max;
	if (size == m_size)
		throw -1;

	if (!(newdata = (T **) memory_realloc(m_data, size * sizeof(T *))))
		throw -1;
	m_data = newdata;
	memset(&m_data[m_size], '\0', (size - m_size) * sizeof(T *));
	m_next_space = m_size;
	m_size = size;
}

