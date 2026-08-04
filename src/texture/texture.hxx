#pragma once

#include "imgui.h"

#ifdef USE_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <GLFW/glfw3.h>
#endif

struct c_texture
{
    GLuint m_texture_id = 0;
    int    m_width = 0;
    int    m_height = 0;
    bool   m_loaded = false;

    bool load_from_file( char const* filename );
    void cleanup( );
};

