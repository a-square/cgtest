#include <SDL.h>
#include "util.h"
#include "oldschool.h"

#define XT_SIZE 256
static u8 xt[XT_SIZE][XT_SIZE];

void init() {
    for (int y = 0; y < XT_SIZE; ++y) {
        for (int x = 0; x < XT_SIZE; x++) {
            xt[y][x] = x ^ y;
        }
    }
}

void handle_event(SDL_Event *event) {
    if (event->type == SDL_KEYUP) {
        destroy_window();
        exit(0);
    }
}

void next_frame_p(next_frame_params_t p) {
    for (int y = 0; y < g_screen_height; ++y) {
        for (int x = 0; x < g_screen_width; ++x) {
            u8 value = 255 * (y / (g_screen_height - 0.f));
            p.back[idx(x, y)] = (rgba_t){ xt[y & 0xff][x & 0xff], value, 255 - value, 255 };
        }
    }
}