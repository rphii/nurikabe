#include <rltui.h>

typedef struct Nurikabe {
    Tui_Sync sync;
    Tui_Core_Callbacks callbacks;
    struct Tui_Core *tui;
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
    return false;
}

void nurikabe_render(Tui_Buffer *buffer, void *user) {
}

void nurikabe_resized(Tui_Point size, Tui_Point pixels, void *user) {
}

int main(void) {

    Nurikabe nurikabe = {
        .callbacks.input = nurikabe_input,
        .callbacks.update = nurikabe_update,
        .callbacks.render = nurikabe_render,
        .callbacks.resized = nurikabe_resized,
        .tui = tui_core_new(),
    };

    tui_enter();
    tui_core_init(nurikabe.tui, &nurikabe.callbacks, &nurikabe.sync, &nurikabe);
    while(tui_core_loop(nurikabe.tui)) {}
    tui_core_free(nurikabe.tui);
    tui_exit();

    return 0;
}

