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

#ifndef USE_SDL
static void glfw_error_callback( int error, char const* description )
{
    fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}
#else
static bool s_should_close = false;
#endif

bool c_render::init( int const width, int const height, char const* title )
{
#ifdef USE_SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER ) != 0 )
    {
        fprintf( stderr, "SDL Init Error: %s\n", SDL_GetError( ) );
        return false;
    }

    char const* glsl_version = "#version 130";
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

    SDL_WindowFlags const window_flags = ( SDL_WindowFlags )( SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED );
    m_window = SDL_CreateWindow( title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags );
    if ( m_window == nullptr )
    {
        fprintf( stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError( ) );
        return false;
    }

    m_gl_context = SDL_GL_CreateContext( m_window );
    if ( m_gl_context == nullptr )
    {
        fprintf( stderr, "SDL_GL_CreateContext Error: %s\n", SDL_GetError( ) );
        return false;
    }

    SDL_GL_MakeCurrent( m_window, m_gl_context );
    SDL_GL_SetSwapInterval( 0 );
#else
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
#endif

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

#ifdef USE_SDL
    ImGui_ImplSDL2_InitForOpenGL( m_window, m_gl_context );
#else
    ImGui_ImplGlfw_InitForOpenGL( m_window, true );
#endif
    ImGui_ImplOpenGL3_Init( glsl_version );

    return true;
}

void c_render::shutdown( )
{
    ImGui_ImplOpenGL3_Shutdown( );
#ifdef USE_SDL
    ImGui_ImplSDL2_Shutdown( );
#else
    ImGui_ImplGlfw_Shutdown( );
#endif
    ImGui::DestroyContext( );

    if ( m_window != nullptr )
    {
#ifdef USE_SDL
        if ( m_gl_context != nullptr )
        {
            SDL_GL_DeleteContext( m_gl_context );
            m_gl_context = nullptr;
        }
        SDL_DestroyWindow( m_window );
#else
        glfwDestroyWindow( m_window );
#endif
        m_window = nullptr;
    }

#ifdef USE_SDL
    SDL_Quit( );
#else
    glfwTerminate( );
#endif
}

void c_render::start_frame( )
{
#ifdef USE_SDL
    SDL_Event event;
    while ( SDL_PollEvent( &event ) )
    {
        ImGui_ImplSDL2_ProcessEvent( &event );
        if ( event.type == SDL_QUIT || ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID( m_window ) ) )
        {
            s_should_close = true;
        }
    }

    ImGui_ImplOpenGL3_NewFrame( );
    ImGui_ImplSDL2_NewFrame( );
#else
    glfwPollEvents( );

    ImGui_ImplOpenGL3_NewFrame( );
    ImGui_ImplGlfw_NewFrame( );
#endif
    ImGui::NewFrame( );
}

void c_render::end_frame( ImVec4 const& clear_color )
{
    ImGui::Render( );
    
    int display_w = 0, display_h = 0;
#ifdef USE_SDL
    SDL_GL_GetDrawableSize( m_window, &display_w, &display_h );
#else
    glfwGetFramebufferSize( m_window, &display_w, &display_h );
#endif
    glViewport( 0, 0, display_w, display_h );

    glClearColor( clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w );
    glClear( GL_COLOR_BUFFER_BIT );
    
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData( ) );

    // Capture backbuffer & perform real-time OpenGL GLSL blur for next frame
    g_blur.capture_and_blur( display_w, display_h );

#ifdef USE_SDL
    SDL_GL_SwapWindow( m_window );
#else
    glfwSwapBuffers( m_window );
#endif
}

bool c_render::should_close( ) const
{
#ifdef USE_SDL
    return s_should_close;
#else
    return glfwWindowShouldClose( m_window );
#endif
}

double c_render::get_time( ) const
{
#ifdef USE_SDL
    return ( double )SDL_GetTicks64( ) / 1000.0;
#else
    return glfwGetTime( );
#endif
}

float c_render::get_delta_time( ) const
{
    return ImGui::GetIO( ).DeltaTime;
}

void c_render::limit_fps( double const target_fps, double const current_time ) const
{
    double const target_frame_time = 1.0 / target_fps;

    while ( get_time( ) < current_time + target_frame_time )
    {
        std::this_thread::yield( );
    }
}

