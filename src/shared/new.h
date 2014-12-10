#ifndef RR_NEW_H
#define RR_NEW_H

#include <stddef.h>
inline void *operator new(size_t, void *p) { return p; }
inline void *operator new[](size_t, void *p) { return p; }
inline void operator delete(void *, void *) {}
inline void operator delete[](void *, void *) {}

#endif //RR_NEW_H
