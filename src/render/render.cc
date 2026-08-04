#include "render.hxx"
#include "fonts/fonts.hxx"
#include "blur/blur.hxx"

#include <stdio.h>
#include <thread>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

c_render g_render;

static void glfw_error_callback( int error, char const* description )
{
    fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

bool c_render::init( int const width, int const height, char const* title )
{
    glfwSetErrorCallback( glfw_error_callback );

    if ( !glfwInit( ) )
    {
        return false;
    }

    char const* glsl_version = "#version 130";
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
    glfwWindowHint( GLFW_MAXIMIZED, GLFW_TRUE );

    m_window = glfwCreateWindow( width, height, title, nullptr, nullptr );

    if ( m_window == nullptr )
    {
        return false;
    }

    glfwMakeContextCurrent( m_window );
    glfwSwapInterval( 0 );

    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );
    ImGuiIO& io = ImGui::GetIO( );
    ( void )io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    g_fonts.init( );
    m_font_regular = g_fonts.m_regular_18;
    m_font_medium = g_fonts.m_medium_18;
    m_font_medium_32 = g_fonts.m_medium_32;

    ImGui::StyleColorsDark( );

    ImGui_ImplGlfw_InitForOpenGL( m_window, true );
    ImGui_ImplOpenGL3_Init( glsl_version );

    return true;
}

void c_render::shutdown( )
{
    ImGui_ImplOpenGL3_Shutdown( );
    ImGui_ImplGlfw_Shutdown( );
    ImGui::DestroyContext( );

    if ( m_window != nullptr )
    {
        glfwDestroyWindow( m_window );
        m_window = nullptr;
    }

    glfwTerminate( );
}

void c_render::start_frame( )
{
    glfwPollEvents( );

    ImGui_ImplOpenGL3_NewFrame( );
    ImGui_ImplGlfw_NewFrame( );
    ImGui::NewFrame( );
}

void c_render::end_frame( ImVec4 const& clear_color )
{
    ImGui::Render( );
    
    int display_w, display_h;
    glfwGetFramebufferSize( m_window, &display_w, &display_h );
    glViewport( 0, 0, display_w, display_h );

    glClearColor( clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w );
    glClear( GL_COLOR_BUFFER_BIT );
    
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );

    // Capture backbuffer & perform real-time OpenGL GLSL blur for next frame
    g_blur.capture_and_blur( display_w, display_h );

    glfwSwapBuffers( m_window );
}

bool c_render::should_close( ) const
{
    return glfwWindowShouldClose( m_window );
}

float c_render::get_delta_time( ) const
{
    return ImGui::GetIO( ).DeltaTime;
}

void c_render::limit_fps( double const target_fps, double const current_time ) const
{
    double const target_frame_time = 1.0 / target_fps;

    while ( glfwGetTime( ) < current_time + target_frame_time )
    {
        std::this_thread::yield( );
    }
}
