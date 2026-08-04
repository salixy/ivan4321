#pragma once

#include "imgui.h"
#include "motion/motion.hxx"

struct c_button
{
    ImVec2      m_size = ImVec2( 423.33f, 69.33f );
    ImU32       m_bg_color = IM_COL32( 0x7B, 0x73, 0xAD, 255 );
    ImU32       m_label_color = IM_COL32( 0x26, 0x26, 0x26, 255 );

    animation_t m_anim_hover;
    animation_t m_anim_click;

    bool render( ImVec2 const& pos, char const* label, ImFont* font_32, float const delta_time );
};

extern c_button g_button;
