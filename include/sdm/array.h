/*
 * Header Name:	array.h
 * Description:	Array Header
 */

#ifndef _SDM_ARRAY_H
#define _SDM_ARRAY_H

#define MOO_ARRAY_DEFAULT_SIZE		32
#define MOO_ARRAY_GROWTH_FACTOR		1.75

template<typename T>
class MooArray {
	int m_size;
	int m_max;
	int m_next_space;
	T **m_data;

    public:
	MooArray(int size = MOO_ARRAY_DEFAULT_SIZE, int max = -1);
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

#endif

