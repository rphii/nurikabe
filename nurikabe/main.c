#include <rltui.h>

typedef struct Nue {
    Tui_Sync sync;
    Tui_Core_Callbacks callbacks;
    struct Tui_Core *tui;
} Nue;

bool nue_input(Tui_Input *input, bool *flush, void *user) {
    Nue *nue = user;
    switch(input->id) {
        case INPUT_TEXT: {
            switch(input->text.val) {
                case 'q': {
                    tui_core_quit(nue->tui);
                } break;
            }
        } break;
        default: break;
    }
    return false;
}

bool nue_update(void *user) {
    return false;
}

void nue_render(Tui_Buffer *buffer, void *user) {
}

void nue_resized(Tui_Point size, Tui_Point pixels, void *user) {
}

int main(void) {

    Nue nue = {
        .callbacks.input = nue_input,
        .callbacks.update = nue_update,
        .callbacks.render = nue_render,
        .callbacks.resized = nue_resized,
        .tui = tui_core_new(),
    };

    tui_enter();
    tui_core_init(nue.tui, &nue.callbacks, &nue.sync, &nue);
    while(tui_core_loop(nue.tui)) {}
    tui_core_free(nue.tui);
    tui_exit();

    return 0;
}

