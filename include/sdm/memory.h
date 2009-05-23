/*
 * Header Name:	memory.h
 * Description:	Memory Allocator and Tracker Header
 */

#ifndef _SDM_MEMORY_H
#define _SDM_MEMORY_H

#include <stdlib.h>

#define memory_alloc(size)		( malloc(size) )
#define memory_realloc(ptr, size)	( realloc(ptr, size) )
#define memory_free(ptr)		( free(ptr) )

/*
void *memory_alloc(unsigned int);
void *memory_realloc(void *, unsigned int);
void memory_free(void *);
*/

#endif

