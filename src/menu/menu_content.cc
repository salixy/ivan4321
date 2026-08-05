#include "menu/menu_content.hxx"
#include "menu/menu.hxx"
#include "tabs/tabs.hxx"
#include "widgets/widgets.hxx"
#include "settings/settings.hxx"

#include <vector>

namespace menu_content
{
    void render_dashboard_content(
        c_menu* menu,
        float const tabs_alpha,
        float const ease_t,
        ImFont* font_medium_32,
        float const delta_time
    )
    {
        if ( tabs_alpha <= 0.001f ) { return; }

        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
        int const a_255 = ( int )( 255.0f * tabs_alpha );

        float const search_slide_x = ( 1.0f - ease_t ) * 30.0f;

        // Push clip rect to strictly constrain all dashboard cards & widgets within the main window boundary!
        draw_list->PushClipRect( ImVec2( menu->m_pos.x + 250.0f, menu->m_pos.y ), ImVec2( menu->m_pos.x + menu->m_size.x, menu->m_pos.y + menu->m_size.y ), true );

        // Content Area Rectangle Container (#1A1A1A, extending to top, bottom, and right edges of menu)
        ImVec2 const content_min = ImVec2( menu->m_pos.x + 265.0f + search_slide_x, menu->m_pos.y );
        ImVec2 const content_max = ImVec2( menu->m_pos.x + menu->m_size.x, menu->m_pos.y + menu->m_size.y );

        draw_list->AddRectFilled( content_min, content_max, IM_COL32( 0x1A, 0x1A, 0x1A, a_255 ), 40.0f );

        // Render active tab & subtab content cards
        if ( g_tabs.m_active_tab >= 0 && g_tabs.m_active_tab < c_tabs::k_tab_count )
        {
            int const active_tab = g_tabs.m_active_tab;
            int const active_sub = g_tabs.m_active_sub_tab[ active_tab ];

            ImVec2 const col1_pos = ImVec2( content_min.x + 25.0f, content_min.y + 25.0f );
            ImVec2 const col2_pos = ImVec2( content_min.x + 420.0f, content_min.y + 25.0f );

            if ( active_tab == 0 ) // Combat
            {
                if ( active_sub == 0 ) // Aim
                {
                    ImGui::SetCursorScreenPos( ImVec2( col2_pos.x + 10.0f, col2_pos.y + 20.0f ) );
                    std::vector<char const*> bone_items = { "Head", "Neck", "Chest" };
                    g_widgets.combo( "Aim Bone", &g_settings.m_combat.m_aim_bone, bone_items, tabs_alpha, delta_time, font_medium_32, "combat_aim_bone" );
                }
            }
            else if ( active_tab == 1 ) // Visuals
            {
                if ( active_sub == 2 ) // Settings
                {
                    ImGui::SetCursorScreenPos( ImVec2( col2_pos.x + 18.0f, col2_pos.y + 55.0f ) );
                    g_widgets.color_picker_dot( "ESP Color", g_settings.m_visuals.m_esp_color, tabs_alpha, delta_time, font_medium_32, "vis_col_esp" );
                    g_widgets.color_picker_dot( "Box Color", g_settings.m_visuals.m_esp_color, tabs_alpha, delta_time, font_medium_32, "vis_col_box" );
                    g_widgets.color_picker_dot( "Skeleton Color", g_settings.m_visuals.m_esp_color, tabs_alpha, delta_time, font_medium_32, "vis_col_skel" );
                    g_widgets.color_picker_dot( "Chams Color", g_settings.m_visuals.m_chams_color, tabs_alpha, delta_time, font_medium_32, "vis_col_chams" );
                    g_widgets.color_picker_dot( "Glow Color", g_settings.m_visuals.m_chams_color, tabs_alpha, delta_time, font_medium_32, "vis_col_glow" );
                    g_widgets.color_picker_dot( "Snapline Color", g_settings.m_visuals.m_esp_color, tabs_alpha, delta_time, font_medium_32, "vis_col_snap" );
                }
            }
            else if ( active_tab == 3 ) // World
            {
                if ( active_sub == 1 ) // Visual Style
                {
                    ImGui::SetCursorScreenPos( ImVec2( col1_pos.x + 18.0f, col1_pos.y + 55.0f ) );
                    std::vector<char const*> skyboxes = { "Default", "Sunset", "Night", "Custom" };
                    g_widgets.combo( "Skybox", &g_settings.m_world.m_skybox, skyboxes, tabs_alpha, delta_time, font_medium_32, "world_skybox" );
                    g_widgets.color_picker_dot( "World Color", g_settings.m_world.m_world_color, tabs_alpha, delta_time, font_medium_32, "world_mod_col" );
                }
            }
            else if ( active_tab == 5 ) // Misc
            {
                if ( active_sub == 1 ) // Utilities
                {
                    ImGui::SetCursorScreenPos( ImVec2( col1_pos.x + 18.0f, col1_pos.y + 55.0f ) );
                    std::vector<char const*> sounds = { "Default", "Metallic", "Bell", "Bubble" };
                    g_widgets.combo( "Hit Sound", &g_settings.m_misc.m_hit_sound, sounds, tabs_alpha, delta_time, font_medium_32, "misc_hit_sound" );
                }
            }
            else if ( active_tab == 6 ) // Configs
            {
                if ( active_sub == 0 ) // Manager
                {
                    ImGui::SetCursorScreenPos( ImVec2( col1_pos.x + 18.0f, col1_pos.y + 55.0f ) );
                    std::vector<char const*> presets = { "Default", "Legit", "Rage", "Custom" };
                    g_widgets.combo( "Active Config", &g_settings.m_configs.m_preset_index, presets, tabs_alpha, delta_time, font_medium_32, "cfg_preset" );
                }
            }
        }

        draw_list->PopClipRect( );
    }
}
