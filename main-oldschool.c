#include <stddef.h>
#include <locale.h>
#include <SDL.h>
#include <gl3.h>
#include "util.h"
#include "oldschool.h"

typedef struct vertex {
    float x, y, z, u, v;
} __attribute__((packed)) vertex_t;

static GLuint g_vbuffer;
static vertex_t g_vs[4];
static GLuint g_shaders;

static frame_t g_frames[2];
static int g_current_frame = 0;

int g_screen_width, g_screen_height;

static frame_t create_frame() {
    return (frame_t) {
        .buffer = calloc(g_screen_width * g_screen_height, sizeof(rgba_t)),
        .ticks = 0,
    };
}

static void draw_frame() {
    // upload the frame to the GPU
    glTexImage2D(
        GL_TEXTURE_RECTANGLE,
        0,
        GL_RGBA,
        g_screen_width,
        g_screen_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        g_frames[g_current_frame].buffer
    ); verify_gl();

    glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(g_vs) / sizeof(*g_vs)); verify_gl();
}

static void loop() {
    static u32 lastFPSReport = 0, frameCounter = 0;
    static const u32 fpsInterval = 3000;

    for (;;) {
        // pass all pending events to the game
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handle_event(&event);
        }
        
        // get ticks and emit the FPS counter if needed
        u32 ticks = SDL_GetTicks();
        frameCounter++;
        if (ticks >= lastFPSReport + fpsInterval) {
            log_printf("FPS: %g", frameCounter / ((ticks - lastFPSReport) / 1000.f));
            lastFPSReport = ticks;
            frameCounter = 0;
        }
        
        int prev_frame = 1 - g_current_frame;
        
        // update the tick count of the current frame
        g_frames[g_current_frame].ticks = ticks;
        
        // game-specific software rendering
        next_frame(
            .front = g_frames[prev_frame].buffer,
            .back = g_frames[g_current_frame].buffer,
            .ticks = g_frames[g_current_frame].ticks,
            .prev_ticks = g_frames[prev_frame].ticks,
        );
        
        // draw the screen quad and swap buffers
        draw_frame();
        
        // swap buffers (including our textures)
        swap_window_buffers();
        g_current_frame = prev_frame;        
    }
}

static void initialize_objects() {
    // prepare memory for frames
    g_frames[0] = create_frame();
    g_frames[1] = create_frame();
    
    glDisable(GL_DITHER); verify_gl();
    
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array); verify_gl();
    glBindVertexArray(vertex_array); verify_gl();
    
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
    glBindTexture(GL_TEXTURE_RECTANGLE, texture); verify_gl();    
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST); verify_gl();
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST); verify_gl();
    
    static const char *vertex_shader_source = ""
        "#version 330\n"
        "\n"
        "in vec3 coord;\n"
        "in vec2 texCoord;\n"
        "out vec2 fragTexCoord;\n"
        "\n"
        "void main() {\n"
        "    fragTexCoord = texCoord;\n"
        "    gl_Position = vec4(coord, 1);\n"
        "}\n";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER); verify_gl();
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL); verify_gl();
    glCompileShader(vertex_shader); verify_gl_shader(vertex_shader);
    
    static const char *fragment_shader_source = ""
        "#version 330\n"
        "\n"
        "uniform sampler2DRect tex;\n"
        "in vec2 fragTexCoord;\n"
        "out vec4 finalColor;\n"
        "\n"
        "void main() {\n"
        "    finalColor = texture(tex, fragTexCoord);\n"
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

int main(int argc, char **argv) {
    argc--; argv++;
    
    // use system locale so that we can print UTF-8 to the console
    setlocale(LC_ALL, "");
    
    // initialize the OpenGL context
    bool fullscreen = (argc > 0 && strcmp(argv[0], "--fullscreen") == 0);
    make_window_return_t actual_size = make_window(
        .title = "Old-school CG",
        .fullscreen = fullscreen
    );
    g_screen_width = actual_size.width;
    g_screen_height = actual_size.height;
    
    initialize_objects();
    
    // game-specific initialization
    init();
    
    // game loop
    loop();
    
    // call destroy_window from the event handler to quit
    // destroy_window();
}