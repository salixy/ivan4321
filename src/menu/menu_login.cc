#include "menu/menu_login.hxx"
#include "menu/menu.hxx"
#include "fonts/fonts.hxx"
#include "button/button.hxx"

#include <algorithm>
#include <cmath>

static inline float lerp_f( float a, float b, float t )
{
    return a + ( b - a ) * t;
}

namespace menu_login
{
    void render_left_panel(
        ImVec2 const& pos,
        ImVec2 const& size,
        float const ease_t,
        c_texture const& bg_texture,
        c_texture const& cici_texture,
        animation_t& anim_tg_hover,
        animation_t& anim_discord_hover,
        float const delta_time
    )
    {
        ImGuiIO const& io = ImGui::GetIO( );
        ImVec2 const mouse_pos = io.MousePos;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

        float const cover_alpha = std::clamp( ease_t * 2.5f, 0.0f, 1.0f );
        float const left_fade = 1.0f - cover_alpha;
        int const left_a = ( int )( left_fade * 255.0f );

        float const fast_fade = std::clamp( 1.0f - ease_t * 4.5f, 0.0f, 1.0f );
        int const fast_a = ( int )( fast_fade * 255.0f );

        float const rounding = 40.0f;

        if ( fast_fade > 0.001f )
        {
            ImVec2 const p_left_min = ImVec2( pos.x, pos.y - 1.0f );
            ImVec2 const p_left_max = ImVec2( pos.x + 351.0f, pos.y + size.y + 1.0f );
            ImDrawFlags const left_rounding_flags = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomLeft;

            draw_list->PushClipRect( p_left_min, p_left_max, true );

            draw_list->AddRectFilled( p_left_min, p_left_max, IM_COL32( 0x7A, 0x72, 0xAC, fast_a ), rounding, left_rounding_flags );

            if ( bg_texture.m_loaded )
            {
                draw_list->AddImageRounded(
                    ( ImTextureID )( intptr_t )bg_texture.m_texture_id,
                    p_left_min, p_left_max,
                    ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 1.0f ),
                    IM_COL32( 255, 255, 255, fast_a ),
                    rounding, left_rounding_flags
                );
            }

            if ( cici_texture.m_loaded && fast_fade > 0.001f )
            {
                float const aspect1 = ( float )cici_texture.m_width / ( float )cici_texture.m_height;
                float const char_h = size.y;
                float const char_w = char_h * aspect1;
                float const char_x = pos.x + 351.0f - char_w;
                float const char_y = pos.y;

                ImVec2 const cici_min = ImVec2( char_x, char_y );
                ImVec2 const cici_max = ImVec2( char_x + char_w, char_y + char_h );

                draw_list->AddImage(
                    ( ImTextureID )( intptr_t )cici_texture.m_texture_id,
                    cici_min, cici_max, ImVec2( 0.0f, 0.0f ),
                    ImVec2( 1.0f, 1.0f ), IM_COL32( 255, 255, 255, ( int )( fast_fade * 128.0f ) )
                );
            }

            // Render Telegram & Discord brand icons
            ImFont* brand_font = ( g_fonts.m_brand_font_40 != nullptr ) ? g_fonts.m_brand_font_40 : g_fonts.m_icon_font;
            if ( brand_font != nullptr && fast_fade > 0.001f )
            {
                ImVec2 const tg_pos = ImVec2( pos.x + 31.0f, pos.y + 102.67f );
                ImVec2 const discord_pos = ImVec2( tg_pos.x + 35.0f, pos.y + 102.67f );

                bool const is_tg_hovered = ( mouse_pos.x >= tg_pos.x && mouse_pos.x <= tg_pos.x + 26.67f &&
                                             mouse_pos.y >= tg_pos.y && mouse_pos.y <= tg_pos.y + 26.67f );

                bool const is_discord_hovered = ( mouse_pos.x >= discord_pos.x && mouse_pos.x <= discord_pos.x + 26.67f &&
                                                  mouse_pos.y >= discord_pos.y && mouse_pos.y <= discord_pos.y + 26.67f );

                anim_tg_hover.m_speed = 14.0f;
                anim_tg_hover.set( is_tg_hovered ? 1.0f : 0.0f );
                anim_tg_hover.update( delta_time );

                anim_discord_hover.m_speed = 14.0f;
                anim_discord_hover.set( is_discord_hovered ? 1.0f : 0.0f );
                anim_discord_hover.update( delta_time );

                float const tg_val = anim_tg_hover.m_value;
                float const discord_val = anim_discord_hover.m_value;

                ImU32 const col_tg = IM_COL32(
                    ( int )( 156.0f + 99.0f * tg_val ),
                    ( int )( 148.0f + 107.0f * tg_val ),
                    ( int )( 200.0f + 55.0f * tg_val ),
                    fast_a
                );

                ImU32 const col_discord = IM_COL32(
                    ( int )( 156.0f + 99.0f * discord_val ),
                    ( int )( 148.0f + 107.0f * discord_val ),
                    ( int )( 200.0f + 55.0f * discord_val ),
                    fast_a
                );

                char const* icon_telegram = "\xef\x8b\x86";
                char const* icon_discord  = "\xef\x8e\x92";

                draw_list->AddText( brand_font, 26.67f, tg_pos, col_tg, icon_telegram );
                draw_list->AddText( brand_font, 26.67f, discord_pos, col_discord, icon_discord );
            }

            // Text "mareland" & "best of best."
            ImFont* calsans_font = ( g_fonts.m_calsans_42 != nullptr ) ? g_fonts.m_calsans_42 : g_fonts.m_medium_32;
            if ( calsans_font != nullptr && fast_fade > 0.001f )
            {
                float const text_x = pos.x + 31.0f;
                float const text_y = pos.y + 182.0f;
                ImU32 const text_col = IM_COL32( 255, 255, 255, fast_a );

                draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x + 0.8f, text_y ), text_col, "mareland" );
                draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x, text_y ), text_col, "mareland" );

                ImU32 const col_subtext = IM_COL32( 0x1B, 0x16, 0x29, fast_a );
                float const line2_y = text_y + 30.0f;
                draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x + 0.8f, line2_y ), col_subtext, "best of best." );
                draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x, line2_y ), col_subtext, "best of best." );
            }

            draw_list->PopClipRect( );
        }
    }

    void render_inputs(
        c_menu* menu,
        float const ease_t,
        ImFont* font_medium_32,
        float const delta_time
    )
    {
        float const cover_alpha = std::clamp( ease_t * 2.5f, 0.0f, 1.0f );
        if ( cover_alpha >= 0.999f ) { return; }

        float const start_x = menu->m_pos.x + 805.0f - menu->m_padding - menu->m_input_username.m_size.x;
        float const start_y = menu->m_pos.y + menu->m_padding + 2.67f;

        ImVec2 const pos_input1 = ImVec2( start_x, start_y );
        ImVec2 const pos_input2 = ImVec2( start_x, pos_input1.y + menu->m_input_username.m_size.y + menu->m_spacing );
        ImVec2 const pos_button = ImVec2( start_x, pos_input2.y + menu->m_input_password.m_size.y + menu->m_spacing );

        ImGuiIO const& io = ImGui::GetIO( );
        ImVec2 const mouse_pos = io.MousePos;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

        draw_list->PushClipRect( menu->m_pos, ImVec2( menu->m_pos.x + menu->m_size.x, menu->m_pos.y + menu->m_size.y ), true );

        // Render Username Input
        menu->m_input_username.m_bg_color = IM_COL32( 0x1E, 0x1E, 0x1E, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        menu->m_input_username.m_placeholder_color = IM_COL32( 0x50, 0x50, 0x50, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        menu->m_input_username.m_text_color = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        menu->m_input_username.render( pos_input1, "Username", menu->m_username_text, font_medium_32, delta_time, false );

        // Render Password Input
        menu->m_input_password.m_bg_color = IM_COL32( 0x1E, 0x1E, 0x1E, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        menu->m_input_password.m_placeholder_color = IM_COL32( 0x50, 0x50, 0x50, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        menu->m_input_password.m_text_color = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        menu->m_input_password.render( pos_input2, "Password", menu->m_password_text, font_medium_32, delta_time, true );

        // Render Login Button
        float const btn_w = 423.33f;
        float const btn_h = 69.33f;

        bool const is_btn_hovered = ( mouse_pos.x >= pos_button.x && mouse_pos.x <= pos_button.x + btn_w &&
                                      mouse_pos.y >= pos_button.y && mouse_pos.y <= pos_button.y + btn_h );

        menu->m_anim_login_hover.m_speed = 18.0f;
        menu->m_anim_login_hover.set( is_btn_hovered ? 1.0f : 0.0f );
        menu->m_anim_login_hover.update( delta_time );

        float const btn_h_val = menu->m_anim_login_hover.m_value;
        int const r_bg = ( int )lerp_f( 0x7B, 0x8C, btn_h_val );
        int const g_bg = ( int )lerp_f( 0x73, 0x84, btn_h_val );
        int const b_bg = ( int )lerp_f( 0xAD, 0xBF, btn_h_val );
        int const btn_a = ( int )( ( 1.0f - cover_alpha ) * 255.0f );

        ImU32 const btn_bg_col = IM_COL32( r_bg, g_bg, b_bg, btn_a );
        draw_list->AddRectFilled( pos_button, ImVec2( pos_button.x + btn_w, pos_button.y + btn_h ), btn_bg_col, 36.0f );

        // Text "Login"
        if ( font_medium_32 != nullptr )
        {
            ImVec2 const txt_sz = font_medium_32->CalcTextSizeA( 21.33f, FLT_MAX, 0.0f, "Login" );
            ImVec2 const txt_pos = ImVec2( pos_button.x + ( btn_w - txt_sz.x ) * 0.5f, pos_button.y + ( btn_h - txt_sz.y ) * 0.5f );
            draw_list->AddText( font_medium_32, 21.33f, txt_pos, IM_COL32( 0x26, 0x26, 0x26, btn_a ), "Login" );
        }

        // Process Click or Enter to trigger morph animation to Main Dashboard
        bool const is_enter_pressed = ImGui::IsKeyPressed( ImGuiKey_Enter ) || ImGui::IsKeyPressed( ImGuiKey_KeypadEnter );
        if ( ( ( is_btn_hovered && io.MouseClicked[ 0 ] ) || is_enter_pressed ) && menu->m_state == menu_state_t::LOGIN )
        {
            menu->m_center_pos = ImVec2( menu->m_pos.x + menu->m_size.x * 0.5f, menu->m_pos.y + menu->m_size.y * 0.5f );
            menu->m_state = menu_state_t::TRANSITION_TO_MAIN;
        }

        // Overlay Cover Mask over Right Inputs Area during transition
        if ( cover_alpha > 0.001f )
        {
            int const mask_a = ( int )( 255.0f * cover_alpha );
            draw_list->AddRectFilled(
                ImVec2( menu->m_pos.x + 351.0f, menu->m_pos.y ),
                ImVec2( menu->m_pos.x + menu->m_size.x, menu->m_pos.y + menu->m_size.y ),
                IM_COL32( 0x16, 0x16, 0x16, mask_a ),
                40.0f, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight
            );
        }

        draw_list->PopClipRect( );
    }
}
