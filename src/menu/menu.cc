#include "menu/menu.hxx"
#include "menu/menu_login.hxx"
#include "menu/menu_content.hxx"
#include "fonts/fonts.hxx"
#include "widgets/widgets.hxx"

#include <algorithm>
#include <cmath>

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

    float const collapse_t = g_tabs.m_anim_collapse.m_value;
    float const collapse_shift_x = 197.0f * collapse_t * ease_t;
    float const current_main_w = 1101.0f - collapse_shift_x;

    m_size.x = lerp_f( m_login_size.x, current_main_w, ease_t );
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

    // Effective rendering position (contracts from left side)
    ImVec2 const effective_pos = ImVec2( m_pos.x + collapse_shift_x, m_pos.y );

    // Window dragging logic
    float const input_x = effective_pos.x + 805.0f - m_padding - m_input_username.m_size.x;
    float const input_y = effective_pos.y + m_padding + 2.67f;

    ImVec2 const preview_input1 = ImVec2( input_x, input_y );
    ImVec2 const preview_input2 = ImVec2( input_x, preview_input1.y + m_input_username.m_size.y + m_spacing );
    ImVec2 const preview_button = ImVec2( input_x, preview_input2.y + m_input_password.m_size.y + m_spacing );

    float const panel_x_calc = effective_pos.x - 30.0f * ease_t;
    float const panel_y_calc = effective_pos.y + ( m_size.y - 641.0f ) * 0.5f;

    float const search_x_calc = panel_x_calc + 280.0f + 21.0f;
    float const search_y_calc = effective_pos.y + 11.0f;
    ImVec2 const preview_search = ImVec2( search_x_calc, search_y_calc );
    ImVec2 const search_size = ImVec2( 280.0f, 58.0f );

    ImVec2 const preview_tabs = ImVec2( panel_x_calc + 20.0f, panel_y_calc + 80.0f );
    ImVec2 const tabs_area_size = ImVec2( 240.0f, 5 * 54.0f );

    bool const is_hovered = ( mouse_pos.x >= effective_pos.x - 30.0f && mouse_pos.x <= effective_pos.x + m_size.x &&
                               mouse_pos.y >= effective_pos.y && mouse_pos.y <= effective_pos.y + m_size.y );

    bool const in_dashboard_content = ( mouse_pos.x >= effective_pos.x + 250.0f &&
                                         mouse_pos.x <= effective_pos.x + m_size.x + 50.0f &&
                                         mouse_pos.y >= effective_pos.y + 75.0f &&
                                         mouse_pos.y <= effective_pos.y + m_size.y + 150.0f );

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
        m_drag_offset = ImVec2( mouse_pos.x - effective_pos.x, mouse_pos.y - effective_pos.y );
    }

    if ( m_dragging )
    {
        if ( mouse_down )
        {
            m_pos = ImVec2( mouse_pos.x - m_drag_offset.x - collapse_shift_x, mouse_pos.y - m_drag_offset.y );
        }
        else
        {
            m_dragging = false;
        }
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
    ImU32 const bg_color = IM_COL32( 0x16, 0x16, 0x16, 255 );
    float const rounding = 40.0f;

    // Main rounded background container (#161616)
    draw_list->AddRectFilled( effective_pos, ImVec2( effective_pos.x + m_size.x, effective_pos.y + m_size.y ), bg_color, rounding );

    // Delegate left branding panel rendering
    menu_login::render_left_panel( effective_pos, m_size, ease_t, m_bg_texture, m_cici_texture, m_anim_tg_hover, m_anim_discord_hover, delta_time );
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
    float const collapse_val = g_tabs.m_anim_collapse.m_value;
    float const collapse_shift_x = 197.0f * collapse_val * ease_t;
    ImVec2 const effective_pos = ImVec2( m_pos.x + collapse_shift_x, m_pos.y );

    float const panel_w = lerp_f( 0.0f, 275.0f, ease_t );
    float const panel_h = lerp_f( 275.0f, 641.0f, ease_t );
    float const panel_x = effective_pos.x + ( 1.0f - ease_t ) * 35.0f;
    float const panel_y = effective_pos.y + ( m_size.y - panel_h ) * 0.5f;

    float const tabs_alpha = ease_t;

    // Render Logo with smooth position slide & gradient color transition
    if ( m_logo_texture.m_loaded )
    {
        float const curr_panel_w = g_tabs.m_current_panel_w;

        float const logo_w = lerp_f( 68.0f, 42.0f, collapse_val );
        float const logo_h = lerp_f( 44.0f, 27.0f, collapse_val );

        float const start_logo_x = effective_pos.x + 31.0f;
        float const start_logo_y = effective_pos.y + 41.0f;

        float const end_logo_x = panel_x + ( curr_panel_w - logo_w ) * 0.5f;
        float const end_logo_y = effective_pos.y + lerp_f( 37.0f, 28.0f, collapse_val );

        float const curr_logo_x = lerp_f( start_logo_x, end_logo_x, ease_t );
        float const curr_logo_y = lerp_f( start_logo_y, end_logo_y, ease_t );

        ImVec2 const pos_logo_min = ImVec2( curr_logo_x, curr_logo_y );
        ImVec2 const pos_logo_max = ImVec2( pos_logo_min.x + logo_w, pos_logo_min.y + logo_h );

        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

        int const logo_a = 255;

        float const r_left = lerp_f( 27.0f, 255.0f, ease_t );
        float const g_left = lerp_f( 22.0f, 255.0f, ease_t );
        float const b_left = lerp_f( 41.0f, 255.0f, ease_t );
        ImU32 const col_left = IM_COL32( ( int )r_left, ( int )g_left, ( int )b_left, logo_a );

        float const r_right = lerp_f( 27.0f, 158.0f, ease_t );
        float const g_right = lerp_f( 22.0f, 149.0f, ease_t );
        float const b_right = lerp_f( 27.0f, 217.0f, ease_t );
        ImU32 const col_right = IM_COL32( ( int )r_right, ( int )g_right, ( int )b_right, logo_a );

        draw_list->PushTextureID( ( ImTextureID )( intptr_t )m_logo_texture.m_texture_id );
        draw_list->PrimReserve( 6, 4 );
        draw_list->PrimWriteIdx( ( ImDrawIdx )draw_list->_VtxCurrentIdx );
        draw_list->PrimWriteIdx( ( ImDrawIdx )( draw_list->_VtxCurrentIdx + 1 ) );
        draw_list->PrimWriteIdx( ( ImDrawIdx )( draw_list->_VtxCurrentIdx + 2 ) );
        draw_list->PrimWriteIdx( ( ImDrawIdx )draw_list->_VtxCurrentIdx );
        draw_list->PrimWriteIdx( ( ImDrawIdx )( draw_list->_VtxCurrentIdx + 2 ) );
        draw_list->PrimWriteIdx( ( ImDrawIdx )( draw_list->_VtxCurrentIdx + 3 ) );

        draw_list->PrimWriteVtx( pos_logo_min, ImVec2( 0.0f, 0.0f ), col_left );
        draw_list->PrimWriteVtx( ImVec2( pos_logo_max.x, pos_logo_min.y ), ImVec2( 1.0f, 0.0f ), col_right );
        draw_list->PrimWriteVtx( pos_logo_max, ImVec2( 1.0f, 1.0f ), col_right );
        draw_list->PrimWriteVtx( ImVec2( pos_logo_min.x, pos_logo_max.y ), ImVec2( 0.0f, 1.0f ), col_left );
        draw_list->PopTextureID( );
    }

    // Delegate Login Inputs Area rendering
    menu_login::render_inputs( this, ease_t, font_medium_32, delta_time );

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
            icon_font,
            &m_logo_texture
        );

        ImGuiIO const& io = ImGui::GetIO( );
        ImVec2 const mouse_pos = io.MousePos;
        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
        int const a_255 = ( int )( 255.0f * tabs_alpha );

        // Delegate Main Dashboard Content Area rendering
        menu_content::render_dashboard_content( this, tabs_alpha, ease_t, font_medium_32, delta_time );

        // Render Character Image (assets/ooo.png) in bottom-right corner ON TOP of dashboard content background
        if ( ease_t > 0.001f && m_ooo_texture.m_loaded )
        {
            float const aspect_ooo = ( float )m_ooo_texture.m_width / ( float )m_ooo_texture.m_height;
            float const char_h = std::clamp( m_size.y * 0.72f, 350.0f, 480.0f );
            float const char_w = char_h * aspect_ooo;

            float const char_x = effective_pos.x + m_size.x - char_w;
            float const char_y = effective_pos.y + m_size.y - char_h;

            ImVec2 const ooo_min = ImVec2( char_x, char_y );
            ImVec2 const ooo_max = ImVec2( char_x + char_w, char_y + char_h );

            int const ooo_a = ( int )( ease_t * 128.0f );

            draw_list->PushClipRect( effective_pos, ImVec2( effective_pos.x + m_size.x, effective_pos.y + m_size.y ), true );
            draw_list->AddImage(
                ( ImTextureID )( intptr_t )m_ooo_texture.m_texture_id,
                ooo_min, ooo_max,
                ImVec2( 0.0f, 0.0f ), ImVec2( 1.0f, 1.0f ),
                IM_COL32( 255, 255, 255, ooo_a )
            );
            draw_list->PopClipRect( );
        }

        // Draw Back Arrow Button in bottom-right corner of the window (rendered in foreground above all content)
        ImVec2 const back_icon_size = ImVec2( 32.0f, 32.0f );
        ImVec2 const back_icon_pos = ImVec2( effective_pos.x + m_size.x - back_icon_size.x - 24.0f, effective_pos.y + m_size.y - back_icon_size.y - 24.0f );

        bool const is_back_hovered = ( mouse_pos.x >= back_icon_pos.x && mouse_pos.x <= back_icon_pos.x + back_icon_size.x &&
                                      mouse_pos.y >= back_icon_pos.y && mouse_pos.y <= back_icon_pos.y + back_icon_size.y );

        m_anim_back_hover.m_speed = 18.0f;
        m_anim_back_hover.set( is_back_hovered ? 1.0f : 0.0f );
        m_anim_back_hover.update( delta_time );

        float const back_h_val = m_anim_back_hover.m_value;
        int const b_bg_a = ( int )( lerp_f( 220.0f, 255.0f, back_h_val ) * tabs_alpha );
        int const b_border_a = ( int )( lerp_f( 60.0f, 200.0f, back_h_val ) * tabs_alpha );

        draw_list->AddRectFilled( back_icon_pos, ImVec2( back_icon_pos.x + back_icon_size.x, back_icon_pos.y + back_icon_size.y ), IM_COL32( 26, 24, 34, b_bg_a ), 16.0f );
        draw_list->AddRect( back_icon_pos, ImVec2( back_icon_pos.x + back_icon_size.x, back_icon_pos.y + back_icon_size.y ), IM_COL32( 158, 149, 217, b_border_a ), 16.0f, 0, 1.2f );

        ImU32 const back_icon_col = IM_COL32(
            ( int )lerp_f( 0x9E, 0xFF, back_h_val ),
            ( int )lerp_f( 0x95, 0xFF, back_h_val ),
            ( int )lerp_f( 0xD9, 0xFF, back_h_val ),
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
