#include "fonts/fonts.hxx"
#include "misc/freetype/imgui_freetype.h"
#include <fstream>
#include <cstdio>

c_fonts g_fonts;

static bool file_exists( char const* path )
{
    std::ifstream f( path );
    return f.good( );
}

static char const* find_font_path( char const* relative_path )
{
    if ( file_exists( relative_path ) )
    {
        return relative_path;
    }

    static char alt_path[ 512 ];
    snprintf( alt_path, sizeof( alt_path ), "../%s", relative_path );
    if ( file_exists( alt_path ) )
    {
        return alt_path;
    }

    return relative_path;
}

bool c_fonts::init( )
{
    ImGuiIO& io = ImGui::GetIO( );
    
    ImWchar const* cyrillic = io.Fonts->GetGlyphRangesCyrillic( );
    static ImWchar const icon_ranges[ ] = { 0xF000, 0xF8FF, 0 };

    char const* path_regular = find_font_path( "assets/fonts/Poppins-Regular.ttf" );
    char const* path_medium = find_font_path( "assets/fonts/Poppins-Medium.ttf" );
    char const* path_calsans = find_font_path( "assets/fonts/CalSans-SemiBold.ttf" );
    char const* path_fa = find_font_path( "assets/fonts/fa-solid-900.ttf" );
    char const* path_fa_brands = find_font_path( "assets/fonts/fa-brands-400.ttf" );

    m_regular_18 = io.Fonts->AddFontFromFileTTF( path_regular, 12.0f, nullptr, cyrillic );
    m_medium_18 = io.Fonts->AddFontFromFileTTF( path_medium, 12.0f, nullptr, cyrillic );
    m_medium_32 = io.Fonts->AddFontFromFileTTF( path_medium, 23.33f, nullptr, cyrillic );

    if ( file_exists( path_calsans ) )
    {
        ImFontConfig font_config;
        font_config.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_Bold;
        m_calsans_42 = io.Fonts->AddFontFromFileTTF( path_calsans, 42.0f, &font_config, nullptr );
    }

    if ( file_exists( path_fa_brands ) )
    {
        m_brand_font_40 = io.Fonts->AddFontFromFileTTF( path_fa_brands, 26.67f, nullptr, icon_ranges );
    }

    if ( !io.Fonts->Fonts.empty( ) && file_exists( path_fa ) )
    {
        ImFontConfig icon_config;
        icon_config.MergeMode = true;
        icon_config.PixelSnapH = true;

        io.Fonts->AddFontFromFileTTF( path_fa, 14.67f, &icon_config, icon_ranges );
    }

    if ( file_exists( path_fa ) )
    {
        m_icon_font = io.Fonts->AddFontFromFileTTF( path_fa, 14.67f, nullptr, icon_ranges );
    }

    return ( m_regular_18 != nullptr && m_medium_32 != nullptr );
}
