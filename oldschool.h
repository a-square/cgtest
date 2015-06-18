#pragma once

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef struct rgba {
    u8 r, g, b, a;
} __attribute__((packed)) rgba_t;

typedef struct frame {
    rgba_t *buffer;
    unsigned int ticks;
} frame_t;

extern int g_screen_width, g_screen_height;

static inline int idx(int x, int y) {
    return y * g_screen_width + x;
}

//
// the client must provide the following functions
//

void init();
void handle_event(SDL_Event *event);

typedef struct next_frame_params {
    const rgba_t * restrict front;
    rgba_t * restrict back;
    unsigned int ticks, prev_ticks;
} next_frame_params_t;

void next_frame_p(next_frame_params_t p);
#define next_frame(...) next_frame_p((next_frame_params_t){ __VA_ARGS__ })