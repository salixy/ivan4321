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

    struct category_t
    {
        char const* m_name;
        char const* m_icon;
        int         m_item_count;
        char const* m_items[ 4 ];
    };

    category_t const categories[ k_tab_count ] = {
        { "Combat",   "\xef\x84\xb5", 3, { "Aim", "Triggerbot", "Accuracy" } },
        { "Visuals",  "\xef\x81\xae", 3, { "ESP", "Effects", "Settings" } },
        { "Movement", "\xef\x95\x94", 2, { "Assistance", "Exploits" } },
        { "World",    "\xef\x82\xac", 2, { "Environment", "Visual Style" } },
        { "Menu",     "\xef\x80\x93", 2, { "Appearance", "Animation" } },
        { "Misc",     "\xef\x87\x9e", 2, { "General", "Utilities" } },
        { "Configs",  "\xef\x83\x87", 2, { "Manager", "Transfer" } }
    };

    float const item_slide_x = ( 1.0f - alpha ) * 35.0f;
    float const cat_x = panel_pos.x + 35.0f + item_slide_x;
    float const item_x = panel_pos.x + 45.0f + item_slide_x;

    float const logo_h_offset = ( logo_texture != nullptr && logo_texture->m_loaded ) ? 100.0f : 0.0f;
    float const base_start_y = ( logo_h_offset > 0.0f ) ? ( panel_pos.y + logo_h_offset ) : ( panel_pos.y + 20.0f );

    float const cat_header_h = 28.0f;
    float const item_h = 36.0f;
    float const item_spacing = 2.0f;
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
            ImVec2 const t_max = ImVec2( item_x + 200.0f, calc_y + item_h );

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
        // Render Category Header (Icon + Smaller, darker text description)
        ImVec2 const cat_min = ImVec2( cat_x, current_y );

        ImU32 const icon_col = IM_COL32( 65, 60, 75, ( int )( 160.0f * alpha ) );
        ImU32 const title_col = IM_COL32( 72, 68, 80, ( int )( 170.0f * alpha ) );

        if ( icon_font != nullptr )
        {
            ImVec2 const icon_sz = icon_font->CalcTextSizeA( 16.0f, FLT_MAX, 0.0f, categories[ i ].m_icon );
            ImVec2 const icon_pos = ImVec2( cat_min.x, cat_min.y + ( cat_header_h - icon_sz.y ) * 0.5f );
            draw_list->AddText( icon_font, 16.0f, icon_pos, icon_col, categories[ i ].m_icon );
        }

        if ( font_medium_32 != nullptr )
        {
            std::string const upper_name = to_upper( categories[ i ].m_name );
            ImVec2 const text_pos = ImVec2( cat_min.x + 24.0f, cat_min.y + ( cat_header_h - 16.0f ) * 0.5f );
            draw_list->AddText( font_medium_32, 16.0f, text_pos, title_col, upper_name.c_str( ) );
        }

        current_y += cat_header_h;

        // Render Tab Items under this Category
        for ( int j = 0; j < categories[ i ].m_item_count; ++j )
        {
            ImVec2 const item_min = ImVec2( item_x, current_y );
            ImVec2 const item_max = ImVec2( item_x + 200.0f, current_y + item_h );

            bool const is_selected = ( m_active_tab == i && m_active_sub_tab[ i ] == j );
            bool const is_hovered = ( mouse_pos.x >= item_min.x && mouse_pos.x <= item_max.x &&
                                      mouse_pos.y >= item_min.y && mouse_pos.y <= item_max.y );

            m_anim_sub_hover[ i ][ j ].m_speed = 12.0f;
            m_anim_sub_hover[ i ][ j ].set( is_selected ? 1.0f : ( is_hovered ? 0.5f : 0.0f ) );
            m_anim_sub_hover[ i ][ j ].update( delta_time );

            float const h_val = m_anim_sub_hover[ i ][ j ].m_value;

            // Text color lerp from inactive (110, 110, 125) to active accent (158, 149, 217)
            float const r_txt = lerp_f( 110.0f, 158.0f, h_val );
            float const g_txt = lerp_f( 110.0f, 149.0f, h_val );
            float const b_txt = lerp_f( 125.0f, 217.0f, h_val );

            ImU32 const item_col = IM_COL32( ( int )r_txt, ( int )g_txt, ( int )b_txt, a_255 );

            if ( font_medium_32 != nullptr )
            {
                ImVec2 const text_pos = ImVec2( item_min.x + 12.0f, item_min.y + ( item_h - 25.0f ) * 0.5f );
                draw_list->AddText( font_medium_32, 25.0f, text_pos, item_col, categories[ i ].m_items[ j ] );
            }

            current_y += item_h + item_spacing;
        }

        current_y += cat_spacing;
    }

    draw_list->PopClipRect( );
}
