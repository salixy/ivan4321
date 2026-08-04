#include "render/render.hxx"
#include "menu/menu.hxx"

int main( int, char** )
{
    if ( !g_render.init( 1280, 720, "Dear ImGui GLFW+OpenGL3 example" ) )
    {
        return 1;
    }

    g_menu.init( );

    double const target_fps = 240.0;

    while ( !g_render.should_close( ) )
    {
        double const current_time = glfwGetTime( );

        g_render.start_frame( );
        if ( g_menu.m_open )
        {
            g_menu.draw( g_render.m_font_medium_32, g_render.get_delta_time( ) );
        }
        g_render.end_frame( g_menu.m_clear_color );

        g_render.limit_fps( target_fps, current_time );
    }

    g_render.shutdown( );

    return 0;
}
