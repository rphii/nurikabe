#ifndef ENTRY_H

#include <rlpw.h>
#include <rlso.h>
#include <sys/stat.h>
#include <rltui.h>

typedef struct Entry {
    So path;
    char *cpath;
    struct Entry *parent; // parent, duh
    struct Entry **list; // children
    size_t size_self; // only self
    size_t size_total; // self + all children
    pthread_mutex_t mtx;
    struct stat stats;
    int error;
} Entry;

//void entry_dispatch(Entry *
void entry_register_dispatch(Pw *pw, Tui_Sync_Main *sync_m, Entry *parent, Entry *entry, So path);

#define ENTRY_H
#endif // ENTRY_H

