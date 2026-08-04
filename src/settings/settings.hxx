#pragma once

#include <vector>

struct combat_settings_t
{
    // General (Subtab 0)
    bool  m_aim_assist    = true;
    float m_aim_fov       = 90.0f;   // 1 .. 360
    float m_aim_smooth    = 15.0f;   // 1 .. 100
    int   m_aim_bone      = 0;       // 0: Head, 1: Neck, 2: Chest
    bool  m_visible_check = true;
    bool  m_team_check    = true;

    // Targeting (Subtab 1)
    bool  m_silent_aim    = false;
    bool  m_auto_shoot    = false;
    bool  m_triggerbot    = true;
    int   m_trigger_delay = 50;      // 0 .. 500 ms

    // Accuracy (Subtab 2)
    float m_recoil_control= 80.0f;   // 0 .. 100 %
    float m_spread_control= 50.0f;   // 0 .. 100 %
};

struct visuals_settings_t
{
    // ESP (Subtab 0)
    bool  m_esp_enable     = true;
    bool  m_box_esp        = true;
    bool  m_corner_box     = false;
    bool  m_skeleton_esp   = true;
    bool  m_health_bar     = true;
    bool  m_armor_bar      = false;
    bool  m_name_esp       = true;
    bool  m_distance_esp   = true;
    bool  m_weapon_esp     = false;
    bool  m_snaplines      = false;

    // Chams & Glow (Subtab 1)
    bool  m_glow           = true;
    bool  m_chams          = true;
    bool  m_crosshair      = true;
    bool  m_fov_circle     = true;
    bool  m_radar          = true;

    // Settings (Subtab 2)
    float m_esp_distance   = 250.0f;  // 10 .. 500 m
    float m_box_thickness  = 1.5f;    // 1 .. 5 px
    float m_line_thickness = 1.5f;    // 1 .. 5 px
    float m_font_size      = 18.0f;   // 12 .. 32 px
    float m_opacity        = 100.0f;  // 0 .. 100 %
    float m_esp_color[ 4 ] = { 0.48f, 0.45f, 0.67f, 1.0f };
    float m_chams_color[ 4 ] = { 0.48f, 0.45f, 0.67f, 1.0f };
};

struct misc_settings_t
{
    // General (Subtab 0)
    bool  m_auto_accept    = true;
    bool  m_auto_respawn   = false;
    bool  m_spectator_list = true;
    bool  m_watermark      = true;

    // Utilities (Subtab 1)
    bool  m_fps_booster    = true;
    float m_unlock_fps     = 240.0f;  // 30 .. 300 FPS
    int   m_hit_sound      = 0;       // 0: Default, 1: Metallic, 2: Bell, 3: Bubble
    bool  m_hit_marker     = true;
    bool  m_kill_effect    = true;
};

struct config_settings_t
{
    int   m_preset_index   = 0;
    bool  m_auto_load      = true;
};

struct menu_settings_t
{
    // Appearance (Subtab 0)
    float m_menu_scale     = 100.0f;  // 50 .. 200 %
    float m_menu_blur      = 15.0f;   // 0 .. 100 %
    float m_accent_color[ 4 ] = { 0.48f, 0.45f, 0.67f, 1.0f };
    float m_bg_opacity     = 90.0f;   // 0 .. 100 %

    // Animation & Style (Subtab 1)
    float m_anim_speed     = 16.0f;   // 1 .. 30
    float m_rounded_corners= 20.0f;   // 0 .. 30 px
    bool  m_rainbow_accent = false;
};

struct world_settings_t
{
    // Environment (Subtab 0)
    bool  m_full_bright    = true;
    float m_night_mode     = 0.0f;    // 0 .. 100 %
    bool  m_remove_fog     = true;
    bool  m_remove_smoke   = true;
    bool  m_remove_flash   = true;

    // Visual Style (Subtab 1)
    int   m_skybox         = 0;       // 0: Default, 1: Sunset, 2: Night, 3: Custom
    float m_world_color[ 4 ] = { 0.48f, 0.45f, 0.67f, 1.0f };
};

struct movement_settings_t
{
    // Assistance (Subtab 0)
    bool  m_bhop           = true;
    bool  m_auto_strafe    = false;
    bool  m_no_slow        = true;
    bool  m_fast_stop      = true;

    // Exploits (Subtab 1)
    bool  m_edge_jump      = false;
    bool  m_jump_bug       = false;
    bool  m_auto_peek      = true;
    float m_speed_mult     = 1.0f;    // 1.0 .. 5.0 x
};

struct c_settings
{
    combat_settings_t   m_combat;
    visuals_settings_t  m_visuals;
    misc_settings_t     m_misc;
    config_settings_t   m_configs;
    menu_settings_t     m_menu;
    world_settings_t    m_world;
    movement_settings_t m_movement;
};

extern c_settings g_settings;
