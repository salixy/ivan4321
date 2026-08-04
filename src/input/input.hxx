#pragma once

#include "imgui.h"
#include "motion/motion.hxx"
#include <string>

struct c_input
{
    ImVec2      m_size = ImVec2( 423.33f, 69.33f );
    ImU32       m_bg_color = IM_COL32( 0x23, 0x23, 0x23, 255 );
    ImU32       m_placeholder_color = IM_COL32( 0x4A, 0x4A, 0x4A, 255 );
    ImU32       m_text_color = IM_COL32( 0xE2, 0xE2, 0xE2, 255 );
    ImU32       m_selection_color = IM_COL32( 50, 50, 50, 100 );
    float       m_pad_x = 38.67f;

    bool        m_focused = false;
    int         m_cursor_index = 0;
    int         m_select_index = 0;
    bool        m_is_selecting = false;
    float       m_lead_timer = 0.0f;
    bool        m_show_password = false;

    animation_t m_anim_cursor_offset;
    animation_t m_anim_select_start;
    animation_t m_anim_select_end;
    animation_t m_anim_cursor_alpha;
    animation_t m_anim_scroll_x;
    animation_t m_anim_fade_left;
    animation_t m_anim_fade_right;
    animation_t m_anim_eye_hover;
    animation_t m_anim_eye_switch;
    animation_t m_anim_hover;
    animation_t m_anim_focus;
    float       m_blink_timer = 0.0f;

    bool render( ImVec2 const& pos, char const* placeholder, std::string& text, ImFont* font_32, float const delta_time, bool const is_password = false, bool const is_search = false );
};
