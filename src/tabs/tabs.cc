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
        { "Combat",   3, { { "Aim", "\xef\x81\x9b" }, { "Triggerbot", "\xef\x83\xa7" }, { "Accuracy", "\xef\x85\x80" } } },
        { "Visuals",  3, { { "ESP", "\xef\x81\xae" }, { "Effects", "\xef\x80\x85" }, { "Settings", "\xef\x87\x9e" } } },
        { "Movement", 2, { { "Assistance", "\xef\x95\x94" }, { "Exploits", "\xef\x86\x88" } } },
        { "Menu",     2, { { "Appearance", "\xef\x84\x88" }, { "Animation", "\xef\x85\x84" } } },
        { "Misc",     2, { { "General", "\xef\x80\x93" }, { "Utilities", "\xef\x82\xad" } } },
        { "Configs",  2, { { "Manager", "\xef\x81\xbc" }, { "Transfer", "\xef\x85\x8d" } } }
    };

    // Update smooth collapse animation state
    m_anim_collapse.m_speed = 14.0f;
    m_anim_collapse.set( m_collapsed ? 1.0f : 0.0f );
    m_anim_collapse.update( delta_time );
    float const collapse_t = m_anim_collapse.m_value;

    m_current_panel_w = lerp_f( panel_w, 78.0f * ( panel_w / 275.0f ), collapse_t );

    // Render Collapse / Expand Toggle Button at top-right edge of the window
    if ( can_interact && alpha > 0.5f )
    {
        ImVec2 const toggle_sz = ImVec2( 26.0f, 26.0f );
        float const window_total_w = 1101.0f - 197.0f * collapse_t;
        float const toggle_x = panel_pos.x + window_total_w - 48.0f;
        float const toggle_y = panel_pos.y + 20.0f;
        ImVec2 const toggle_min = ImVec2( toggle_x, toggle_y );
        ImVec2 const toggle_max = ImVec2( toggle_min.x + toggle_sz.x, toggle_min.y + toggle_sz.y );

        bool const is_t_hovered = ( mouse_pos.x >= toggle_min.x && mouse_pos.x <= toggle_max.x &&
                                    mouse_pos.y >= toggle_min.y && mouse_pos.y <= toggle_max.y );

        m_anim_toggle_hover.m_speed = 16.0f;
        m_anim_toggle_hover.set( is_t_hovered ? 1.0f : 0.0f );
        m_anim_toggle_hover.update( delta_time );

        float const t_h_val = m_anim_toggle_hover.m_value;
        int const t_bg_a = ( int )( lerp_f( 25.0f, 75.0f, t_h_val ) * alpha );
        int const t_txt_a = ( int )( lerp_f( 160.0f, 255.0f, t_h_val ) * alpha );

        draw_list->AddRectFilled( toggle_min, toggle_max, IM_COL32( 255, 255, 255, t_bg_a ), 8.0f );
        draw_list->AddRect( toggle_min, toggle_max, IM_COL32( 255, 255, 255, ( int )( 30.0f * alpha ) ), 8.0f, 0, 1.0f );

        char const* toggle_icon = m_collapsed ? "\xef\x81\x94" : "\xef\x81\x93"; // fa-chevron-right / fa-chevron-left
        if ( icon_font != nullptr )
        {
            ImVec2 const t_icon_sz = icon_font->CalcTextSizeA( 14.0f, FLT_MAX, 0.0f, toggle_icon );
            ImVec2 const t_icon_pos = ImVec2( toggle_min.x + ( toggle_sz.x - t_icon_sz.x ) * 0.5f, toggle_min.y + ( toggle_sz.y - t_icon_sz.y ) * 0.5f );
            draw_list->AddText( icon_font, 14.0f, t_icon_pos, IM_COL32( 220, 220, 240, t_txt_a ), toggle_icon );
        }

        if ( is_t_hovered && io.MouseClicked[ 0 ] )
        {
            m_collapsed = !m_collapsed;
        }
    }

    float const item_slide_x = ( 1.0f - alpha ) * 35.0f;
    float const item_w = lerp_f( 235.0f, 50.0f, collapse_t );
    float const item_x = lerp_f( panel_pos.x + 20.0f + item_slide_x, panel_pos.x + ( m_current_panel_w - item_w ) * 0.5f, collapse_t );
    float const item_pad_x = lerp_f( 18.0f, 0.0f, collapse_t );
    float const cat_x = item_x + 18.0f;

    float const logo_h_offset = ( logo_texture != nullptr && logo_texture->m_loaded ) ? lerp_f( 100.0f, 95.0f, collapse_t ) : 0.0f;
    float const base_start_y = ( logo_h_offset > 0.0f ) ? ( panel_pos.y + logo_h_offset ) : ( panel_pos.y + 20.0f );

    float const scroll_clip_min_y = base_start_y + 4.0f;
    float const scroll_clip_max_y = panel_pos.y + panel_h - 16.0f;

    draw_list->PushClipRect( ImVec2( p_panel_min.x, scroll_clip_min_y ), ImVec2( p_panel_min.x + m_current_panel_w + 120.0f, scroll_clip_max_y ), false );

    float const cat_header_h = lerp_f( 38.0f, 16.0f, collapse_t );
    float const item_h = 54.0f;
    float const item_spacing = 3.0f;
    float const cat_spacing = lerp_f( 14.0f, 8.0f, collapse_t );

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

    bool const is_panel_hovered = ( mouse_pos.x >= p_panel_min.x && mouse_pos.x <= p_panel_min.x + m_current_panel_w &&
                                    mouse_pos.y >= p_panel_min.y && mouse_pos.y <= p_panel_min.y + panel_h );

    float const max_visible_h = scroll_clip_max_y - scroll_clip_min_y;
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
        // Render Category Header (fade out text when collapsing)
        if ( font_medium_32 != nullptr && collapse_t < 0.90f )
        {
            ImVec2 const cat_min = ImVec2( cat_x, current_y );
            int const cat_alpha_val = ( int )( ( 1.0f - collapse_t ) * 170.0f * alpha );
            if ( cat_alpha_val > 0 )
            {
                ImU32 const title_col = IM_COL32( 62, 58, 70, cat_alpha_val );
                std::string const upper_name = to_upper( categories[ i ].m_name );
                ImVec2 const text_pos = ImVec2( cat_min.x, cat_min.y + ( cat_header_h - 25.0f ) * 0.5f );
                draw_list->AddText( font_medium_32, 25.0f, text_pos, title_col, upper_name.c_str( ) );
            }
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
                int const bg_alpha = ( int )( lerp_f( 0.0f, 5.0f, h_val ) * alpha );
                int const border_alpha = ( int )( lerp_f( 0.0f, 8.0f, h_val ) * alpha );

                draw_list->AddRectFilled( item_min, item_max, IM_COL32( 255, 255, 255, bg_alpha ), lerp_f( 12.0f, 14.0f, collapse_t ) );
                draw_list->AddRect( item_min, item_max, IM_COL32( 255, 255, 255, border_alpha ), lerp_f( 12.0f, 14.0f, collapse_t ), 0, 1.0f );
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
                float const icon_x_pos = lerp_f( item_min.x + 18.0f, item_min.x + ( item_w - icon_sz.x ) * 0.5f, collapse_t );
                ImVec2 const icon_pos = ImVec2( icon_x_pos, item_min.y + ( item_h - icon_sz.y ) * 0.5f );
                draw_list->AddText( icon_font, 18.0f, icon_pos, item_col, categories[ i ].m_items[ j ].m_icon );
            }

            // Subtab label text (fades out as panel collapses)
            if ( font_medium_32 != nullptr && collapse_t < 0.95f )
            {
                int const text_a = ( int )( ( 1.0f - collapse_t ) * ( float )a_255 );
                if ( text_a > 0 )
                {
                    ImU32 const label_col = IM_COL32( ( int )r_txt, ( int )g_txt, ( int )b_txt, text_a );
                    float const text_x_offset = 18.0f + icon_w + ( icon_w > 0.0f ? 18.0f : 0.0f );
                    ImVec2 const text_pos = ImVec2( item_min.x + text_x_offset, item_min.y + ( item_h - 25.0f ) * 0.5f );
                    draw_list->AddText( font_medium_32, 25.0f, text_pos, label_col, categories[ i ].m_items[ j ].m_name );
                }
            }

            // Stylish floating Tooltip when collapsed and hovered
            if ( collapse_t > 0.4f && is_hovered && font_medium_32 != nullptr )
            {
                char const* item_label = categories[ i ].m_items[ j ].m_name;
                ImVec2 const txt_sz = font_medium_32->CalcTextSizeA( 20.0f, FLT_MAX, 0.0f, item_label );
                ImVec2 const tt_sz = ImVec2( txt_sz.x + 24.0f, 32.0f );
                ImVec2 const tt_min = ImVec2( item_max.x + 10.0f, item_min.y + ( item_h - tt_sz.y ) * 0.5f );
                ImVec2 const tt_max = ImVec2( tt_min.x + tt_sz.x, tt_min.y + tt_sz.y );

                int const tt_alpha = ( int )( ( collapse_t - 0.4f ) / 0.6f * ( float )a_255 );
                draw_list->AddRectFilled( tt_min, tt_max, IM_COL32( 28, 26, 36, tt_alpha ), 8.0f );
                draw_list->AddRect( tt_min, tt_max, IM_COL32( 158, 149, 217, ( int )( 0.6f * ( float )tt_alpha ) ), 8.0f, 0, 1.0f );

                ImVec2 const tt_txt_pos = ImVec2( tt_min.x + 12.0f, tt_min.y + ( tt_sz.y - 20.0f ) * 0.5f );
                draw_list->AddText( font_medium_32, 20.0f, tt_txt_pos, IM_COL32( 240, 240, 250, tt_alpha ), item_label );
            }

            current_y += item_h + item_spacing;
        }

        current_y += cat_spacing;
    }

    // Render smooth top and bottom vertical gradient fade overlays (#161616 panel background)
    if ( a_255 > 0 )
    {
        float const top_fade_factor = std::clamp( cur_scroll / 22.0f, 0.0f, 1.0f );
        float const bottom_fade_factor = ( max_scroll > 1.0f ) ? std::clamp( ( max_scroll - cur_scroll ) / 22.0f, 0.0f, 1.0f ) : 0.0f;

        float const fade_min_x = item_x;
        float const fade_max_x = item_x + item_w;

        // Top fade-in gradient overlay (active only when scrolled down from top)
        if ( top_fade_factor > 0.001f )
        {
            int const top_a = ( int )( ( float )a_255 * top_fade_factor );
            ImU32 const top_full = IM_COL32( 0x16, 0x16, 0x16, top_a );
            ImU32 const top_zero = IM_COL32( 0x16, 0x16, 0x16, 0 );

            draw_list->AddRectFilledMultiColor(
                ImVec2( fade_min_x, scroll_clip_min_y ),
                ImVec2( fade_max_x, scroll_clip_min_y + 22.0f ),
                top_full, top_full, top_zero, top_zero
            );
        }

        // Bottom fade-out gradient overlay (active only when content remains below)
        if ( bottom_fade_factor > 0.001f )
        {
            int const bot_a = ( int )( ( float )a_255 * bottom_fade_factor );
            ImU32 const bot_full = IM_COL32( 0x16, 0x16, 0x16, bot_a );
            ImU32 const bot_zero = IM_COL32( 0x16, 0x16, 0x16, 0 );

            draw_list->AddRectFilledMultiColor(
                ImVec2( fade_min_x, scroll_clip_max_y - 22.0f ),
                ImVec2( fade_max_x, scroll_clip_max_y ),
                bot_zero, bot_zero, bot_full, bot_full
            );
        }
    }

    draw_list->PopClipRect( );
}
