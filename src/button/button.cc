#include "button/button.hxx"
#include "menu/menu.hxx"

c_button g_button;

bool c_button::render( ImVec2 const& pos, char const* label, ImFont* font_32, float const delta_time )
{
    ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
    ImGuiIO const& io = ImGui::GetIO( );

    ImVec2 const p_min = pos;
    ImVec2 const p_max = ImVec2( pos.x + m_size.x, pos.y + m_size.y );

    ImVec2 const mouse_pos = io.MousePos;
    bool const is_hovered = ( mouse_pos.x >= p_min.x && mouse_pos.x <= p_max.x &&
                              mouse_pos.y >= p_min.y && mouse_pos.y <= p_max.y ) && !g_menu.m_dragging;

    bool const is_active = is_hovered && io.MouseDown[ 0 ];
    bool const is_clicked = is_hovered && io.MouseClicked[ 0 ];

    m_anim_hover.set( is_hovered ? 1.0f : 0.0f );
    m_anim_click.set( is_active ? 1.0f : 0.0f );

    m_anim_hover.m_speed = 12.0f;
    m_anim_click.m_speed = 16.0f;

    m_anim_hover.update( delta_time );
    m_anim_click.update( delta_time );

    float const hover = m_anim_hover.m_value;
    float const click = m_anim_click.m_value;

    float r = 123.0f + 25.0f * hover - 25.0f * click;
    float g = 115.0f + 20.0f * hover - 25.0f * click;
    float b = 173.0f + 30.0f * hover - 25.0f * click;

    auto const clamp_f = []( float const val, float const min_val, float const max_val ) -> float
    {
        return val < min_val ? min_val : ( val > max_val ? max_val : val );
    };

    r = clamp_f( r, 0.0f, 255.0f );
    g = clamp_f( g, 0.0f, 255.0f );
    b = clamp_f( b, 0.0f, 255.0f );

    ImU32 const current_bg = IM_COL32( ( int )r, ( int )g, ( int )b, 255 );
    ImU32 const current_aa = IM_COL32( ( int )r, ( int )g, ( int )b, 180 );

    float const rounding = 36.0f;

    draw_list->AddRectFilled( p_min, p_max, current_bg, rounding );
    draw_list->AddRect( p_min, p_max, current_aa, rounding, 0, 1.5f );

    if ( label != nullptr )
    {
        if ( font_32 != nullptr )
        {
            ImVec2 const text_size = font_32->CalcTextSizeA( 23.33f, FLT_MAX, 0.0f, label );
            ImVec2 const text_pos = ImVec2(
                pos.x + ( m_size.x - text_size.x ) * 0.5f,
                pos.y + ( m_size.y - text_size.y ) * 0.5f
            );

            draw_list->AddText( font_32, 23.33f, text_pos, m_label_color, label );
        }
        else
        {
            ImVec2 const text_size = ImGui::CalcTextSize( label );
            ImVec2 const text_pos = ImVec2(
                pos.x + ( m_size.x - text_size.x ) * 0.5f,
                pos.y + ( m_size.y - text_size.y ) * 0.5f
            );

            draw_list->AddText( text_pos, m_label_color, label );
        }
    }

    return is_clicked;
}
