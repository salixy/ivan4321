#pragma once

#include "imgui.h"
#include "motion/motion.hxx"

struct sub_tab_item_t
{
    char const* m_name;
};

struct tab_item_t
{
    char const*     m_name;
    char const*     m_icon;
    int             m_sub_count;
    sub_tab_item_t m_sub_tabs[ 4 ];
};

struct c_tabs
{
    static constexpr int k_tab_count = 7;

    int         m_active_tab = -1; // Closed by default!
    int         m_active_sub_tab[ k_tab_count ] = { 0, 0, 0, 0, 0, 0, 0 };

    animation_t m_anim_tab_hover[ k_tab_count ];
    animation_t m_anim_tab_active[ k_tab_count ];
    animation_t m_anim_sub_expand[ k_tab_count ];
    animation_t m_anim_sub_hover[ k_tab_count ][ 4 ];
    animation_t m_anim_sub_bullet_rel_y[ k_tab_count ]; // Relative local panel Y coordinate

    void render( ImVec2 const& panel_pos, float const panel_w, float const panel_h, float const alpha, float const delta_time, bool const can_interact, ImFont* font_medium_32, ImFont* icon_font );
};

extern c_tabs g_tabs;
