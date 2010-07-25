/*
 * Header Name:	array.h
 * Description:	Array Header
 */

#ifndef _SDM_ARRAY_H
#define _SDM_ARRAY_H

#define MOO_ARRAY_DEFAULT_SIZE		32

template<typename T>
class MooArray {
	int size;
	int max;
	int next_space;
	T **data;

    public:
	MooArray(int size = MOO_ARRAY_DEFAULT_SIZE, int max = -1);
	~MooArray();

	T operator[] (int index);
	T get(int index);
	T set(int index, T value);

	void resize(int size);
};

#endif

