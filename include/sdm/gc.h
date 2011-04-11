/*
 * Module Name:	gc.h
 * Description:	Garbage Collector
 */

#ifndef _SDM_GC_H
#define _SDM_GC_H

#include <sdm/exception.h>

#ifdef USE_LIBGC

#define MOO_INCREF(ptr)		( ptr )
#define MOO_DECREF(ptr)		(  )

typedef gc MooGC;

#else

#define MOO_INCREF(ptr)		( (class MooObject *) MooGC::incref(ptr) )
#define MOO_DECREF(ptr)		( MooGC::decref(ptr) )

#define GC_NOFREE		32767

class MooGC {
    public:
	int m_refs;

	MooGC() {
		m_refs = 1;
	}

	virtual ~MooGC() {
		if (m_refs)
			moo_status("MEM: Premature free attempt at 0x%x", this);
	}

	inline void *operator new(size_t size) {
		//void *ptr;
		//ptr = malloc(size);
		//moo_status("MEM: alloc 0x%x", ptr);
		//return(ptr);
		return(malloc(size));
	}
	inline void *operator new(size_t size, void *ptr) {
		// TODO wtf??
		return(ptr);
	}
	inline void operator delete(void *ptr) {
		//moo_status("MEM: free 0x%x", ptr);
		// TODO temporarily disable freeing
		//free(ptr);
	}

	static inline MooGC *incref(MooGC *obj) {
		if (obj && obj->m_refs != GC_NOFREE)
			obj->m_refs++;
		return(obj);
	}
	static inline void decref(MooGC *obj) {
		if (obj && obj->m_refs != GC_NOFREE && --obj->m_refs <= 0)
			delete(obj);
	}

	inline void set_nofree() {
		m_refs = GC_NOFREE;
	}
};

#endif


#endif

