/*
 * Header Name:	array.h
 * Description:	Array Header
 */

#ifndef _SDM_ARRAY_H
#define _SDM_ARRAY_H

#define MOO_ABF_DELETE			0x01	// Delete elements when removed from list
#define MOO_ABF_DELETEALL		0x02	// Delete all elements when list is destroyed
#define MOO_ABF_RESIZE			0x04	// Allow resizing of the list when needed
#define MOO_ABF_REPLACE			0x08	// Allow new elements to replace existing elements at the same index

#define MOO_ARRAY_DEFAULT_BITS		MOO_ABF_DELETEALL | MOO_ABF_RESIZE
#define MOO_ARRAY_DEFAULT_SIZE		32
#define MOO_ARRAY_GROWTH_FACTOR		1.75

template<typename T>
class MooArray {
	int m_bits;
	int m_size;
	int m_max;
	int m_next_space;
	T **m_data;

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

#endif

