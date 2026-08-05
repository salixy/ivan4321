#include "menu/menu_content.hxx"
#include "menu/menu.hxx"
#include "tabs/tabs.hxx"
#include "widgets/widgets.hxx"
#include "settings/settings.hxx"

#include <vector>

namespace menu_content
{
    void render_dashboard_content(
        c_menu* menu,
        float const tabs_alpha,
        float const ease_t,
        ImFont* font_medium_32,
        float const delta_time
    )
    {
        if ( tabs_alpha <= 0.001f ) { return; }

        ImDrawList* draw_list = ImGui::GetForegroundDrawList( );
        int const a_255 = ( int )( 255.0f * tabs_alpha );

        float const search_slide_x = ( 1.0f - ease_t ) * 30.0f;

        // Push clip rect to strictly constrain all dashboard cards & widgets within the main window boundary!
        draw_list->PushClipRect( ImVec2( menu->m_pos.x + 250.0f, menu->m_pos.y ), ImVec2( menu->m_pos.x + menu->m_size.x, menu->m_pos.y + menu->m_size.y ), true );

        // Content Area Rectangle Container (#1A1A1A, extending to top, bottom, and right edges of menu)
        ImVec2 const content_min = ImVec2( menu->m_pos.x + 265.0f + search_slide_x, menu->m_pos.y );
        ImVec2 const content_max = ImVec2( menu->m_pos.x + menu->m_size.x, menu->m_pos.y + menu->m_size.y );

        draw_list->AddRectFilled( content_min, content_max, IM_COL32( 0x1A, 0x1A, 0x1A, a_255 ), 40.0f );

        // Render active tab & subtab content cards
        if ( g_tabs.m_active_tab >= 0 && g_tabs.m_active_tab < c_tabs::k_tab_count )
        {
            // Active tab contents area
        }

        draw_list->PopClipRect( );
    }
}
