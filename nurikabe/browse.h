#ifndef BROWSE_H

#include <rlpw.h>
#include "entry.h"

typedef struct Browse {
    pthread_mutex_t mtx;
    Entry *active;
    size_t index;
    size_t offset;
} Browse;

#define BROWSE_H
#endif // BROWSE_H

