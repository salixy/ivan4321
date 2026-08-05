#include "tabs/tabs.hxx"
#include "motion/motion.hxx"
#include "texture/texture.hxx"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <string>

c_tabs g_tabs;

static inline float lerp_f( float a, float b, float t )
{
    return a + ( b - a ) * t;
}

static std::string to_upper( char const* str )
{
    std::string res;
    if ( str == nullptr ) return res;
    for ( char const* p = str; *p != '\0'; ++p )
    {
        res += ( char )std::toupper( ( unsigned char )*p );
    }
    return res;
}

void c_tabs::render( ImVec2 const& panel_pos, float const panel_w, float const panel_h, float const alpha, float const delta_time, bool const can_interact, ImFont* font_medium_32, ImFont* icon_font, c_texture const* logo_texture )
{
    if ( alpha <= 0.001f || panel_w <= 1.0f )
    {
        return;
    }

    ImGuiIO const& io = ImGui::GetIO( );
    ImVec2 const mouse_pos = io.MousePos;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
    int const a_255 = ( int )( 255.0f * alpha );

    ImVec2 const p_panel_min = panel_pos;
    ImVec2 const p_panel_max = ImVec2( panel_pos.x + panel_w, panel_pos.y + panel_h );

    draw_list->PushClipRect( p_panel_min, p_panel_max, true );

    struct tab_child_t
    {
        char const* m_name;
        char const* m_icon;
    };

    struct category_t
    {
        char const* m_name;
        int         m_item_count;
        tab_child_t m_items[ 4 ];
    };

    category_t const categories[ k_tab_count ] = {
        { "Combat",   3, { { "Aim", "\xef\x84\xb5" }, { "Triggerbot", "\xef\x80\x91" }, { "Accuracy", "\xef\x87\x9e" } } },
        { "Visuals",  3, { { "ESP", "\xef\x81\xae" }, { "Effects", "\xef\x81\x82" }, { "Settings", "\xef\x80\x93" } } },
        { "Movement", 2, { { "Assistance", "\xef\x95\x94" }, { "Exploits", "\xef\x87\x9b" } } },
        { "Menu",     2, { { "Appearance", "\xef\x80\x93" }, { "Animation", "\xef\x85\x90" } } },
        { "Misc",     2, { { "General", "\xef\x80\x93" }, { "Utilities", "\xef\x87\x9e" } } },
        { "Configs",  2, { { "Manager", "\xef\x83\x87" }, { "Transfer", "\xef\x82\xac" } } }
    };

    float const item_slide_x = ( 1.0f - alpha ) * 35.0f;
    float const item_x = panel_pos.x + 20.0f + item_slide_x;
    float const item_pad_x = 18.0f;
    float const cat_x = item_x + item_pad_x;
    float const item_w = 225.0f;

    float const logo_h_offset = ( logo_texture != nullptr && logo_texture->m_loaded ) ? 100.0f : 0.0f;
    float const base_start_y = ( logo_h_offset > 0.0f ) ? ( panel_pos.y + logo_h_offset ) : ( panel_pos.y + 20.0f );

    float const cat_header_h = 38.0f;
    float const item_h = 54.0f;
    float const item_spacing = 3.0f;
    float const cat_spacing = 14.0f;

    // Default to first tab (Combat -> Aim) if no tab is selected
    if ( m_active_tab < 0 || m_active_tab >= k_tab_count )
    {
        m_active_tab = 0;
        m_active_sub_tab[ 0 ] = 0;
    }

    // STEP 1: Calculate total content height for smooth scrolling
    float total_content_h = 0.0f;
    for ( int i = 0; i < k_tab_count; ++i )
    {
        total_content_h += cat_header_h + categories[ i ].m_item_count * ( item_h + item_spacing ) + cat_spacing;
    }

    bool const is_panel_hovered = ( mouse_pos.x >= p_panel_min.x && mouse_pos.x <= p_panel_min.x + panel_w &&
                                    mouse_pos.y >= p_panel_min.y && mouse_pos.y <= p_panel_min.y + panel_h );

    float const max_visible_h = panel_h - ( logo_h_offset > 0.0f ? logo_h_offset : 20.0f ) - 15.0f;
    float const max_scroll = std::max( 0.0f, total_content_h - max_visible_h );

    if ( is_panel_hovered && can_interact && io.MouseWheel != 0.0f && max_scroll > 0.0f )
    {
        m_scroll_y -= io.MouseWheel * 35.0f;
        m_scroll_y = std::clamp( m_scroll_y, 0.0f, max_scroll );
    }
    else if ( max_scroll <= 0.0f )
    {
        m_scroll_y = 0.0f;
    }
    m_scroll_y = std::clamp( m_scroll_y, 0.0f, max_scroll );

    m_anim_scroll.m_speed = 16.0f;
    m_anim_scroll.set( m_scroll_y );
    m_anim_scroll.update( delta_time );

    float const cur_scroll = m_anim_scroll.m_value;
    float const tab_start_y = base_start_y - cur_scroll;

    // STEP 2: Collision detection pass for clicking tab items under categories
    float calc_y = tab_start_y;
    int clicked_cat_index = -1;
    int clicked_item_index = -1;

    for ( int i = 0; i < k_tab_count; ++i )
    {
        calc_y += cat_header_h; // Skip unclickable Category Header

        for ( int j = 0; j < categories[ i ].m_item_count; ++j )
        {
            ImVec2 const t_min = ImVec2( item_x, calc_y );
            ImVec2 const t_max = ImVec2( item_x + item_w, calc_y + item_h );

            if ( mouse_pos.x >= t_min.x && mouse_pos.x <= t_max.x &&
                 mouse_pos.y >= t_min.y && mouse_pos.y <= t_max.y )
            {
                if ( io.MouseClicked[ 0 ] && can_interact )
                {
                    clicked_cat_index = i;
                    clicked_item_index = j;
                }
            }

            calc_y += item_h + item_spacing;
        }

        calc_y += cat_spacing;
    }

    if ( clicked_cat_index >= 0 && clicked_item_index >= 0 )
    {
        m_active_tab = clicked_cat_index;
        m_active_sub_tab[ clicked_cat_index ] = clicked_item_index;
    }

    // STEP 3: Render pass for Category Headers and Tab Items
    float current_y = tab_start_y;

    for ( int i = 0; i < k_tab_count; ++i )
    {
        // Render Category Header (Darker text description without icon)
        ImVec2 const cat_min = ImVec2( cat_x, current_y );
        ImU32 const title_col = IM_COL32( 62, 58, 70, ( int )( 170.0f * alpha ) );

        if ( font_medium_32 != nullptr )
        {
            std::string const upper_name = to_upper( categories[ i ].m_name );
            ImVec2 const text_pos = ImVec2( cat_min.x, cat_min.y + ( cat_header_h - 25.0f ) * 0.5f );
            draw_list->AddText( font_medium_32, 25.0f, text_pos, title_col, upper_name.c_str( ) );
        }

        current_y += cat_header_h;

        // Render Tab Items under this Category
        for ( int j = 0; j < categories[ i ].m_item_count; ++j )
        {
            ImVec2 const item_min = ImVec2( item_x, current_y );
            ImVec2 const item_max = ImVec2( item_x + item_w, current_y + item_h );

            bool const is_selected = ( m_active_tab == i && m_active_sub_tab[ i ] == j );
            bool const is_hovered = ( mouse_pos.x >= item_min.x && mouse_pos.x <= item_max.x &&
                                      mouse_pos.y >= item_min.y && mouse_pos.y <= item_max.y );

            m_anim_sub_hover[ i ][ j ].m_speed = 12.0f;
            m_anim_sub_hover[ i ][ j ].set( is_selected ? 1.0f : ( is_hovered ? 0.4f : 0.0f ) );
            m_anim_sub_hover[ i ][ j ].update( delta_time );

            float const h_val = m_anim_sub_hover[ i ][ j ].m_value;

            // Draw subtle whitish background & soft whitish border for active/hovered tab
            if ( h_val > 0.001f )
            {
                int const bg_alpha = ( int )( lerp_f( 0.0f, 10.0f, h_val ) * alpha );
                int const border_alpha = ( int )( lerp_f( 0.0f, 18.0f, h_val ) * alpha );

                draw_list->AddRectFilled( item_min, item_max, IM_COL32( 255, 255, 255, bg_alpha ), 12.0f );
                draw_list->AddRect( item_min, item_max, IM_COL32( 255, 255, 255, border_alpha ), 12.0f, 0, 1.0f );
            }

            // Text & icon color transition from inactive (110, 110, 125) to active accent (158, 149, 217)
            float const r_txt = lerp_f( 110.0f, 158.0f, h_val );
            float const g_txt = lerp_f( 110.0f, 149.0f, h_val );
            float const b_txt = lerp_f( 125.0f, 217.0f, h_val );

            ImU32 const item_col = IM_COL32( ( int )r_txt, ( int )g_txt, ( int )b_txt, a_255 );

            float icon_w = 0.0f;
            if ( icon_font != nullptr )
            {
                ImVec2 const icon_sz = icon_font->CalcTextSizeA( 18.0f, FLT_MAX, 0.0f, categories[ i ].m_items[ j ].m_icon );
                icon_w = icon_sz.x;
                ImVec2 const icon_pos = ImVec2( item_min.x + item_pad_x, item_min.y + ( item_h - icon_sz.y ) * 0.5f );
                draw_list->AddText( icon_font, 18.0f, icon_pos, item_col, categories[ i ].m_items[ j ].m_icon );
            }

            if ( font_medium_32 != nullptr )
            {
                float const text_x_offset = item_pad_x + icon_w + ( icon_w > 0.0f ? item_pad_x : 0.0f );
                ImVec2 const text_pos = ImVec2( item_min.x + text_x_offset, item_min.y + ( item_h - 25.0f ) * 0.5f );
                draw_list->AddText( font_medium_32, 25.0f, text_pos, item_col, categories[ i ].m_items[ j ].m_name );
            }

            current_y += item_h + item_spacing;
        }

        current_y += cat_spacing;
    }

    draw_list->PopClipRect( );
}
