#pragma once

#include "imgui.h"

#ifdef USE_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "imgui_impl_sdl2.h"
#else
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#endif

#include "imgui_impl_opengl3.h"

struct c_render
{
#ifdef USE_SDL
    SDL_Window*   m_window = nullptr;
    SDL_GLContext m_gl_context = nullptr;
#else
    GLFWwindow*   m_window = nullptr;
#endif
    ImFont*       m_font_regular = nullptr;
    ImFont*       m_font_medium = nullptr;
    ImFont*       m_font_medium_32 = nullptr;

    bool init( int const width, int const height, char const* title );
    void shutdown( );

    void start_frame( );
    void end_frame( ImVec4 const& clear_color );

    bool should_close( ) const;
    double get_time( ) const;
    float get_delta_time( ) const;
    void limit_fps( double const target_fps, double const current_time ) const;
};

extern c_render g_render;

