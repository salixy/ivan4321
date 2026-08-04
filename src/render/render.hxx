#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

struct c_render
{
    GLFWwindow* m_window = nullptr;
    ImFont*     m_font_regular = nullptr;
    ImFont*     m_font_medium = nullptr;
    ImFont*     m_font_medium_32 = nullptr;

    bool init( int const width, int const height, char const* title );
    void shutdown( );

    void start_frame( );
    void end_frame( ImVec4 const& clear_color );

    bool should_close( ) const;
    float get_delta_time( ) const;
    void limit_fps( double const target_fps, double const current_time ) const;
};

extern c_render g_render;
