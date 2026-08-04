#include "menu/menu.hxx"
#include "fonts/fonts.hxx"
#include "blur/blur.hxx"
#include "input/input.hxx"
#include "button/button.hxx"
#include "widgets/widgets.hxx"
#include "settings/settings.hxx"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

c_menu g_menu;

static bool in_rect( ImVec2 const& pt, ImVec2 const& rect_min, ImVec2 const& rect_size )
{
    return ( pt.x >= rect_min.x && pt.x <= rect_min.x + rect_size.x &&
             pt.y >= rect_min.y && pt.y <= rect_min.y + rect_size.y );
}

static inline float lerp_f( float a, float b, float t )
{
    return a + ( b - a ) * t;
}

void c_menu::init( )
{
    m_bg_texture.load_from_file( "assets/bg.png" );
    m_logo_texture.load_from_file( "assets/logo.png" );
    m_cici_texture.load_from_file( "assets/cici.png" );
    if ( !m_ooo_texture.load_from_file( "assets/ooo.png" ) )
    {
        m_ooo_texture.load_from_file( "assets/cici.png" );
    }
    m_search_texture.load_from_file( "assets/search.png" );

    m_state = menu_state_t::LOGIN;
    m_trans_progress = 0.0f;
}

void c_menu::draw_background( float const delta_time )
{
    if ( !m_open ) { return; }

    ImGuiIO const& io = ImGui::GetIO( );
    ImVec2 const mouse_pos = io.MousePos;
    bool const mouse_down = io.MouseDown[ 0 ];

    if ( m_state == menu_state_t::TRANSITION_TO_MAIN )
    {
        m_trans_progress += m_anim_speed * delta_time;
        if ( m_trans_progress >= 1.0f )
        {
            m_trans_progress = 1.0f;
            m_state = menu_state_t::MAIN_DASHBOARD;
        }
    }
    else if ( m_state == menu_state_t::TRANSITION_TO_LOGIN )
    {
        m_trans_progress -= m_anim_speed * delta_time;
        if ( m_trans_progress <= 0.0f )
        {
            m_trans_progress = 0.0f;
            m_state = menu_state_t::LOGIN;
        }
    }

    float const t_val = m_trans_progress;

    auto ease_quintic = []( float const p ) -> float
    {
        float const clamped_p = std::clamp( p, 0.0f, 1.0f );
        return ( clamped_p < 0.5f ) ? ( 16.0f * clamped_p * clamped_p * clamped_p * clamped_p * clamped_p ) : ( 1.0f - std::pow( -2.0f * clamped_p + 2.0f, 5.0f ) * 0.5f );
    };

    float const ease_t = ease_quintic( t_val );
    float const ease_w3 = ease_t;
    float const cover_alpha = ease_t;
    float const tabs_alpha = ease_t;

    m_size.x = lerp_f( m_login_size.x, m_main_size.x, ease_t );
    m_size.y = lerp_f( m_login_size.y, m_main_size.y, ease_t );

    if ( !m_initialized )
    {
        m_pos = ImVec2(
            ( io.DisplaySize.x - m_size.x ) * 0.5f,
            ( io.DisplaySize.y - m_size.y ) * 0.5f
        );
        m_initialized = true;
    }

    if ( m_state == menu_state_t::LOGIN || m_state == menu_state_t::MAIN_DASHBOARD || m_dragging || m_center_pos.x <= 1.0f )
    {
        m_center_pos = ImVec2( m_pos.x + m_size.x * 0.5f, m_pos.y + m_size.y * 0.5f );
    }

    if ( m_state == menu_state_t::TRANSITION_TO_MAIN || m_state == menu_state_t::TRANSITION_TO_LOGIN )
    {
        if ( !m_dragging )
        {
            m_pos = ImVec2( std::round( m_center_pos.x - m_size.x * 0.5f ), std::round( m_center_pos.y - m_size.y * 0.5f ) );
        }
    }

    // Window dragging logic
    float const input_x = m_pos.x + 805.0f - m_padding - m_input_username.m_size.x;
    float const input_y = m_pos.y + m_padding + 2.67f;

    ImVec2 const preview_input1 = ImVec2( input_x, input_y );
    ImVec2 const preview_input2 = ImVec2( input_x, preview_input1.y + m_input_username.m_size.y + m_spacing );
    ImVec2 const preview_button = ImVec2( input_x, preview_input2.y + m_input_password.m_size.y + m_spacing );

    float const panel_x_calc = m_pos.x - 30.0f * ease_w3;
    float const panel_y_calc = m_pos.y + ( m_size.y - 641.0f ) * 0.5f;

    float const search_x_calc = panel_x_calc + 280.0f + 21.0f;
    float const search_y_calc = m_pos.y + 11.0f;
    ImVec2 const preview_search = ImVec2( search_x_calc, search_y_calc );
    ImVec2 const search_size = ImVec2( 280.0f, 58.0f );

    ImVec2 const preview_tabs = ImVec2( panel_x_calc + 20.0f, panel_y_calc + 80.0f );
    ImVec2 const tabs_area_size = ImVec2( 240.0f, 5 * 54.0f );

    bool const is_hovered = ( mouse_pos.x >= m_pos.x - 30.0f && mouse_pos.x <= m_pos.x + m_size.x &&
                               mouse_pos.y >= m_pos.y && mouse_pos.y <= m_pos.y + m_size.y );

    bool const in_dashboard_content = ( mouse_pos.x >= m_pos.x + 250.0f &&
                                        mouse_pos.x <= m_pos.x + m_size.x + 50.0f &&
                                        mouse_pos.y >= m_pos.y + 75.0f &&
                                        mouse_pos.y <= m_pos.y + m_size.y + 150.0f );

    bool const over_interactive = ( ( m_state == menu_state_t::LOGIN ) &&
                                     ( in_rect( mouse_pos, preview_input1, m_input_username.m_size ) ||
                                       in_rect( mouse_pos, preview_input2, m_input_password.m_size ) ||
                                       in_rect( mouse_pos, preview_button, g_button.m_size ) ||
                                       m_input_username.m_is_selecting || m_input_password.m_is_selecting ) ) ||
                                   ( ( m_state == menu_state_t::MAIN_DASHBOARD || m_state == menu_state_t::TRANSITION_TO_MAIN ) &&
                                     ( in_rect( mouse_pos, preview_search, search_size ) ||
                                       in_rect( mouse_pos, preview_tabs, tabs_area_size ) ||
                                       in_dashboard_content ||
                                       g_widgets.is_any_open( ) ||
                                       m_input_search.m_is_selecting || m_input_search.m_focused ) );

    if ( is_hovered && io.MouseClicked[ 0 ] && !over_interactive )
    {
        m_dragging = true;
        m_drag_offset = ImVec2( mouse_pos.x - m_pos.x, mouse_pos.y - m_pos.y );
    }

    if ( m_dragging )
    {
        if ( mouse_down )
        {
            m_pos = ImVec2( mouse_pos.x - m_drag_offset.x, mouse_pos.y - m_drag_offset.y );
        }
        else
        {
            m_dragging = false;
        }
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

    ImU32 const bg_color        = IM_COL32( 0x16, 0x16, 0x16, 255 );
    ImU32 const aa_smooth_color = IM_COL32( 0x16, 0x16, 0x16, 200 );

    float const rounding = 40.0f;
    ImVec2 const p_min = m_pos;
    ImVec2 const p_max = ImVec2( m_pos.x + m_size.x, m_pos.y + m_size.y );

    float const left_fade = 1.0f - cover_alpha;
    int const left_a = ( int )( left_fade * 255.0f );

    // Main rounded background container (#161616) - full window container on all frames
    draw_list->AddRectFilled( p_min, p_max, bg_color, rounding );

    // Render Left Side Branding Panel (#7A72AC), bg.png, and Character (cici.png)
    if ( left_fade > 0.001f )
    {
        ImVec2 const p_left_min = ImVec2( m_pos.x, m_pos.y - 1.0f );
        ImVec2 const p_left_max = ImVec2( m_pos.x + 351.0f, m_pos.y + m_size.y + 1.0f );
        ImDrawFlags const left_rounding_flags = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomLeft;

        draw_list->PushClipRect( p_left_min, p_left_max, true );

        draw_list->AddRectFilled( p_left_min, p_left_max, IM_COL32( 0x7A, 0x72, 0xAC, left_a ), rounding, left_rounding_flags );

        if ( m_bg_texture.m_loaded )
        {
            draw_list->AddImageRounded(
                ( ImTextureID )( intptr_t )m_bg_texture.m_texture_id,
                p_left_min, p_left_max,
                ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 1.0f ),
                IM_COL32( 255, 255, 255, left_a ),
                rounding, left_rounding_flags
            );
        }

        if ( m_cici_texture.m_loaded )
        {
            float const aspect1 = ( float )m_cici_texture.m_width / ( float )m_cici_texture.m_height;
            float const char_h = m_size.y;
            float const char_w = char_h * aspect1;
            float const char_x = m_pos.x + 351.0f - char_w;
            float const char_y = m_pos.y;

            ImVec2 const cici_min = ImVec2( char_x, char_y );
            ImVec2 const cici_max = ImVec2( char_x + char_w, char_y + char_h );

            draw_list->AddImage(
                ( ImTextureID )( intptr_t )m_cici_texture.m_texture_id,
                cici_min, cici_max, ImVec2( 0.0f, 0.0f ),
                ImVec2( 1.0f, 1.0f ), IM_COL32( 255, 255, 255, ( int )( left_fade * 128.0f ) )
            );
        }

        // Render Telegram & Discord brand icons
        ImFont* brand_font = ( g_fonts.m_brand_font_40 != nullptr ) ? g_fonts.m_brand_font_40 : g_fonts.m_icon_font;
        if ( brand_font != nullptr )
        {
            ImVec2 const tg_pos = ImVec2( m_pos.x + 31.0f, m_pos.y + 102.67f );
            ImVec2 const discord_pos = ImVec2( tg_pos.x + 35.0f, m_pos.y + 102.67f );

            bool const is_tg_hovered = ( mouse_pos.x >= tg_pos.x && mouse_pos.x <= tg_pos.x + 26.67f &&
                                         mouse_pos.y >= tg_pos.y && mouse_pos.y <= tg_pos.y + 26.67f );

            bool const is_discord_hovered = ( mouse_pos.x >= discord_pos.x && mouse_pos.x <= discord_pos.x + 26.67f &&
                                              mouse_pos.y >= discord_pos.y && mouse_pos.y <= discord_pos.y + 26.67f );

            m_anim_tg_hover.m_speed = 14.0f;
            m_anim_tg_hover.set( is_tg_hovered ? 1.0f : 0.0f );
            m_anim_tg_hover.update( delta_time );

            m_anim_discord_hover.m_speed = 14.0f;
            m_anim_discord_hover.set( is_discord_hovered ? 1.0f : 0.0f );
            m_anim_discord_hover.update( delta_time );

            float const tg_val = m_anim_tg_hover.m_value;
            float const discord_val = m_anim_discord_hover.m_value;

            ImU32 const col_tg = IM_COL32(
                ( int )( 156.0f + 99.0f * tg_val ),
                ( int )( 148.0f + 107.0f * tg_val ),
                ( int )( 200.0f + 55.0f * tg_val ),
                left_a
            );

            ImU32 const col_discord = IM_COL32(
                ( int )( 156.0f + 99.0f * discord_val ),
                ( int )( 148.0f + 107.0f * discord_val ),
                ( int )( 200.0f + 55.0f * discord_val ),
                left_a
            );

            char const* icon_telegram = "\xef\x8b\x86";
            char const* icon_discord  = "\xef\x8e\x92";

            draw_list->AddText( brand_font, 26.67f, tg_pos, col_tg, icon_telegram );
            draw_list->AddText( brand_font, 26.67f, discord_pos, col_discord, icon_discord );
        }

        // Text "mareland" & "best of best."
        ImFont* calsans_font = ( g_fonts.m_calsans_42 != nullptr ) ? g_fonts.m_calsans_42 : g_fonts.m_medium_32;
        if ( calsans_font != nullptr )
        {
            float const text_x = m_pos.x + 31.0f;
            float const text_y = m_pos.y + 182.0f;
            ImU32 const text_col = IM_COL32( 255, 255, 255, left_a );

            draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x + 0.8f, text_y ), text_col, "mareland" );
            draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x, text_y ), text_col, "mareland" );

            ImU32 const col_subtext = IM_COL32( 0x1B, 0x16, 0x29, left_a );
            float const line2_y = text_y + 30.0f;
            draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x + 0.8f, line2_y ), col_subtext, "best of best." );
            draw_list->AddText( calsans_font, 42.0f, ImVec2( text_x, line2_y ), col_subtext, "best of best." );
        }

        // Logo (logo.png)
        if ( m_logo_texture.m_loaded )
        {
            ImVec2 const pos_logo_min = ImVec2( m_pos.x + 31.0f, m_pos.y + 36.0f );
            ImVec2 const pos_logo_max = ImVec2( pos_logo_min.x + 68.0f, pos_logo_min.y + 44.0f );

            draw_list->AddImage(
                ( ImTextureID )( intptr_t )m_logo_texture.m_texture_id,
                pos_logo_min, pos_logo_max,
                ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 1.0f ),
                IM_COL32( 255, 255, 255, left_a )
            );
        }

        draw_list->PopClipRect( );
    }

    // Render Character Image (assets/ooo.png) in bottom-right corner of second window with 50% opacity and enlarged size
    if ( ease_t > 0.001f && m_ooo_texture.m_loaded )
    {
        float const aspect_ooo = ( float )m_ooo_texture.m_width / ( float )m_ooo_texture.m_height;
        float const char_h = std::clamp( m_size.y * 0.72f, 350.0f, 480.0f );
        float const char_w = char_h * aspect_ooo;

        float const char_x = m_pos.x + m_size.x - char_w;
        float const char_y = m_pos.y + m_size.y - char_h;

        ImVec2 const ooo_min = ImVec2( char_x, char_y );
        ImVec2 const ooo_max = ImVec2( char_x + char_w, char_y + char_h );

        int const ooo_a = ( int )( ease_t * 128.0f );

        draw_list->PushClipRect( m_pos, ImVec2( m_pos.x + m_size.x, m_pos.y + m_size.y ), true );
        draw_list->AddImage(
            ( ImTextureID )( intptr_t )m_ooo_texture.m_texture_id,
            ooo_min, ooo_max,
            ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 1.0f ),
            IM_COL32( 255, 255, 255, ooo_a )
        );
        draw_list->PopClipRect( );
    }
}

void c_menu::draw_foreground( ImFont* font_medium_32, float const delta_time )
{
    if ( !m_open ) { return; }

    float const t_val = m_trans_progress;

    auto ease_quintic = []( float const p ) -> float
    {
        float const clamped_p = std::clamp( p, 0.0f, 1.0f );
        return ( clamped_p < 0.5f ) ? ( 16.0f * clamped_p * clamped_p * clamped_p * clamped_p * clamped_p ) : ( 1.0f - std::pow( -2.0f * clamped_p + 2.0f, 5.0f ) * 0.5f );
    };

    float const ease_t = ease_quintic( t_val );
    float const cover_alpha = std::clamp( ease_t * 2.5f, 0.0f, 1.0f );

    float const panel_w = lerp_f( 0.0f, 280.0f, ease_t );
    float const panel_h = lerp_f( 275.0f, 641.0f, ease_t );
    // Smooth right-to-left entrance and exit slide animation for left overlay panel
    float const panel_x = ( m_pos.x - 30.0f ) + ( 1.0f - ease_t ) * 35.0f;
    float const panel_y = m_pos.y + ( m_size.y - panel_h ) * 0.5f;

    float const tabs_alpha = ease_t;

    // Render Login Inputs Area (Username, Password, Login Arrow Button)
    if ( cover_alpha < 0.999f )
    {
        float const start_x = m_pos.x + 805.0f - m_padding - m_input_username.m_size.x;
        float const start_y = m_pos.y + m_padding + 2.67f;

        ImVec2 const pos_input1 = ImVec2( start_x, start_y );
        ImVec2 const pos_input2 = ImVec2( start_x, pos_input1.y + m_input_username.m_size.y + m_spacing );
        ImVec2 const pos_button = ImVec2( start_x, pos_input2.y + m_input_password.m_size.y + m_spacing );

        ImGuiIO const& io = ImGui::GetIO( );
        ImVec2 const mouse_pos = io.MousePos;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

        draw_list->PushClipRect( m_pos, ImVec2( m_pos.x + m_size.x, m_pos.y + m_size.y ), true );

        // Render Username Input (no eye icon)
        m_input_username.m_bg_color = IM_COL32( 0x1E, 0x1E, 0x1E, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        m_input_username.m_placeholder_color = IM_COL32( 0x50, 0x50, 0x50, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        m_input_username.m_text_color = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        m_input_username.render( pos_input1, "Username", m_username_text, font_medium_32, delta_time, false );

        // Render Password Input (password eye icon)
        m_input_password.m_bg_color = IM_COL32( 0x1E, 0x1E, 0x1E, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        m_input_password.m_placeholder_color = IM_COL32( 0x50, 0x50, 0x50, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        m_input_password.m_text_color = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( ( 1.0f - cover_alpha ) * 255.0f ) );
        m_input_password.render( pos_input2, "Password", m_password_text, font_medium_32, delta_time, true );

        // Render Login Button (Size: 423.33x69.33, Color: #7B73AD, Text: "Login" 21.33px #262626 centered)
        float const btn_w = 423.33f;
        float const btn_h = 69.33f;

        bool const is_btn_hovered = ( mouse_pos.x >= pos_button.x && mouse_pos.x <= pos_button.x + btn_w &&
                                      mouse_pos.y >= pos_button.y && mouse_pos.y <= pos_button.y + btn_h );

        m_anim_login_hover.m_speed = 18.0f;
        m_anim_login_hover.set( is_btn_hovered ? 1.0f : 0.0f );
        m_anim_login_hover.update( delta_time );

        float const btn_h_val = m_anim_login_hover.m_value;
        int const r_bg = ( int )lerp_f( 0x7B, 0x8C, btn_h_val );
        int const g_bg = ( int )lerp_f( 0x73, 0x84, btn_h_val );
        int const b_bg = ( int )lerp_f( 0xAD, 0xBF, btn_h_val );
        int const btn_a = ( int )( ( 1.0f - cover_alpha ) * 255.0f );

        ImU32 const btn_bg_col = IM_COL32( r_bg, g_bg, b_bg, btn_a );
        draw_list->AddRectFilled( pos_button, ImVec2( pos_button.x + btn_w, pos_button.y + btn_h ), btn_bg_col, 36.0f );

        // Text "Login" (Size 21.33f, Color #262626, Centered)
        if ( font_medium_32 != nullptr )
        {
            ImVec2 const txt_sz = font_medium_32->CalcTextSizeA( 21.33f, FLT_MAX, 0.0f, "Login" );
            ImVec2 const txt_pos = ImVec2( pos_button.x + ( btn_w - txt_sz.x ) * 0.5f, pos_button.y + ( btn_h - txt_sz.y ) * 0.5f );
            draw_list->AddText( font_medium_32, 21.33f, txt_pos, IM_COL32( 0x26, 0x26, 0x26, btn_a ), "Login" );
        }

        // Process Click or Enter to trigger morph animation to Main Dashboard
        bool const is_enter_pressed = ImGui::IsKeyPressed( ImGuiKey_Enter ) || ImGui::IsKeyPressed( ImGuiKey_KeypadEnter );
        if ( ( ( is_btn_hovered && io.MouseClicked[ 0 ] ) || is_enter_pressed ) && m_state == menu_state_t::LOGIN )
        {
            m_center_pos = ImVec2( m_pos.x + m_size.x * 0.5f, m_pos.y + m_size.y * 0.5f );
            m_state = menu_state_t::TRANSITION_TO_MAIN;
        }

        // Overlay Cover Mask (#161616) over Right Inputs Area during transition
        if ( cover_alpha > 0.001f )
        {
            int const mask_a = ( int )( 255.0f * cover_alpha );
            draw_list->AddRectFilled(
                ImVec2( m_pos.x + 351.0f, m_pos.y ),
                ImVec2( m_pos.x + m_size.x, m_pos.y + m_size.y ),
                IM_COL32( 0x16, 0x16, 0x16, mask_a ),
                40.0f, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomRight
            );
        }

        draw_list->PopClipRect( );
    }

    // Render Overlay Panel & Navigation Tabs via g_tabs
    if ( tabs_alpha > 0.001f && panel_w > 1.0f )
    {
        ImFont* icon_font = ( g_fonts.m_icon_font != nullptr ) ? g_fonts.m_icon_font : font_medium_32;

        g_tabs.render(
            ImVec2( panel_x, panel_y ),
            panel_w,
            panel_h,
            tabs_alpha,
            delta_time,
            m_state == menu_state_t::MAIN_DASHBOARD,
            font_medium_32,
            icon_font
        );

        ImGuiIO const& io = ImGui::GetIO( );
        ImVec2 const mouse_pos = io.MousePos;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
        int const a_255 = ( int )( 255.0f * tabs_alpha );

        // Draw Back Arrow Button in bottom-right corner of the window
        ImVec2 const back_icon_size = ImVec2( 28.0f, 28.0f );
        ImVec2 const back_icon_pos = ImVec2( m_pos.x + m_size.x - back_icon_size.x - 24.0f, m_pos.y + m_size.y - back_icon_size.y - 24.0f );

        bool const is_back_hovered = ( mouse_pos.x >= back_icon_pos.x && mouse_pos.x <= back_icon_pos.x + back_icon_size.x &&
                                      mouse_pos.y >= back_icon_pos.y && mouse_pos.y <= back_icon_pos.y + back_icon_size.y );

        m_anim_back_hover.m_speed = 18.0f;
        m_anim_back_hover.set( is_back_hovered ? 1.0f : 0.0f );
        m_anim_back_hover.update( delta_time );

        float const back_h_val = m_anim_back_hover.m_value;
        ImU32 const back_icon_col = IM_COL32(
            ( int )lerp_f( 0x8E, 0xFF, back_h_val ),
            ( int )lerp_f( 0x8E, 0xFF, back_h_val ),
            ( int )lerp_f( 0x93, 0xFF, back_h_val ),
            a_255
        );

        // Vector back arrow (<-)
        ImVec2 const b_center = ImVec2( back_icon_pos.x + back_icon_size.x * 0.5f, back_icon_pos.y + back_icon_size.y * 0.5f );
        draw_list->AddLine( ImVec2( b_center.x + 6.0f, b_center.y ), ImVec2( b_center.x - 5.0f, b_center.y ), back_icon_col, 2.4f );
        draw_list->AddLine( ImVec2( b_center.x - 5.0f, b_center.y ), ImVec2( b_center.x - 1.0f, b_center.y - 5.0f ), back_icon_col, 2.4f );
        draw_list->AddLine( ImVec2( b_center.x - 5.0f, b_center.y ), ImVec2( b_center.x - 1.0f, b_center.y + 5.0f ), back_icon_col, 2.4f );

        if ( is_back_hovered && io.MouseClicked[ 0 ] && m_state == menu_state_t::MAIN_DASHBOARD )
        {
            m_center_pos = ImVec2( m_pos.x + m_size.x * 0.5f, m_pos.y + m_size.y * 0.5f );
            m_state = menu_state_t::TRANSITION_TO_LOGIN;
            g_tabs.m_active_tab = -1;
        }

        // Main Dashboard Content Header & Search Box (280x58, #1A1A1A, Search... #4B4B4B with loupe icon)
        if ( tabs_alpha > 0.001f )
        {
            float const search_slide_x = ( 1.0f - ease_t ) * 30.0f;
            float const search_x_calc = m_pos.x + 270.0f + search_slide_x;
            float const search_y_calc = m_pos.y + 11.0f;

            // Push clip rect to strictly constrain all dashboard cards & widgets within the main window boundary!
            draw_list->PushClipRect( ImVec2( m_pos.x + 250.0f, m_pos.y ), ImVec2( m_pos.x + m_size.x, m_pos.y + m_size.y ), true );

            // Search Box (temporarily commented out)
            // m_input_search.m_size = ImVec2( 280.0f, 58.0f );
            // m_input_search.m_bg_color = IM_COL32( 0x1A, 0x1A, 0x1A, a_255 );
            // m_input_search.m_placeholder_color = IM_COL32( 0x4B, 0x4B, 0x4B, a_255 );
            // m_input_search.m_text_color = IM_COL32( 0xE2, 0xE2, 0xE2, a_255 );
            // m_input_search.render( ImVec2( search_x_calc, search_y_calc ), "Search...", m_search_text, font_medium_32, delta_time, false, true );

            // Render active tab & subtab content cards
            if ( g_tabs.m_active_tab >= 0 && g_tabs.m_active_tab < c_tabs::k_tab_count )
            {
                int const active_tab = g_tabs.m_active_tab;
                int const active_sub = g_tabs.m_active_sub_tab[ active_tab ];

                ImVec2 const col1_pos = ImVec2( m_pos.x + 270.0f + search_slide_x, m_pos.y + 85.0f );
                ImVec2 const col2_pos = ImVec2( m_pos.x + 663.0f + search_slide_x, m_pos.y + 85.0f );
                float const col_w = 377.0f;

                if ( active_tab == 0 ) // Combat
                {
                    if ( active_sub == 0 ) // Aim
                    {
                        ImGui::SetCursorScreenPos( ImVec2( col2_pos.x + 18.0f, col2_pos.y + 55.0f ) );
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
}

void c_menu::draw( ImFont* font_medium_32, float const delta_time )
{
    ImGui::SetNextWindowPos( ImVec2( 0.0f, 0.0f ) );
    ImGui::SetNextWindowSize( ImGui::GetIO( ).DisplaySize );
    ImGui::Begin( "##MainWindowContainer", nullptr,
                  ImGuiWindowFlags_NoTitleBar |
                  ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoCollapse |
                  ImGuiWindowFlags_NoScrollbar |
                  ImGuiWindowFlags_NoSavedSettings |
                  ImGuiWindowFlags_NoInputs |
                  ImGuiWindowFlags_NoBackground );

    draw_background( delta_time );
    draw_foreground( font_medium_32, delta_time );

    ImGui::End( );
}
