#pragma once

#include "imgui.h"

struct c_fonts
{
    ImFont* m_regular_18 = nullptr;
    ImFont* m_medium_18 = nullptr;
    ImFont* m_medium_32 = nullptr;
    ImFont* m_calsans_42 = nullptr;
    ImFont* m_brand_font_40 = nullptr;
    ImFont* m_icon_font = nullptr;

    bool init( );
};

extern c_fonts g_fonts;
