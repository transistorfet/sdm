/*
 * Header Name:	array.h
 * Description:	Array Header
 */

#ifndef _SDM_ARRAY_H
#define _SDM_ARRAY_H

#include <pthread.h>

#include <sdm/data.h>
#include <sdm/globals.h>
#include <sdm/objs/object.h>
#include <sdm/objs/mutable.h>

#define MOO_ABF_DELETE			0x01	/// Delete elements when removed from list
#define MOO_ABF_DELETEALL		0x02	/// Delete all elements when list is destroyed
#define MOO_ABF_RESIZE			0x04	/// Allow resizing of the list when needed
#define MOO_ABF_REPLACE			0x08	/// Allow new elements to replace existing elements at the same index

#define MOO_ARRAY_DEFAULT_BITS		MOO_ABF_RESIZE
#define MOO_ARRAY_DEFAULT_SIZE		32
#define MOO_ARRAY_GROWTH_FACTOR		2
#define MOO_OBJECT_ARRAY_DEFAULT_BITS	MOO_ABF_DELETE | MOO_ABF_DELETEALL | MOO_ABF_RESIZE | MOO_ABF_REPLACE

class MooThing;

template<typename T>
class MooArray : public MooMutable {
    protected:
	//pthread_mutex_t m_lock;
	int m_bits;
	int m_size;
	int m_max;
	int m_last;
	int m_next_space;
	void (*m_destroy)(T);
	T *m_data;

    public:
	MooArray(int size = MOO_ARRAY_DEFAULT_SIZE, int max = -1, int bits = MOO_ARRAY_DEFAULT_BITS, void (*destroy)(T) = MooArray<T>::destroy);
	virtual ~MooArray();
	static void destroy(T value) { delete value; }

	void clear();
	T operator[] (int index);
	T get(int index);
	T set(int index, T value);
	int add(T value);
	int remove(T value);
	int search(T value);

	int push(T value);
	T pop();
	T shift();
	int unshift(T value);
	int insert(int index, T value);
	T splice(int index);
	// TODO perhaps add a function which does splice() but also deletes the value appropriately without returning it

	T get_first() { if (m_last < 0) return(NULL); return(m_data[0]); }
	T get_last() { if (m_last < 0) return(NULL); return(m_data[m_last]); }

	int resize(int size);

    public:
	/// Accessors
	int size() { return(m_size); }
	int last() { return(m_last); }
	int next_space() { return(m_next_space); }
};

class MooObjectArray : public MooArray<MooObject *> {
    public:
	MooObjectArray(int size = MOO_ARRAY_DEFAULT_SIZE, int max = -1, int bits = MOO_OBJECT_ARRAY_DEFAULT_BITS);

	int read_entry(const char *type, MooDataFile *data);
	int write_object(MooDataFile *data);
	int to_string(char *buffer, int max);

	long int get_integer(int index);
	double get_float(int index);
	const char *get_string(int index);
	MooThing *get_thing(int index);

    private:
	virtual MooObject *access_property(const char *name, MooObject *value = NULL);
	virtual MooObject *access_method(const char *name, MooObject *value = NULL);
};

extern MooObjectType moo_array_obj_type;
MooObject *load_moo_array(MooDataFile *data);

int init_array(void);
void release_array(void);

template<typename T>
MooArray<T>::MooArray(int size, int max, int bits, void (*destroy)(T))
{
	//pthread_mutex_init(&m_lock, NULL);
	m_bits = bits;
	m_size = 0;
	m_max = max;
	m_last = -1;
	m_next_space = 0;
	m_destroy = destroy;
	m_data = NULL;
	this->resize(size);
}

template<typename T>
MooArray<T>::~MooArray()
{
	int i;

	// TODO should we mutext lock here
	if (m_bits & MOO_ABF_DELETEALL && m_destroy) {
		for (i = 0; i < m_size; i++) {
			if (m_data[i])
				m_destroy(m_data[i]);
		}
	}
	//pthread_mutex_destroy(&m_lock);
	free(m_data);
}

template<typename T>
void MooArray<T>::clear()
{
	int i;

	//pthread_mutex_lock(&m_lock);
	if (m_bits & MOO_ABF_DELETE && m_destroy) {
		for (i = 0; i < m_size; i++) {
			if (m_data[i])
				m_destroy(m_data[i]);
		}
	}
	m_last = -1;
	m_next_space = 0;
	//pthread_mutex_unlock(&m_lock);
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
	if (index >= m_size) {
		// TODO you probably need to mutex lock but it would be better if you move this to resize or something
		if ((m_bits & MOO_ABF_RESIZE) && (m_max == -1 || index < m_max))
			this->resize((int) (index * 1.25));
		else
			return(NULL);
	}

	if (m_data[index]) {
		if (value && !(m_bits & MOO_ABF_REPLACE))
			return(NULL);
		if (m_bits & MOO_ABF_DELETE && m_destroy)
			m_destroy(m_data[index]);
	}

	//pthread_mutex_lock(&m_lock);
	m_data[index] = value;
	/// Update the next_space and last values accordingly
	if (!value) {
		if (index < m_next_space)
			m_next_space = index;
		if (index == m_last) {
			for (; m_last >= 0; m_last--) {
				if (m_data[m_last])
					break;
			}
		}
	}
	else {
		if (index > m_last)
			m_last = index;
		for (; m_next_space < m_size; m_next_space++) {
			if (!m_data[m_next_space])
				break;
		}
	}
	//pthread_mutex_unlock(&m_lock);
	return(value);
}

template<typename T>
int MooArray<T>::add(T value)
{
	int space = m_next_space;

	// TODO is this resize correct?  Should it be limited to a max increase after a certain size?
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
			this->set(i, NULL);
			return(1);
		}
	}
	return(0);
}

template<typename T>
int MooArray<T>::search(T value)
{
	int i;

	for (i = 0; i <= m_last; i++) {
		if (m_data[i] == value)
			return(i);
	}
	return(-1);
}

template<typename T>
int MooArray<T>::push(T value)
{
	return(this->insert(m_last + 1, value));
}

template<typename T>
T MooArray<T>::pop()
{
	return(this->splice(m_last));
}

template<typename T>
T MooArray<T>::shift()
{
	return(this->splice(0));
}

template<typename T>
int MooArray<T>::unshift(T value)
{
	return(this->insert(0, value));
}

template<typename T>
int MooArray<T>::insert(int index, T value)
{
	if (index < 0 || index > m_last + 1)
		return(-1);
	/// Resize the array if required
	if ((m_last + 1 >= m_size) && this->resize((int) (m_size * MOO_ARRAY_GROWTH_FACTOR)))
		return(-1);
	for (int i = m_last; i >= index; i--)
		m_data[i + 1] = m_data[i];
	m_data[index] = value;
	m_last++;
	for (; m_next_space < m_size; m_next_space++) {
		if (!m_data[m_next_space])
			break;
	}
	return(0);
}

template<typename T>
T MooArray<T>::splice(int index)
{
	T ret;

	if (index < 0 || index > m_last)
		return(NULL);
	ret = m_data[index];
	for (int i = index; i < m_last; i++)
		m_data[i] = m_data[i + 1];
	m_data[m_last] = NULL;
	if (m_last < m_next_space)
		m_next_space = m_last;
	m_last--;
	return(ret);
}


template<typename T>
int MooArray<T>::resize(int size)
{
	T *newdata;

	if (m_max > 0 && size > m_max)
		size = m_max;
	if (size == m_size)
		return(-1);

	if (!(newdata = (T *) realloc(m_data, size * sizeof(T))))
		throw moo_mem_error;
	m_data = newdata;
	memset(&m_data[m_size], '\0', (size - m_size) * sizeof(T));
	m_next_space = m_size;
	m_size = size;
	return(0);
}

#endif

