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
	this->size = size;
	this->max = max;
	this->next_space = 0;
	this->data = NULL;
	this->resize(size);
}

template<typename T>
T MooArray<T>::operator[](int index)
{

}

template<typename T>
T MooArray<T>::get(int index)
{

}

template<typename T>
T MooArray<T>::set(int index, T value)
{

}


template<typename T>
void MooArray<T>::resize(int size)
{

}

