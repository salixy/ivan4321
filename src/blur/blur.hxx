#pragma once

#include "imgui.h"

// Blur completely removed per user request.

struct c_blur
{
    void init( ) { }
    void cleanup( ) { }
    void process_blur( int = 0, int = 0 ) { }
    void capture_and_blur( int = 0, int = 0 ) { }
    void draw_backdrop( ImDrawList*, ImVec2 const&, ImVec2 const&, float = 0.0f ) { }
};

extern c_blur g_blur;
