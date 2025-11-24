#include <rltui.h>
#include <rlpw.h>
#include <math.h>
#include <limits.h>

#include "browse.h"

typedef struct Nurikabe {
    struct Tui_Core *tui;
    Tui_Sync sync;
    Tui_Core_Callbacks callbacks;
    /* layout */
    Tui_Rect rc_files;
    Tui_Rect rc_parent;
    Tui_Rect rc_preview;
    Tui_Rect rc_header;
    Pw pw_task;
    Browse browse;
    Entry entry_root;
} Nurikabe;

bool nurikabe_input(Tui_Input *input, bool *flush, void *user) {
    Nurikabe *nurikabe = user;
    switch(input->id) {
        case INPUT_TEXT: {
            switch(input->text.val) {
                case 'q': {
                    tui_core_quit(nurikabe->tui);
                } break;
            }
        } break;
        default: break;
    }
    return false;
}

bool nurikabe_update(void *user) {
    return true;
}

void nurikabe_render(Tui_Buffer *buffer, void *user) {
    Nurikabe *nk = user;

    tui_buffer_draw(buffer, nk->rc_header, 0, 0, 0, so("Nurikabe!"));
    tui_buffer_draw(buffer, nk->rc_header, 0, 0, 0, nk->entry_root.path);

    So tmp = SO;
    pthread_mutex_lock(&nk->entry_root.mtx);
    Tui_Buffer_Cache tbc = {
        .rect = nk->rc_files,
        .fill = true,
    };
    tbc.rect.dim.y = 1;
    for(size_t i = 0; i < array_len(nk->entry_root.list); ++i) {
        Entry *entry = array_at(nk->entry_root.list, i);
        so_clear(&tmp);
        so_fmt(&tmp, "%zu B -- %.*s", entry->size_total, SO_F(entry->path));
        tui_buffer_draw_cache(buffer, &tbc, tmp);
        tbc.rect.anc.y++;
        tbc.pt = (Tui_Point){0};
    }
    pthread_mutex_unlock(&nk->entry_root.mtx);

}

void nurikabe_resized(Tui_Point size, Tui_Point pixels, void *user) {
    Nurikabe *nk = user;

    /* header at top */
    nk->rc_header.anc.x = 0;
    nk->rc_header.anc.y = 0;
    nk->rc_header.dim.x = size.x;
    nk->rc_header.dim.y = 1;

    /* make space for header */
    nk->rc_files.anc.y = 1;
    nk->rc_files.dim.y = size.y - 1;
    nk->rc_parent.anc.y = 1;
    nk->rc_parent.anc.y = size.y - 1;
    nk->rc_preview.anc.y = 1;
    nk->rc_preview.anc.y = size.y - 1;

    /* figure out ratios */
    size_t w_parent = round((double)size.x / 10.0 * 1.0);
    size_t w_files = round((double)size.x / 10.0 * 6.0);
    size_t w_preview = round((double)size.x / 10.0 * 3.0);

    /* apply ratios to splits */
    nk->rc_parent.anc.x = 0;
    nk->rc_parent.dim.x = w_parent;
    nk->rc_files.anc.x = w_parent;
    nk->rc_files.dim.x = w_files;
    nk->rc_preview.anc.x = w_parent + w_files;
    nk->rc_preview.dim.x = w_preview;

}

int main(int argc, char **argv) {

    // TODO: array_itE is stupid. call it array_itB (back) and not End!

    Nurikabe nurikabe = {
        .callbacks.input = nurikabe_input,
        .callbacks.update = nurikabe_update,
        .callbacks.render = nurikabe_render,
        .callbacks.resized = nurikabe_resized,
        .tui = tui_core_new(),
    };

    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    pw_init(&nurikabe.pw_task, number_of_processors ? number_of_processors : 1);
    pw_dispatch(&nurikabe.pw_task);

    if(argc >= 2) {
        char creal[PATH_MAX];
        realpath(argv[1], creal);
        So real = so_clone(so_l(creal));
        entry_register_dispatch(&nurikabe.pw_task, &nurikabe.sync.main, 0, &nurikabe.entry_root, real);
    } else {
        char ccwd[PATH_MAX];
        getcwd(ccwd, PATH_MAX);
        So cwd = so_clone(so_l(ccwd));
        entry_register_dispatch(&nurikabe.pw_task, &nurikabe.sync.main, 0, &nurikabe.entry_root, cwd);
    }

    tui_enter();
    tui_core_init(nurikabe.tui, &nurikabe.callbacks, &nurikabe.sync, &nurikabe);
    while(tui_core_loop(nurikabe.tui)) {}
    tui_core_free(nurikabe.tui);
    tui_exit();

    return 0;
}

