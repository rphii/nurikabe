#include <dirent.h>
#include "entry.h"

typedef struct Entry_Register_Task {
    Pw *pw;
    Entry *parent;
    Entry *entry;
    Tui_Sync_Main *sync_m;
    So path;
} Entry_Register_Task;

ErrDecl so_file_exec(So file_or_dir, bool hidden, bool recursive, So_File_Exec_Callback cb_file, So_File_Exec_Callback cb_dir, void *user);

void entry_recalculate_size(Entry *entry) {

    pthread_mutex_lock(&entry->mtx);
    entry->size_self = entry->stats.st_blksize;
    entry->size_total += entry->size_self;
    pthread_mutex_unlock(&entry->mtx);

    Entry *parent = entry->parent;
    while(parent) {
        pthread_mutex_lock(&parent->mtx);
        parent->size_total += entry->size_self;
        pthread_mutex_unlock(&parent->mtx);
        parent = parent->parent;
    }
}

void entry_register_dir(Entry *entry, Entry_Register_Task *task) {

    DIR *dir = opendir(entry->cpath);
    if(!dir) {
        entry->error = 1;
        return;
    }

    struct dirent *e;

    pthread_mutex_lock(&entry->mtx);
    while((e = readdir(dir))) {
        So dirname = so_l(e->d_name);
        if(!so_cmp(dirname, so(".")) || !so_cmp(dirname, so(".."))) continue;
        Entry *zero;
        NEW(Entry, zero);
        array_push(entry->list, zero);
        Entry *back = array_at(entry->list, array_len(entry->list) - 1);
        So path = SO;
        so_path_join(&path, so_ensure_dir(entry->path), so_ensure_dir(dirname));
        entry_register_dispatch(task->pw, task->sync_m, entry, back, path);
    }
    pthread_mutex_unlock(&entry->mtx);

    closedir(dir);

}

void *entry_register_async(Pw *pw, bool *cancel, void *user) {
    Entry_Register_Task *task = user;
    ASSERT_ARG(task);
    ASSERT_ARG(task->entry);
    /* probe stats */
    Entry *entry = task->entry;
    entry->parent = task->parent;
    entry->path = task->path;
    entry->cpath = so_dup(task->path);
    //struct stat stats;
    entry->error = lstat(entry->cpath, &entry->stats);
    //printff("STAT[%.*s]",SO_F(task->path));exit(1);
    entry_recalculate_size(entry);
    if(S_ISDIR(entry->stats.st_mode)) {
        entry_register_dir(entry, task);
    }
    tui_sync_main_both(task->sync_m);
    /* done */
    free(task);
    return 0;
}

void entry_register_dispatch(Pw *pw, Tui_Sync_Main *sync_m, Entry *parent, Entry *entry, So path) {
    Entry_Register_Task *task;
    ASSERT_ARG(pw);
    ASSERT_ARG(sync_m);
    ASSERT_ARG(entry);
    NEW(Entry_Register_Task, task);
    task->pw = pw;
    task->parent = parent;
    task->entry = entry;
    task->path = path;
    task->sync_m = sync_m;
    pw_queue(pw, entry_register_async, task);
}

