#include "widgets/widgets.hxx"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cmath>

c_widgets g_widgets;

static inline float lerp_f( float a, float b, float t )
{
    return a + ( b - a ) * t;
}

bool c_widgets::is_any_open( ) const
{
    for ( auto const& kv : m_combo_open_states )
    {
        if ( kv.second ) { return true; }
    }
    for ( auto const& kv : m_picker_open_states )
    {
        if ( kv.second ) { return true; }
    }
    return false;
}

bool c_widgets::color_picker_dot( char const* label, float col[ 4 ], float const alpha, float const delta_time, ImFont* font, char const* id )
{
    if ( alpha <= 0.001f || col == nullptr )
    {
        return false;
    }

    std::string key = id ? id : label;
    bool& is_open = m_picker_open_states[ key ];

    ImGuiIO const& io = ImGui::GetIO( );
    ImVec2 const mouse_pos = io.MousePos;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

    ImVec2 const cursor_pos = ImGui::GetCursorScreenPos( );
    float const row_w = 340.0f;
    float const row_h = 28.0f;

    ImVec2 const r_min = cursor_pos;
    ImVec2 const r_max = ImVec2( cursor_pos.x + row_w, cursor_pos.y + row_h );

    // Label Text
    int const a255 = ( int )( 255.0f * alpha );
    if ( font != nullptr )
    {
        ImVec2 const txt_pos = ImVec2( r_min.x, r_min.y + ( row_h - 18.0f ) * 0.5f );
        draw_list->AddText( font, 18.0f, txt_pos, IM_COL32( 0xCC, 0xCC, 0xDB, a255 ), label );
    }

    // Color Dot
    ImVec2 const dot_center = ImVec2( r_max.x - 12.0f, r_min.y + row_h * 0.5f );
    float const dot_r = 7.5f;

    ImU32 const cur_col = IM_COL32( ( int )( col[ 0 ] * 255.0f ), ( int )( col[ 1 ] * 255.0f ), ( int )( col[ 2 ] * 255.0f ), a255 );
    draw_list->AddCircleFilled( dot_center, dot_r, cur_col );
    draw_list->AddCircle( dot_center, dot_r, IM_COL32( 255, 255, 255, ( int )( alpha * 140.0f ) ), 12, 1.2f );

    // Click dot to cycle colors
    bool const is_hovered = ( mouse_pos.x >= dot_center.x - dot_r - 4.0f && mouse_pos.x <= dot_center.x + dot_r + 4.0f &&
                              mouse_pos.y >= dot_center.y - dot_r - 4.0f && mouse_pos.y <= dot_center.y + dot_r + 4.0f );

    bool clicked = false;
    if ( is_hovered && io.MouseClicked[ 0 ] )
    {
        // Preset colors cycling palette: Red, Blue, Purple, Green, Yellow, Cyan
        static ImVec4 const palette[ 6 ] = {
            ImVec4( 1.0f, 0.29f, 0.29f, 1.0f ), // Red
            ImVec4( 0.29f, 0.54f, 1.0f, 1.0f ), // Blue
            ImVec4( 0.48f, 0.45f, 0.67f, 1.0f ), // Purple #7A72AC
            ImVec4( 0.29f, 1.0f, 0.57f, 1.0f ), // Green
            ImVec4( 1.0f, 0.82f, 0.29f, 1.0f ), // Yellow
            ImVec4( 0.29f, 0.95f, 1.0f, 1.0f )  // Cyan
        };

        static int palette_idx = 0;
        palette_idx = ( palette_idx + 1 ) % 6;

        col[ 0 ] = palette[ palette_idx ].x;
        col[ 1 ] = palette[ palette_idx ].y;
        col[ 2 ] = palette[ palette_idx ].z;
        col[ 3 ] = 1.0f;
        clicked = true;
    }

    ImGui::Dummy( ImVec2( row_w, row_h + 4.0f ) );
    return clicked;
}

bool c_widgets::combo( char const* label, int* current_item, std::vector<char const*> const& items, float const alpha, float const delta_time, ImFont* font, char const* id )
{
    if ( alpha <= 0.001f || current_item == nullptr || items.empty( ) )
    {
        return false;
    }

    std::string key = id ? id : label;
    bool& is_open = m_combo_open_states[ key ];

    animation_t& anim_open = m_anim_combos[ key ];
    anim_open.m_speed = 18.0f;
    anim_open.set( is_open ? 1.0f : 0.0f );
    anim_open.update( delta_time );

    float const open_val = anim_open.m_value;

    ImGuiIO const& io = ImGui::GetIO( );
    ImVec2 const mouse_pos = io.MousePos;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );

    ImVec2 const cursor_pos = ImGui::GetCursorScreenPos( );
    float const row_w = 340.0f;
    float const box_h = 32.0f;
    float const total_h = 58.0f;

    int const a255 = ( int )( 255.0f * alpha );

    // Combo Label
    if ( font != nullptr )
    {
        ImVec2 const txt_pos = ImVec2( cursor_pos.x, cursor_pos.y );
        draw_list->AddText( font, 18.0f, txt_pos, IM_COL32( 0xCC, 0xCC, 0xDB, a255 ), label );
    }

    // Dropdown Box Box Pill
    ImVec2 const box_min = ImVec2( cursor_pos.x, cursor_pos.y + 22.0f );
    ImVec2 const box_max = ImVec2( cursor_pos.x + row_w, box_min.y + box_h );

    bool const is_hovered = ( mouse_pos.x >= box_min.x && mouse_pos.x <= box_max.x &&
                              mouse_pos.y >= box_min.y && mouse_pos.y <= box_max.y );

    if ( is_hovered && io.MouseClicked[ 0 ] )
    {
        is_open = !is_open;
    }

    int const r_bg = ( int )lerp_f( 0x1A, 0x22, is_hovered ? 1.0f : 0.0f );
    int const g_bg = ( int )lerp_f( 0x1A, 0x22, is_hovered ? 1.0f : 0.0f );
    int const b_bg = ( int )lerp_f( 0x24, 0x2E, is_hovered ? 1.0f : 0.0f );

    draw_list->AddRectFilled( box_min, box_max, IM_COL32( r_bg, g_bg, b_bg, ( int )( alpha * 230.0f ) ), 8.0f );
    draw_list->AddRect( box_min, box_max, IM_COL32( 0x2A, 0x2A, 0x38, ( int )( alpha * 160.0f ) ), 8.0f, 0, 1.0f );

    // Current Option Text
    int const safe_idx = std::clamp( *current_item, 0, ( int )items.size( ) - 1 );
    char const* curr_text = items[ safe_idx ];

    if ( font != nullptr )
    {
        ImVec2 const item_pos = ImVec2( box_min.x + 12.0f, box_min.y + ( box_h - 18.0f ) * 0.5f );
        draw_list->AddText( font, 18.0f, item_pos, IM_COL32( 0xDD, 0xDD, 0xEC, a255 ), curr_text );
    }

    // Down Arrow Indicator
    ImVec2 const arr_center = ImVec2( box_max.x - 16.0f, box_min.y + box_h * 0.5f );
    ImU32 const arr_col = IM_COL32( 0x88, 0x88, 0x9B, a255 );
    draw_list->AddLine( ImVec2( arr_center.x - 4.0f, arr_center.y - 2.0f ), ImVec2( arr_center.x, arr_center.y + 2.0f ), arr_col, 1.5f );
    draw_list->AddLine( ImVec2( arr_center.x, arr_center.y + 2.0f ), ImVec2( arr_center.x + 4.0f, arr_center.y - 2.0f ), arr_col, 1.5f );

    bool changed = false;

    // Render Smoothly Animated Open Dropdown List Popup
    if ( open_val > 0.001f )
    {
        float const full_pop_h = items.size( ) * 28.0f + 6.0f;
        float const cur_pop_h = full_pop_h * open_val;
        ImVec2 const pop_min = ImVec2( box_min.x, box_max.y + 4.0f );
        ImVec2 const pop_max = ImVec2( box_max.x, pop_min.y + cur_pop_h );

        int const pop_alpha = ( int )( alpha * open_val * 245.0f );
        int const pop_border_a = ( int )( alpha * open_val * 180.0f );

        draw_list->PushClipRect( pop_min, pop_max, true );

        draw_list->AddRectFilled( pop_min, ImVec2( box_max.x, pop_min.y + full_pop_h ), IM_COL32( 0x1A, 0x1A, 0x24, pop_alpha ), 8.0f );
        draw_list->AddRect( pop_min, ImVec2( box_max.x, pop_min.y + full_pop_h ), IM_COL32( 0x7A, 0x72, 0xAC, pop_border_a ), 8.0f, 0, 1.2f );

        for ( size_t i = 0; i < items.size( ); ++i )
        {
            ImVec2 const opt_min = ImVec2( pop_min.x + 4.0f, pop_min.y + 3.0f + i * 28.0f );
            ImVec2 const opt_max = ImVec2( pop_max.x - 4.0f, opt_min.y + 26.0f );

            bool const opt_hovered = ( mouse_pos.x >= opt_min.x && mouse_pos.x <= opt_max.x &&
                                       mouse_pos.y >= opt_min.y && mouse_pos.y <= opt_max.y );

            if ( opt_hovered && open_val > 0.70f )
            {
                draw_list->AddRectFilled( opt_min, opt_max, IM_COL32( 0x2A, 0x26, 0x3E, ( int )( pop_alpha * 0.85f ) ), 6.0f );
                if ( io.MouseClicked[ 0 ] )
                {
                    *current_item = ( int )i;
                    is_open = false;
                    changed = true;
                }
            }

            ImU32 const opt_txt_col = ( ( int )i == safe_idx ) ? IM_COL32( 0x7A, 0x72, 0xAC, pop_alpha ) : IM_COL32( 0xCC, 0xCC, 0xDB, pop_alpha );
            if ( font != nullptr )
            {
                ImVec2 const opt_txt_pos = ImVec2( opt_min.x + 10.0f, opt_min.y + 4.0f );
                draw_list->AddText( font, 17.0f, opt_txt_pos, opt_txt_col, items[ i ] );
            }
        }

        draw_list->PopClipRect( );

        if ( io.MouseClicked[ 0 ] && !is_hovered &&
             ( mouse_pos.x < pop_min.x || mouse_pos.x > pop_max.x || mouse_pos.y < pop_min.y || mouse_pos.y > pop_max.y ) )
        {
            is_open = false;
        }
    }

    ImGui::Dummy( ImVec2( row_w, total_h + 4.0f ) );
    return changed;
}
