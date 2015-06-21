#include <stddef.h>
#include <locale.h>
#include <getopt.h>
#include <SDL.h>
#include "opengl.h"
#include "util.h"
#include "oldschool.h"

typedef struct vertex {
    float x, y, z, u, v;
} __attribute__((packed)) vertex_t;

static GLuint g_vbuffer;
static vertex_t g_vs[4];
static GLuint g_shaders;

static frame_t g_frames[2];
static SDL_mutex *g_frame_mutexes[2];

int g_screen_width, g_screen_height;

//
// game initialization
//

static frame_t create_frame() {
    return (frame_t) {
        .buffer = calloc(g_screen_width * g_screen_height, sizeof(rgba_t)),
        .ticks = 0,
    };
}

//
// render loop
//

static void draw_frame(int current_frame) {
    // upload the frame to the GPU
    glTexImage2D(
        GL_TEXTURE_RECTANGLE_ARB,
        0,
        GL_RGBA,
        g_screen_width,
        g_screen_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        g_frames[current_frame].buffer
    ); verify_gl();

    glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(g_vs) / sizeof(*g_vs)); verify_gl();
}

typedef struct render_loop_params {
    bool emit_fps;
} render_loop_params_t;
#define render_loop(...) render_loop_p((render_loop_params_t){ __VA_ARGS__ })
static void render_loop_p(render_loop_params_t p) {
    u32 lastFPSReport = 0, frameCounter = 0;
    const u32 fpsInterval = 3000;
    
    int current_frame = 1, prev_frame = 1 - current_frame;

    for (;;) {
        verify_sdl(
            SDL_LockMutex(g_frame_mutexes[current_frame]),
            "Render loop mutex lock"
        );
        
        // emit the FPS counter if needed
        u32 ticks = SDL_GetTicks();
        if (p.emit_fps) {
            frameCounter++;
            if (ticks >= lastFPSReport + fpsInterval) {
                log_printf("FPS: %g", frameCounter / ((ticks - lastFPSReport) / 1000.f));
                lastFPSReport = ticks;
                frameCounter = 0;
            }
        }
        
        // pump pending input events to the game
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handle_event(&event);
        }
        
        // draw the screen quad and swap buffers
        draw_frame(current_frame);
        swap_window_buffers();
        
        verify_sdl(
            SDL_UnlockMutex(g_frame_mutexes[current_frame]),
            "Render loop mutex unlock"
        );

        current_frame = prev_frame;        
        prev_frame = 1 - current_frame;
    }
}

//
// game loop
//

int game_loop_thread(void *context __attribute__((unused))) {
    int current_frame = 0, prev_frame = 1 - current_frame;
    for (;;) {
        verify_sdl(
            SDL_LockMutex(g_frame_mutexes[current_frame]),
            "Game loop mutex lock"
        );
        
        // get current ticks (to be used by the game)
        u32 ticks = SDL_GetTicks();
        
        // game-specific software rendering
        g_frames[current_frame].ticks = ticks;
        next_frame(
            .front = g_frames[prev_frame].buffer,
            .back = g_frames[current_frame].buffer,
            .ticks = g_frames[current_frame].ticks,
            .prev_ticks = g_frames[prev_frame].ticks,
        );
        
        verify_sdl(
            SDL_UnlockMutex(g_frame_mutexes[current_frame]),
            "Game loop mutex unlock"
        );
        
        current_frame = prev_frame;
        prev_frame = 1 - current_frame;
    }
    
    return 0;
}

void game_loop() {
    for (int i = 0; i < 2; ++i) {
        verify(
            (g_frame_mutexes[i] = SDL_CreateMutex()),
            "Could not create a mutex: %s", SDL_GetError()
        );
    }
    
    verify(
        SDL_CreateThread(game_loop_thread, "Game loop", NULL),
        "Could not create the game loop thread: %s", SDL_GetError()
    );
}

//
// OpenGL initialization
//

static void initialize_objects() {
    // prepare memory for frames
    g_frames[0] = create_frame();
    g_frames[1] = create_frame();
    
    glDisable(GL_DITHER); verify_gl();
    
    float u = g_screen_width;
    float v = g_screen_height;
    g_vs[0] = (vertex_t){ .x = -1, .y = -1, .z = 0, .u = 0, .v = v }; // bottom left
    g_vs[1] = (vertex_t){ .x =  1, .y = -1, .z = 0, .u = u, .v = v }; // bottom right
    g_vs[2] = (vertex_t){ .x =  1, .y =  1, .z = 0, .u = u, .v = 0 }; // top right
    g_vs[3] = (vertex_t){ .x = -1, .y =  1, .z = 0, .u = 0, .v = 0 }; // top left

    glGenBuffers(1, &g_vbuffer); verify_gl();
    glBindBuffer(GL_ARRAY_BUFFER, g_vbuffer); verify_gl();
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vs), g_vs, GL_STATIC_DRAW); verify_gl();
    
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    glGenTextures(1, &texture); verify_gl();
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture); verify_gl();    
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST); verify_gl();
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST); verify_gl();
    
    static const char *vertex_shader_source = ""
        "#version 120\n"
        "\n"
        "attribute vec4 coord;\n"
        "attribute vec2 texCoord;\n"
        "varying vec2 fragTexCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = coord;\n"
        "    fragTexCoord = texCoord;\n"
        "}\n";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER); verify_gl();
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL); verify_gl();
    glCompileShader(vertex_shader); verify_gl_shader(vertex_shader);
    
    static const char *fragment_shader_source = ""
        "#version 120\n"
        "\n"
        "uniform sampler2DRect tex;\n"
        "varying vec2 fragTexCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_FragColor = texture2DRect(tex, fragTexCoord);\n"
        "}\n";
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER); verify_gl();
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL); verify_gl();
    glCompileShader(fragment_shader); verify_gl_shader(fragment_shader);
    
    g_shaders = glCreateProgram(); verify_gl();
    glAttachShader(g_shaders, vertex_shader); verify_gl();
    glAttachShader(g_shaders, fragment_shader); verify_gl();
    glLinkProgram(g_shaders); verify_gl_program(g_shaders);
    
    glUseProgram(g_shaders); verify_gl();
    glUniform1i(glGetUniformLocation(g_shaders, "tex"), 0); verify_gl();

    GLint coordPosition = glGetAttribLocation(g_shaders, "coord");
    glEnableVertexAttribArray(coordPosition); verify_gl();
    glVertexAttribPointer(
        coordPosition,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex_t),
        (void *)offsetof(vertex_t, x)
    ); verify_gl();

    GLint texCoordPosition = glGetAttribLocation(g_shaders, "texCoord");
    glEnableVertexAttribArray(texCoordPosition); verify_gl();
    glVertexAttribPointer(
        texCoordPosition,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vertex_t),
        (void *)offsetof(vertex_t, u)
    ); verify_gl();
}

//
// argument parsing and usage
//

typedef struct args {
    bool help;
    
    bool fullscreen;
    bool vsync;
    bool fps;
} args_t;

args_t parse_arguments(int argc, char **argv) {
    args_t args = {
        .help = false,
        .fullscreen = false,
        .vsync = false,
        .fps = false,
    };
    
    static struct option longopts[] = {
        { "help",       no_argument,        NULL,       'h' },
        { "fullscreen", no_argument,        NULL,       'f' },
        { "vsync",      no_argument,        NULL,       's' },
        { "fps",        no_argument,        NULL,       'p' },
        { NULL,         0,                  NULL,       0   }
    };
    
    int ch;
    while ((ch = getopt_long(argc, argv, "hfsp", longopts, NULL)) != -1) {
        switch (ch) {
        case 'f':
            args.fullscreen = true;
            break;
        case 's':
            args.vsync = true;
            break;
        case 'p':
            args.fps = true;
            break;
        case 'h':
        default:
            args.help = true;
        }
    }
    
    return args;
}

void usage() {
    static char message[] =
        "Usage:\n"
        "--help       (-h): this info\n"
        "--fullscreen (-f): run in the fullscreen mode\n"
        "--vsync      (-s): enable (adaptive) vsync\n"
        "--fps        (-p): emit fps counter every 3 seconds in console\n"
    ;
    
    fputs(message, stderr);
}

//
// main
//

int main(int argc, char **argv) {
    args_t args = parse_arguments(argc, argv);
    if (args.help) {
        usage();
        exit(1);
    }
    
    // use system locale so that we can print UTF-8 to the console
    setlocale(LC_ALL, "");
    
    // initialize the OpenGL context
    make_window_return_t actual_size = make_window(
        .title = "Old-school CG",
        .fullscreen = args.fullscreen,
        .vsync = args.vsync,
    );
    g_screen_width = actual_size.width;
    g_screen_height = actual_size.height;
    
    initialize_objects();
    
    // game-specific initialization
    init();
    
    // dual loops
    game_loop();
    render_loop(.emit_fps = args.fps);
    
    // call destroy_window from the event handler to quit
    // destroy_window();
}