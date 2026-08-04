#include "input/input.hxx"
#include "fonts/fonts.hxx"
#include "menu/menu.hxx"
#include "blur/blur.hxx"
#include <cmath>
#include <algorithm>
static inline float lerp_f( float const a, float const b, float const t )
{
    return a + ( b - a ) * t;
}

static int get_previous_utf8_char_index( std::string const& str, int const current_idx )
{
    if ( current_idx <= 0 )
    {
        return 0;
    }

    int idx = current_idx - 1;
    while ( idx > 0 && ( str[ idx ] & 0xC0 ) == 0x80 )
    {
        idx--;
    }

    return idx;
}

static int get_next_utf8_char_index( std::string const& str, int const current_idx )
{
    int const len = ( int )str.length( );
    if ( current_idx >= len )
    {
        return len;
    }

    int idx = current_idx + 1;
    while ( idx < len && ( str[ idx ] & 0xC0 ) == 0x80 )
    {
        idx++;
    }

    return idx;
}

static void encode_utf8_char( char* out, unsigned int const c )
{
    if ( c < 0x80 )
    {
        out[ 0 ] = ( char )c;
        out[ 1 ] = '\0';
    }
    else if ( c < 0x800 )
    {
        out[ 0 ] = ( char )( 0xC0 | ( c >> 6 ) );
        out[ 1 ] = ( char )( 0x80 | ( c & 0x3F ) );
        out[ 2 ] = '\0';
    }
    else if ( c < 0x10000 )
    {
        out[ 0 ] = ( char )( 0xE0 | ( c >> 12 ) );
        out[ 1 ] = ( char )( 0x80 | ( ( c >> 6 ) & 0x3F ) );
        out[ 2 ] = ( char )( 0x80 | ( c & 0x3F ) );
        out[ 3 ] = '\0';
    }
    else if ( c < 0x110000 )
    {
        out[ 0 ] = ( char )( 0xF0 | ( c >> 18 ) );
        out[ 1 ] = ( char )( 0x80 | ( ( c >> 12 ) & 0x3F ) );
        out[ 2 ] = ( char )( 0x80 | ( ( c >> 6 ) & 0x3F ) );
        out[ 3 ] = ( char )( 0x80 | ( c & 0x3F ) );
        out[ 4 ] = '\0';
    }
}

bool c_input::render( ImVec2 const& pos, char const* placeholder, std::string& text, ImFont* font_32, float const delta_time, bool const is_password, bool const is_search )
{
    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
    ImGuiIO& io = ImGui::GetIO( );

    ImVec2 const p_min = pos;
    ImVec2 const p_max = ImVec2( pos.x + m_size.x, pos.y + m_size.y );

    ImVec2 const mouse_pos = io.MousePos;
    bool const is_hovered = ( mouse_pos.x >= p_min.x && mouse_pos.x <= p_max.x &&
                              mouse_pos.y >= p_min.y && mouse_pos.y <= p_max.y );

    float const pad_left = is_search ? 44.0f : m_pad_x;
    float const right_pad = ( is_password || is_search ) ? 20.0f : m_pad_x;

    float const eye_center_x = pos.x + m_size.x - m_pad_x;
    float const eye_center_y = pos.y + m_size.y * 0.5f;

    ImVec2 const eye_min = ImVec2( eye_center_x - 14.67f, eye_center_y - 14.67f );
    ImVec2 const eye_max = ImVec2( eye_center_x + 14.67f, eye_center_y + 14.67f );

    bool const is_eye_hovered = is_password && ( mouse_pos.x >= eye_min.x && mouse_pos.x <= eye_max.x &&
                                                 mouse_pos.y >= eye_min.y && mouse_pos.y <= eye_max.y );

    if ( is_eye_hovered && io.MouseClicked[ 0 ] )
    {
        m_show_password = !m_show_password;
    }

    float const text_start_x_base = pos.x + pad_left;
    float const max_visible_w = m_size.x - pad_left - right_pad;

    float const font_sz = is_search ? 19.0f : ( ( m_size.y <= 60.0f ) ? 18.0f : 23.33f );
    ImVec2 text_sample_size = ImVec2( 0.0f, font_sz );
    if ( font_32 != nullptr )
    {
        text_sample_size = font_32->CalcTextSizeA( font_sz, FLT_MAX, 0.0f, "Ay" );
    }

    float const text_start_y = pos.y + ( m_size.y - text_sample_size.y ) * 0.5f;

    std::string display_text = text;
    if ( is_password && !m_show_password && !text.empty( ) )
    {
        display_text = std::string( text.length( ), '*' );
    }

    auto get_offset_at_index = [ font_32, font_sz, &display_text ]( int const idx ) -> float
    {
        if ( idx <= 0 || display_text.empty( ) )
        {
            return 0.0f;
        }

        int const clamped_idx = std::clamp( idx, 0, ( int )display_text.length( ) );
        std::string const sub = display_text.substr( 0, clamped_idx );

        if ( font_32 != nullptr )
        {
            return font_32->CalcTextSizeA( font_sz, FLT_MAX, 0.0f, sub.c_str( ) ).x;
        }
        
        return ImGui::CalcTextSize( sub.c_str( ) ).x;
    };

    auto get_index_at_offset = [ font_32, font_sz, &display_text ]( float const rel_x ) -> int
    {
        if ( display_text.empty( ) || rel_x <= 0.0f )
        {
            return 0;
        }

        int best_idx = 0;
        float min_dist = FLT_MAX;

        for ( int i = 0; i <= ( int )display_text.length( ); )
        {
            std::string const sub = display_text.substr( 0, i );
            float const w = ( font_32 != nullptr ) ? font_32->CalcTextSizeA( font_sz, FLT_MAX, 0.0f, sub.c_str( ) ).x
                                                    : ImGui::CalcTextSize( sub.c_str( ) ).x;

            float const dist = std::abs( rel_x - w );
            if ( dist < min_dist )
            {
                min_dist = dist;
                best_idx = i;
            }

            int const next_i = get_next_utf8_char_index( display_text, i );
            if ( next_i == i )
            {
                break;
            }
            i = next_i;
        }

        return best_idx;
    };

    float const total_text_w = get_offset_at_index( ( int )display_text.length( ) );
    float const cursor_local_x = get_offset_at_index( m_cursor_index );

    float current_scroll = m_anim_scroll_x.m_value;

    if ( cursor_local_x - current_scroll > max_visible_w )
    {
        current_scroll = cursor_local_x - max_visible_w;
    }
    else if ( cursor_local_x < current_scroll )
    {
        current_scroll = cursor_local_x;
    }

    float const max_scroll = std::max( 0.0f, total_text_w - max_visible_w );
    current_scroll = std::clamp( current_scroll, 0.0f, max_scroll );

    m_anim_scroll_x.m_speed = 16.0f;
    m_anim_scroll_x.set( current_scroll );
    m_anim_scroll_x.update( delta_time );

    float const scroll_x = m_anim_scroll_x.m_value;
    float const text_start_x = text_start_x_base - scroll_x;

    if ( g_menu.m_dragging )
    {
        m_is_selecting = false;
    }

    if ( io.MouseClicked[ 0 ] && !g_menu.m_dragging )
    {
        m_focused = is_hovered && !is_eye_hovered;
        if ( m_focused )
        {
            float const click_rel_x = mouse_pos.x - text_start_x;
            m_cursor_index = get_index_at_offset( click_rel_x );
            m_select_index = m_cursor_index;
            m_is_selecting = true;
            m_blink_timer = 0.0f;
        }
    }

    if ( m_is_selecting && io.MouseDown[ 0 ] && m_focused && !g_menu.m_dragging )
    {
        float const current_rel_x = mouse_pos.x - text_start_x;
        m_cursor_index = get_index_at_offset( current_rel_x );
    }

    if ( !io.MouseDown[ 0 ] || g_menu.m_dragging )
    {
        m_is_selecting = false;
    }

    m_cursor_index = std::clamp( m_cursor_index, 0, ( int )text.length( ) );
    m_select_index = std::clamp( m_select_index, 0, ( int )text.length( ) );

    if ( m_focused )
    {
        bool const shift_held = io.KeyShift;
        int const sel_min = std::min( m_select_index, m_cursor_index );
        int const sel_max = std::max( m_select_index, m_cursor_index );
        bool const has_selection = ( sel_min != sel_max );

        auto delete_selection = [ & ]( )
        {
            if ( has_selection )
            {
                text.erase( sel_min, sel_max - sel_min );
                m_cursor_index = sel_min;
                m_select_index = sel_min;
                return true;
            }
            return false;
        };

        if ( io.KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_A ) )
        {
            m_select_index = 0;
            m_cursor_index = ( int )text.length( );
        }

        if ( io.KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_C ) && has_selection )
        {
            std::string const sel_str = text.substr( sel_min, sel_max - sel_min );
            ImGui::SetClipboardText( sel_str.c_str( ) );
        }

        if ( io.KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_X ) && has_selection )
        {
            std::string const sel_str = text.substr( sel_min, sel_max - sel_min );
            ImGui::SetClipboardText( sel_str.c_str( ) );
            delete_selection( );
        }

        if ( io.KeyCtrl && ImGui::IsKeyPressed( ImGuiKey_V ) )
        {
            char const* clip = ImGui::GetClipboardText( );
            if ( clip != nullptr )
            {
                delete_selection( );
                std::string const clip_str( clip );
                text.insert( m_cursor_index, clip_str );
                m_cursor_index += ( int )clip_str.length( );
                m_select_index = m_cursor_index;
            }
        }

        if ( ImGui::IsKeyPressed( ImGuiKey_LeftArrow, true ) )
        {
            if ( m_cursor_index > 0 )
            {
                m_cursor_index = get_previous_utf8_char_index( text, m_cursor_index );
                if ( !shift_held )
                {
                    m_select_index = m_cursor_index;
                }
            }
            m_blink_timer = 0.0f;
        }

        if ( ImGui::IsKeyPressed( ImGuiKey_RightArrow, true ) )
        {
            if ( m_cursor_index < ( int )text.length( ) )
            {
                m_cursor_index = get_next_utf8_char_index( text, m_cursor_index );
                if ( !shift_held )
                {
                    m_select_index = m_cursor_index;
                }
            }
            m_blink_timer = 0.0f;
        }

        if ( ImGui::IsKeyPressed( ImGuiKey_Backspace, true ) )
        {
            if ( !delete_selection( ) && m_cursor_index > 0 )
            {
                int const prev_idx = get_previous_utf8_char_index( text, m_cursor_index );
                text.erase( prev_idx, m_cursor_index - prev_idx );
                m_cursor_index = prev_idx;
                m_select_index = m_cursor_index;
            }
            m_blink_timer = 0.0f;
        }

        if ( ImGui::IsKeyPressed( ImGuiKey_Delete, true ) )
        {
            if ( !delete_selection( ) && m_cursor_index < ( int )text.length( ) )
            {
                int const next_idx = get_next_utf8_char_index( text, m_cursor_index );
                text.erase( m_cursor_index, next_idx - m_cursor_index );
            }
            m_blink_timer = 0.0f;
        }

        // UTF-8 Character Typing (Supports Cyrillic / Russian)
        for ( int n = 0; n < io.InputQueueCharacters.Size; n++ )
        {
            unsigned int c = ( unsigned int )io.InputQueueCharacters[ n ];

            // Windows CP1251 Cyrillic Mapping Fix
            if ( c >= 0xC0 && c <= 0xFF )
            {
                c = 0x0410 + ( c - 0xC0 );
            }
            else if ( c == 0xA8 )
            {
                c = 0x0401;
            }
            else if ( c == 0xB8 )
            {
                c = 0x0451;
            }

            if ( c >= 32 && c != 127 && text.length( ) < 256 )
            {
                delete_selection( );
                
                char utf8_buf[ 5 ] = { 0 };
                encode_utf8_char( utf8_buf, c );
                
                text.insert( m_cursor_index, utf8_buf );
                m_cursor_index += ( int )strlen( utf8_buf );
                m_select_index = m_cursor_index;
                m_blink_timer = 0.0f;
            }
        }
    }

    m_cursor_index = std::clamp( m_cursor_index, 0, ( int )text.length( ) );
    m_select_index = std::clamp( m_select_index, 0, ( int )text.length( ) );

    float const target_cursor_offset = get_offset_at_index( m_cursor_index );

    int const sel_min_idx = std::min( m_select_index, m_cursor_index );
    int const sel_max_idx = std::max( m_select_index, m_cursor_index );

    float const target_sel_start = get_offset_at_index( sel_min_idx );
    float const target_sel_end = get_offset_at_index( sel_max_idx );

    // Ultra-smooth caret animation (speed 24.0f for fluid typing across all menus)
    m_anim_cursor_offset.m_speed = 24.0f;
    m_anim_cursor_offset.set( target_cursor_offset );
    m_anim_cursor_offset.update( delta_time );

    m_anim_select_start.m_speed = 24.0f;
    m_anim_select_start.set( target_sel_start );
    m_anim_select_start.update( delta_time );

    m_anim_select_end.m_speed = 24.0f;
    m_anim_select_end.set( target_sel_end );
    m_anim_select_end.update( delta_time );

    float const target_fade_left = ( scroll_x > 2.0f ) ? 1.0f : 0.0f;
    float const target_fade_right = ( ( total_text_w - scroll_x ) > max_visible_w + 1.0f ) ? 1.0f : 0.0f;

    m_anim_fade_left.m_speed = 14.0f;
    m_anim_fade_left.set( target_fade_left );
    m_anim_fade_left.update( delta_time );

    m_anim_fade_right.m_speed = 14.0f;
    m_anim_fade_right.set( target_fade_right );
    m_anim_fade_right.update( delta_time );

    m_anim_hover.m_speed = 14.0f;
    m_anim_hover.set( is_hovered ? 1.0f : 0.0f );
    m_anim_hover.update( delta_time );

    m_anim_focus.m_speed = 14.0f;
    m_anim_focus.set( m_focused ? 1.0f : 0.0f );
    m_anim_focus.update( delta_time );

    float const rounding = is_search ? 20.0f : 36.0f;

    // Background Container
    draw_list->AddRectFilled( p_min, p_max, m_bg_color, rounding );

    float const focus_val = m_anim_focus.m_value;
    float const hover_val = m_anim_hover.m_value;
    float const border_glow = std::clamp( focus_val * 1.0f + ( 1.0f - focus_val ) * hover_val * 0.50f, 0.0f, 1.0f );

    if ( border_glow > 0.001f )
    {
        int const base_a = ( int )( ( m_bg_color >> 24 ) & 0xFF );
        int const border_a = ( int )( border_glow * ( focus_val > 0.001f ? 150.0f : 75.0f ) * ( base_a / 255.0f ) );
        draw_list->AddRect( p_min, p_max, IM_COL32( 0x36, 0x36, 0x36, border_a ), rounding, 0, 1.2f );
    }

    // Draw Crisp Vector Loupe (Magnifying Glass) Icon starting at 20px from left edge
    if ( is_search )
    {
        float const loupe_x = pos.x + 20.0f;
        ImVec2 const center = ImVec2( loupe_x + 5.5f, pos.y + m_size.y * 0.5f - 1.0f );
        ImU32 const icon_col = m_placeholder_color;

        // Ultra crisp 32-segment antialiased vector circle & handle
        draw_list->AddCircle( ImVec2( center.x - 1.5f, center.y - 1.5f ), 5.5f, icon_col, 32, 1.8f );
        draw_list->AddLine( ImVec2( center.x + 2.5f, center.y + 2.5f ), ImVec2( center.x + 7.5f, center.y + 7.5f ), icon_col, 2.0f );
    }

    ImVec2 const clip_min = ImVec2( pos.x + pad_left, pos.y );
    ImVec2 const clip_max = ImVec2( pos.x + m_size.x - right_pad, pos.y + m_size.y );

    draw_list->PushClipRect( clip_min, clip_max, true );

    // Smooth Selection Highlight Box
    if ( m_anim_select_end.m_value - m_anim_select_start.m_value > 0.5f )
    {
        ImVec2 const sel_p1 = ImVec2( text_start_x + m_anim_select_start.m_value, text_start_y - 2.0f );
        ImVec2 const sel_p2 = ImVec2( text_start_x + m_anim_select_end.m_value, text_start_y + text_sample_size.y + 2.0f );
        draw_list->AddRectFilled( sel_p1, sel_p2, m_selection_color, 4.0f );
    }

    // Eye switch animation for password text visibility (speed 14.0f)
    m_anim_eye_switch.m_speed = 14.0f;
    m_anim_eye_switch.set( ( is_password && m_show_password ) ? 1.0f : 0.0f );
    m_anim_eye_switch.update( delta_time );
    float const eye_t = is_password ? m_anim_eye_switch.m_value : 1.0f;

    // Render Text or Placeholder with smooth cross-fade for eye icon toggle
    if ( text.empty( ) && placeholder != nullptr )
    {
        if ( font_32 != nullptr )
        {
            draw_list->AddText( font_32, font_sz, ImVec2( text_start_x, text_start_y ), m_placeholder_color, placeholder );
        }
        else
        {
            draw_list->AddText( ImVec2( text_start_x, text_start_y ), m_placeholder_color, placeholder );
        }
    }
    else if ( !text.empty( ) )
    {
        if ( is_password )
        {
            std::string const masked_text = std::string( text.length( ), '*' );
            float const fade_masked = 1.0f - eye_t;
            float const fade_plain  = eye_t;

            // Masked asterisks fade out
            if ( fade_masked > 0.001f )
            {
                ImU32 const col_masked = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( fade_masked * 255.0f ) );
                if ( font_32 != nullptr )
                {
                    draw_list->AddText( font_32, 23.33f, ImVec2( text_start_x, text_start_y ), col_masked, masked_text.c_str( ) );
                }
                else
                {
                    draw_list->AddText( ImVec2( text_start_x, text_start_y ), col_masked, masked_text.c_str( ) );
                }
            }

            // Plain text password fade in
            if ( fade_plain > 0.001f )
            {
                ImU32 const col_plain = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( fade_plain * 255.0f ) );
                if ( font_32 != nullptr )
                {
                    draw_list->AddText( font_32, 23.33f, ImVec2( text_start_x, text_start_y ), col_plain, text.c_str( ) );
                }
                else
                {
                    draw_list->AddText( ImVec2( text_start_x, text_start_y ), col_plain, text.c_str( ) );
                }
            }
        }
        else
        {
            if ( font_32 != nullptr )
            {
                draw_list->AddText( font_32, font_sz, ImVec2( text_start_x, text_start_y ), m_text_color, text.c_str( ) );
            }
            else
            {
                draw_list->AddText( ImVec2( text_start_x, text_start_y ), m_text_color, text.c_str( ) );
            }
        }
    }

    // Caret Alpha & Blink
    m_blink_timer += delta_time;
    float const target_alpha = m_focused ? ( 0.5f + 0.5f * sinf( m_blink_timer * 8.0f ) ) : 0.0f;

    m_anim_cursor_alpha.m_speed = 14.0f;
    m_anim_cursor_alpha.set( target_alpha );
    m_anim_cursor_alpha.update( delta_time );

    // Draw Caret Line
    if ( m_anim_cursor_alpha.m_value > 0.01f )
    {
        float const caret_x = text_start_x + m_anim_cursor_offset.m_value;
        float const caret_y1 = text_start_y;
        float const caret_y2 = text_start_y + text_sample_size.y;
        ImU32 const caret_color = IM_COL32( 0xE2, 0xE2, 0xE2, ( int )( m_anim_cursor_alpha.m_value * 255.0f ) );

        draw_list->AddLine( ImVec2( caret_x, caret_y1 ), ImVec2( caret_x, caret_y2 ), caret_color, 2.0f );
    }

    draw_list->PopClipRect( );

    float const fade_w = 30.0f;

    ImU32 const bg_r = ( m_bg_color >> 0 ) & 0xFF;
    ImU32 const bg_g = ( m_bg_color >> 8 ) & 0xFF;
    ImU32 const bg_b = ( m_bg_color >> 16 ) & 0xFF;
    ImU32 const bg_a = ( m_bg_color >> 24 ) & 0xFF;

    // Right Fade Out
    if ( m_anim_fade_right.m_value > 0.01f )
    {
        ImVec2 const fade_p1 = ImVec2( pos.x + m_size.x - right_pad - fade_w, pos.y + 6.0f );
        ImVec2 const fade_p2 = ImVec2( pos.x + m_size.x - right_pad, pos.y + m_size.y - 6.0f );

        ImU32 const c_transparent = IM_COL32( bg_r, bg_g, bg_b, 0 );
        ImU32 const c_opaque = IM_COL32( bg_r, bg_g, bg_b, ( int )( m_anim_fade_right.m_value * ( bg_a / 255.0f ) * 255.0f ) );

        draw_list->AddRectFilledMultiColor( fade_p1, fade_p2, c_transparent, c_opaque, c_opaque, c_transparent );
    }

    // Left Fade Out
    if ( m_anim_fade_left.m_value > 0.01f )
    {
        ImVec2 const fade_p1 = ImVec2( pos.x + m_pad_x, pos.y + 6.0f );
        ImVec2 const fade_p2 = ImVec2( pos.x + m_pad_x + fade_w, pos.y + m_size.y - 6.0f );

        ImU32 const c_opaque = IM_COL32( bg_r, bg_g, bg_b, ( int )( m_anim_fade_left.m_value * ( bg_a / 255.0f ) * 255.0f ) );
        ImU32 const c_transparent = IM_COL32( bg_r, bg_g, bg_b, 0 );

        draw_list->AddRectFilledMultiColor( fade_p1, fade_p2, c_opaque, c_transparent, c_transparent, c_opaque );
    }

    // Draw FontAwesome Password Eye Toggle Icon with Smooth Scale & Color Click Animation
    if ( is_password )
    {
        m_anim_eye_hover.m_speed = 16.0f;
        m_anim_eye_hover.set( is_eye_hovered ? 1.0f : 0.0f );
        m_anim_eye_hover.update( delta_time );

        m_anim_eye_switch.m_speed = 8.0f;
        m_anim_eye_switch.set( m_show_password ? 1.0f : 0.0f );
        m_anim_eye_switch.update( delta_time );

        float const hover_val  = m_anim_eye_hover.m_value;
        float const switch_val = m_anim_eye_switch.m_value;

        char const* icon_eye_slash = "\xef\x81\xb0";
        char const* icon_eye       = "\xef\x81\xae";

        ImFont* icon_font = ( g_fonts.m_icon_font != nullptr ) ? g_fonts.m_icon_font : font_32;

        if ( icon_font != nullptr )
        {
            // Steady 14.67px size (no scale enlargement on hover, only subtle color highlight)
            float const icon_size = 14.67f;

            // Eye Slash (Hidden icon) - Smooth opacity fade out
            float const alpha_slash = ( 1.0f - switch_val );
            if ( alpha_slash > 0.001f )
            {
                int const r_slash = ( int )lerp_f( 0x60, 0xA0, hover_val );
                int const g_slash = ( int )lerp_f( 0x60, 0xA0, hover_val );
                int const b_slash = ( int )lerp_f( 0x68, 0xB0, hover_val );
                ImU32 const color_slash = IM_COL32( r_slash, g_slash, b_slash, ( int )( alpha_slash * 200.0f ) );

                ImVec2 const sz = icon_font->CalcTextSizeA( icon_size, FLT_MAX, 0.0f, icon_eye_slash );
                ImVec2 const pos_icon = ImVec2( eye_center_x - sz.x * 0.5f, eye_center_y - sz.y * 0.5f );
                draw_list->AddText( icon_font, icon_size, pos_icon, color_slash, icon_eye_slash );
            }

            // Eye (Visible icon) - Smooth opacity fade in
            float const alpha_eye = switch_val;
            if ( alpha_eye > 0.001f )
            {
                int const r_eye = ( int )lerp_f( 0x7A, 0x9B, hover_val );
                int const g_eye = ( int )lerp_f( 0x72, 0x91, hover_val );
                int const b_eye = ( int )lerp_f( 0xAC, 0xD4, hover_val );
                ImU32 const color_eye = IM_COL32( r_eye, g_eye, b_eye, ( int )( alpha_eye * 255.0f ) );

                ImVec2 const sz = icon_font->CalcTextSizeA( icon_size, FLT_MAX, 0.0f, icon_eye );
                ImVec2 const pos_icon = ImVec2( eye_center_x - sz.x * 0.5f, eye_center_y - sz.y * 0.5f );
                draw_list->AddText( icon_font, icon_size, pos_icon, color_eye, icon_eye );
            }
        }
    }

    return m_focused;
}
