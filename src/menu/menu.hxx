#pragma once

#include "imgui.h"
#include "button/button.hxx"
#include "input/input.hxx"
#include "texture/texture.hxx"
#include "tabs/tabs.hxx"

#include <string>

enum class menu_state_t
{
    LOGIN,
    TRANSITION_TO_MAIN,
    MAIN_DASHBOARD,
    TRANSITION_TO_LOGIN
};

struct c_menu
{
    ImVec4 m_clear_color = ImVec4( 102.0f / 255.0f, 96.0f / 255.0f, 116.0f / 255.0f, 1.0f );
    bool   m_open = true;

    menu_state_t m_state = menu_state_t::LOGIN;

    ImVec2 m_pos = ImVec2( 0.0f, 0.0f );
    ImVec2 m_center_pos = ImVec2( 0.0f, 0.0f ); // Fixed anchor center point during morphing
    ImVec2 m_size = ImVec2( 805.0f, 275.0f );
    ImVec2 m_login_size = ImVec2( 805.0f, 275.0f );
    ImVec2 m_main_size  = ImVec2( 1101.0f, 665.0f );
    float  m_padding = 16.67f;
    float  m_spacing = 13.33f;
    bool   m_initialized = false;
    bool   m_dragging = false;
    ImVec2 m_drag_offset = ImVec2( 0.0f, 0.0f );

    c_texture   m_bg_texture;
    c_texture   m_logo_texture;
    c_texture   m_cici_texture;
    c_texture   m_ooo_texture;
    c_texture   m_search_texture;
    c_input     m_input_username;
    c_input     m_input_password;
    c_input     m_input_search;
    std::string m_username_text;
    std::string m_password_text;
    std::string m_search_text;

    animation_t m_anim_tg_hover;
    animation_t m_anim_discord_hover;
    animation_t m_anim_login_hover;
    animation_t m_anim_back_hover;

    // Sub-Navigation Carousel (460x58: General, Weapons, Filters)
    int         m_active_sub_nav = 0; // 0: General, 1: Weapons, 2: Filters
    animation_t m_anim_sub_left_arrow;
    animation_t m_anim_sub_right_arrow;
    animation_t m_anim_sub_nav_item[ 3 ];

    float m_trans_progress = 0.0f;    // 0.0f (Login) -> 1.0f (Main Dashboard 1071x665)

    // Symmetric Animation Timing Values (100% Identical Opening & Closing)
    float m_anim_speed   = 1.34f;     // Master speed (1.34x)
    float m_stage1_ratio = 0.25f;     // Stage 1: Cover Mask ratio (0.25)
    float m_stage2_ratio = 0.50f;     // Stage 2: Window Morph ratio (0.50)
    float m_stage3_ratio = 0.25f;     // Stage 3: Tabs Fade-In ratio (0.25)

    void init( );
    void draw_background( float const delta_time );
    void draw_foreground( ImFont* font_medium_32, float const delta_time );
    void draw( ImFont* font_medium_32, float const delta_time );
};

extern c_menu g_menu;
