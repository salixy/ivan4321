#include "tabs/tabs.hxx"
#include "motion/motion.hxx"

#include <algorithm>
#include <cmath>

c_tabs g_tabs;

static inline float lerp_f( float a, float b, float t )
{
    return a + ( b - a ) * t;
}

void c_tabs::render( ImVec2 const& panel_pos, float const panel_w, float const panel_h, float const alpha, float const delta_time, bool const can_interact, ImFont* font_medium_32, ImFont* icon_font )
{
    if ( alpha <= 0.001f || panel_w <= 1.0f )
    {
        return;
    }

    ImGuiIO const& io = ImGui::GetIO( );
    ImVec2 const mouse_pos = io.MousePos;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
    int const a_255 = ( int )( 255.0f * alpha );

    // Minimalistic, sleek left overlay panel (#1A1A1A, rounding 20px)
    ImVec2 const p_panel_min = panel_pos;
    ImVec2 const p_panel_max = ImVec2( panel_pos.x + panel_w, panel_pos.y + panel_h );

    draw_list->AddRectFilled( p_panel_min, p_panel_max, IM_COL32( 0x1A, 0x1A, 0x1A, a_255 ), 20.0f );
    draw_list->PushClipRect( p_panel_min, p_panel_max, true );

    tab_item_t const tabs[ k_tab_count ] = {
        { "Combat",   "\xef\x84\xb5", 3, { { "Aim" }, { "Triggerbot" }, { "Accuracy" } } },
        { "Visuals",  "\xef\x81\xae", 3, { { "ESP" }, { "Effects" }, { "Settings" } } },
        { "Movement", "\xef\x95\x94", 2, { { "Assistance" }, { "Exploits" } } },
        { "World",    "\xef\x82\xac", 2, { { "Environment" }, { "Visual Style" } } },
        { "Menu",     "\xef\x80\x93", 2, { { "Appearance" }, { "Animation" } } },
        { "Misc",     "\xef\x87\x9e", 2, { { "General" }, { "Utilities" } } },
        { "Configs",  "\xef\x83\x87", 2, { { "Manager" }, { "Transfer" } } }
    };

    float const tab_start_y = panel_pos.y + 60.0f;
    float const tab_w = 240.0f;
    float const tab_h = 36.0f;

    // Smooth horizontal entrance and exit slide animation for left tab menu & subtabs (right-to-left)
    float const item_slide_x = ( 1.0f - alpha ) * 35.0f;
    float const tab_x = panel_pos.x + 20.0f + item_slide_x;

    float const sub_tab_h = 32.0f;
    float const sub_tab_spacing = 4.0f;
    float const total_sub_unit_h = sub_tab_h + sub_tab_spacing; // 36.0f unified height step
    float const sub_w = 206.0f;
    float const sub_x = panel_pos.x + 40.0f + item_slide_x;

    // STEP 1: Update expansion animations FIRST so collision pass & render pass use 100% synchronized layout heights!
    float sub_expand_vals[ k_tab_count ];

    for ( int i = 0; i < k_tab_count; ++i )
    {
        bool const is_selected = ( m_active_tab == i );

        // When tab starts opening from collapsed state, snap subtab states cleanly so no old hover or bullet values tremble
        if ( is_selected && m_anim_sub_expand[ i ].m_value < 0.01f )
        {
            int const sub_count = tabs[ i ].m_sub_count;
            if ( m_active_sub_tab[ i ] < 0 || m_active_sub_tab[ i ] >= sub_count )
            {
                m_active_sub_tab[ i ] = 0;
            }
            int const sel_sub = m_active_sub_tab[ i ];

            m_anim_sub_bullet_rel_y[ i ].m_value = ( float )sel_sub;
            m_anim_sub_bullet_rel_y[ i ].set( ( float )sel_sub );

            for ( int j = 0; j < sub_count; ++j )
            {
                float const target_h = ( j == sel_sub ) ? 1.0f : 0.0f;
                m_anim_sub_hover[ i ][ j ].m_value = target_h;
                m_anim_sub_hover[ i ][ j ].set( target_h );
            }
        }

        // Calculate total subtab height using exact unified total_sub_unit_h (36.0f)
        float const total_sub_height = tabs[ i ].m_sub_count * total_sub_unit_h;
        float const base_speed = 10.0f; // Soft, smooth ~350ms luxury unfolding & closing
        float const anim_speed = base_speed * ( 108.0f / total_sub_height );

        m_anim_sub_expand[ i ].m_speed = anim_speed;
        m_anim_sub_expand[ i ].set( is_selected ? 1.0f : 0.0f );
        m_anim_sub_expand[ i ].update( delta_time );

        sub_expand_vals[ i ] = m_anim_sub_expand[ i ].m_value;
    }

    // STEP 2: Collision detection pass for click interactions (MUST run using updated sub_expand_vals!)
    float calc_y = tab_start_y;
    int clicked_tab_index = -2;
    int clicked_sub_index = -1;

    bool const is_panel_hovered = ( mouse_pos.x >= p_panel_min.x && mouse_pos.x <= p_panel_min.x + panel_w &&
                                    mouse_pos.y >= p_panel_min.y && mouse_pos.y <= p_panel_min.y + panel_h );

    if ( is_panel_hovered )
    {
        clicked_tab_index = -1;
    }

    for ( int i = 0; i < k_tab_count; ++i )
    {
        int const sub_count = tabs[ i ].m_sub_count;

        float const rounded_calc_y = std::round( calc_y );

        ImVec2 const t_min = ImVec2( tab_x, rounded_calc_y );
        ImVec2 const t_max = ImVec2( tab_x + tab_w, rounded_calc_y + tab_h );

        if ( mouse_pos.x >= t_min.x && mouse_pos.x <= t_max.x &&
             mouse_pos.y >= t_min.y && mouse_pos.y <= t_max.y )
        {
            if ( io.MouseClicked[ 0 ] && can_interact )
            {
                clicked_tab_index = i;
            }
        }

        calc_y += tab_h;

        float const expand_val = sub_expand_vals[ i ];

        // Balanced Smoothstep curve (3x^2 - 2x^3) for perfectly smooth opening & closing
        float const smoothed_expand = expand_val * expand_val * ( 3.0f - 2.0f * expand_val );

        float const sub_start_y = calc_y + 4.0f;

        // Only register subtab clicks when accordion tab is sufficiently expanded (> 0.85f)
        if ( expand_val > 0.85f )
        {
            for ( int j = 0; j < sub_count; ++j )
            {
                float const cur_sub_y = sub_start_y + j * total_sub_unit_h;
                ImVec2 const s_min = ImVec2( sub_x, cur_sub_y );
                ImVec2 const s_max = ImVec2( sub_x + sub_w, cur_sub_y + sub_tab_h );

                if ( mouse_pos.x >= s_min.x && mouse_pos.x <= s_max.x &&
                     mouse_pos.y >= s_min.y && mouse_pos.y <= s_max.y + 4.0f )
                {
                    if ( io.MouseClicked[ 0 ] && can_interact )
                    {
                        clicked_tab_index = i;
                        clicked_sub_index = j;
                    }
                }
            }
        }

        float const total_sub_h = sub_count * total_sub_unit_h;
        calc_y += ( total_sub_h + 4.0f ) * smoothed_expand;
        calc_y += 8.0f;
    }

    // Process tab selection changes before updating animation targets
    if ( io.MouseClicked[ 0 ] && can_interact )
    {
        if ( clicked_tab_index >= 0 )
        {
            if ( clicked_sub_index >= 0 )
            {
                m_active_tab = clicked_tab_index;
                m_active_sub_tab[ clicked_tab_index ] = clicked_sub_index;
            }
            else
            {
                if ( m_active_tab == clicked_tab_index )
                {
                    m_active_tab = -1;
                }
                else
                {
                    m_active_tab = clicked_tab_index;
                }
            }
        }
    }

    // STEP 3: Render pass using steady, perfectly synchronized layout coordinates
    float current_y = tab_start_y;

    for ( int i = 0; i < k_tab_count; ++i )
    {
        int const sub_count = tabs[ i ].m_sub_count;

        float const rounded_current_y = std::round( current_y );

        ImVec2 const tab_min = ImVec2( tab_x, rounded_current_y );
        ImVec2 const tab_max = ImVec2( tab_x + tab_w, rounded_current_y + tab_h );

        bool const is_parent_hovered = ( mouse_pos.x >= tab_min.x && mouse_pos.x <= tab_max.x &&
                                         mouse_pos.y >= tab_min.y && mouse_pos.y <= tab_max.y );

        bool const is_selected = ( m_active_tab == i );

        float const expand_val = sub_expand_vals[ i ];

        // Balanced Smoothstep curve (3x^2 - 2x^3)
        float const smoothed_expand = expand_val * expand_val * ( 3.0f - 2.0f * expand_val );

        // Hover & active animations for parent tab
        m_anim_tab_hover[ i ].m_speed = 11.0f;
        m_anim_tab_hover[ i ].set( is_parent_hovered ? 1.0f : 0.0f );
        m_anim_tab_hover[ i ].update( delta_time );

        m_anim_tab_active[ i ].m_speed = 11.0f;
        m_anim_tab_active[ i ].set( is_selected ? 1.0f : 0.0f );
        m_anim_tab_active[ i ].update( delta_time );

        float const h_val = m_anim_tab_hover[ i ].m_value;
        float const a_val = m_anim_tab_active[ i ].m_value;

        // Clean, minimal parent tab styling without heavy boxes or borders
        if ( h_val > 0.001f && !is_selected )
        {
            int const hover_bg_a = ( int )( h_val * 18.0f * alpha );
            draw_list->AddRectFilled( tab_min, tab_max, IM_COL32( 255, 255, 255, hover_bg_a ), 8.0f );
        }

        // Text & icon color for parent tab: smoothly transition to accent color when active
        float const h_subtle = h_val * 0.50f;
        int const r_col = ( int )lerp_f( lerp_f( 105.0f, 230.0f, h_subtle ), 158.0f, a_val );
        int const g_col = ( int )lerp_f( lerp_f( 105.0f, 230.0f, h_subtle ), 149.0f, a_val );
        int const b_col = ( int )lerp_f( lerp_f( 118.0f, 240.0f, h_subtle ), 217.0f, a_val );
        ImU32 const item_col = IM_COL32( r_col, g_col, b_col, a_255 );

        if ( icon_font != nullptr )
        {
            ImVec2 const icon_sz = icon_font->CalcTextSizeA( 22.0f, FLT_MAX, 0.0f, tabs[ i ].m_icon );
            ImVec2 const icon_pos = ImVec2( tab_min.x + 18.0f, tab_min.y + ( tab_h - icon_sz.y ) * 0.5f );
            draw_list->AddText( icon_font, 22.0f, icon_pos, item_col, tabs[ i ].m_icon );
        }

        if ( font_medium_32 != nullptr )
        {
            ImVec2 const text_pos = ImVec2( tab_min.x + 48.0f, tab_min.y + ( tab_h - 26.0f ) * 0.5f );
            draw_list->AddText( font_medium_32, 26.0f, text_pos, item_col, tabs[ i ].m_name );
        }

        current_y += tab_h;

        // Render sub-tabs with smooth clipped accordion container
        if ( expand_val > 0.001f )
        {
            float const sub_start_y = current_y + 4.0f;
            float const total_sub_h = sub_count * total_sub_unit_h;
            float const anim_container_h = total_sub_h * smoothed_expand;

            // Clamp active sub-tab index to valid range [0, sub_count - 1]
            if ( m_active_sub_tab[ i ] >= sub_count )
            {
                m_active_sub_tab[ i ] = std::max( 0, sub_count - 1 );
            }
            int const selected_sub = m_active_sub_tab[ i ];

            // Continuous gliding slot animation between sub-tabs
            m_anim_sub_bullet_rel_y[ i ].m_speed = 14.0f;
            m_anim_sub_bullet_rel_y[ i ].set( ( float )selected_sub );
            m_anim_sub_bullet_rel_y[ i ].update( delta_time );

            // Push subtab container clip rect to smoothly uncover subtabs during expansion without overlaps
            ImVec2 const sub_clip_min = ImVec2( panel_pos.x + 15.0f, sub_start_y - 2.0f );
            ImVec2 const sub_clip_max = ImVec2( sub_x + sub_w + 10.0f, sub_start_y + anim_container_h );

            draw_list->PushClipRect( sub_clip_min, sub_clip_max, true );

            float const float_offset = 0.0f;

            for ( int j = 0; j < sub_count; ++j )
            {
                float const cur_sub_y = sub_start_y + j * total_sub_unit_h + float_offset;

                ImVec2 const sub_min = ImVec2( sub_x, cur_sub_y );
                ImVec2 const sub_max = ImVec2( sub_x + sub_w, cur_sub_y + sub_tab_h );

                bool const is_this_sub_hovered = ( expand_val > 0.85f ) &&
                                                 ( mouse_pos.x >= sub_min.x && mouse_pos.x <= sub_max.x &&
                                                   mouse_pos.y >= sub_min.y && mouse_pos.y <= sub_max.y );

                bool const is_sub_selected = ( is_selected && selected_sub == j );

                m_anim_sub_hover[ i ][ j ].m_speed = 14.0f;
                m_anim_sub_hover[ i ][ j ].set( is_this_sub_hovered ? 1.0f : 0.0f );
                m_anim_sub_hover[ i ][ j ].update( delta_time );

                float const sub_h_val = m_anim_sub_hover[ i ][ j ].m_value;
                float const sub_alpha = alpha * ( is_selected ? std::clamp( smoothed_expand * 1.15f, 0.0f, 1.0f ) : ( m_active_tab >= 0 ? 0.0f : std::clamp( smoothed_expand * 1.15f, 0.0f, 1.0f ) ) );

                if ( sub_alpha > 0.001f )
                {
                    int const sub_a255 = ( int )( 255.0f * sub_alpha );

                    // Sub-tab text color: accent color when active, hover glow when hovered
                    float const sub_h_glow = sub_h_val * 0.60f;
                    float const target_r = is_sub_selected ? 158.0f : lerp_f( 95.0f, 220.0f, sub_h_glow );
                    float const target_g = is_sub_selected ? 149.0f : lerp_f( 95.0f, 220.0f, sub_h_glow );
                    float const target_b = is_sub_selected ? 217.0f : lerp_f( 108.0f, 235.0f, sub_h_glow );

                    ImU32 const sub_txt_col = IM_COL32( ( int )target_r, ( int )target_g, ( int )target_b, sub_a255 );

                    if ( font_medium_32 != nullptr )
                    {
                        ImVec2 const sub_txt_pos = ImVec2( sub_min.x + 18.0f, sub_min.y + ( sub_tab_h - 26.0f ) * 0.5f );
                        draw_list->AddText( font_medium_32, 26.0f, sub_txt_pos, sub_txt_col, tabs[ i ].m_sub_tabs[ j ].m_name );
                    }
                }
            }

            draw_list->PopClipRect( );

            current_y += ( total_sub_h + 4.0f ) * smoothed_expand;
        }

        current_y += 8.0f;
    }

    draw_list->PopClipRect( );
}
