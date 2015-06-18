#include <math.h>
#include <SDL.h>
#include "util.h"
#include "oldschool.h"

#define XT_SIZE 256
static u8 xt[XT_SIZE][XT_SIZE];

typedef struct texcoord {
    int u, v;
} texcoord_t;

static texcoord_t *g_texmap = NULL;

static inline double max(double a, double b) {
    return a < b ? b : a;
}

void init() {
    // generate the xor texture
    for (int y = 0; y < XT_SIZE; ++y) {
        for (int x = 0; x < XT_SIZE; x++) {
            xt[y][x] = x ^ y;
        }
    }
    
    // generate the texture map
    double scale = max(g_screen_width - 1.0, g_screen_height - 1.0);
    double correction = 6 / (2 * M_PI);
    g_texmap = calloc(g_screen_width * g_screen_height, sizeof(texcoord_t));
    for (int y = 0; y < g_screen_height; ++y) {
        for (int x = 0; x < g_screen_width; x++) {
            // centered coordinates
            double cx = (x - (g_screen_width >> 1)) / scale;
            double cy = (y - (g_screen_height >> 1)) / scale;
            
            // transform to polar
            double dist = hypot(cx, cy);
            double angle = atan2(cy, cx);
            
            // compute the projected distance
            double new_dist = (dist - 1) / dist;
            
            // map the texture cylindrically
            g_texmap[idx(x, y)] = (texcoord_t) {
                .u = correction * (XT_SIZE - 1) * new_dist,
                .v = correction * (XT_SIZE - 1) * angle,
            };
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
            int i = idx(x, y);
            texcoord_t *uv = g_texmap + i;
            int u = uv->u + p.ticks;
            int v = uv->v + p.ticks / 2;
            p.back[i] = (rgba_t){ 0, 0, xt[v & 0xff][u & 0xff], 255 };
        }
    }
}