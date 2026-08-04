#include "texture/texture.hxx"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fstream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cstdint>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

static bool file_exists( char const* path )
{
    std::ifstream f( path );
    return f.good( );
}

static char const* find_image_path( char const* relative_path )
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

bool c_texture::load_from_file( char const* filename )
{
    if ( m_loaded )
    {
        return true;
    }

    char const* path = find_image_path( filename );
    if ( !file_exists( path ) )
    {
        return false;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load( path, &width, &height, &channels, 4 );
    if ( data == nullptr )
    {
        return false;
    }

    // Blue Chroma Key removal for cici character image
    // Removes bright blue background pixels completely, preserving 100% of black clothing, hair, skin, and details
    if ( strstr( filename, "cici" ) != nullptr )
    {
        int const pixel_count = width * height;
        for ( int i = 0; i < pixel_count; ++i )
        {
            unsigned char* p = &data[ i * 4 ];
            float const r = ( float )p[ 0 ];
            float const g = ( float )p[ 1 ];
            float const b = ( float )p[ 2 ];

            float const max_rg = std::max( r, g );

            if ( b > 100.0f && ( b - max_rg ) > 25.0f )
            {
                float const diff = b - max_rg;
                if ( diff > 45.0f )
                {
                    p[ 3 ] = 0;
                }
                else
                {
                    float const a = 1.0f - ( diff - 25.0f ) / 20.0f;
                    p[ 3 ] = ( unsigned char )( std::clamp( a, 0.0f, 1.0f ) * ( float )p[ 3 ] );
                }
                p[ 2 ] = ( unsigned char )max_rg; // Remove blue color fringe on hair/skin edge pixels
            }
        }
    }

    GLuint texture_id = 0;
    glGenTextures( 1, &texture_id );
    glBindTexture( GL_TEXTURE_2D, texture_id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    typedef void ( APIENTRY * PFNGLGENERATEMIPMAPPROC )( GLenum target );
    PFNGLGENERATEMIPMAPPROC glGenerateMipmapFunc = ( PFNGLGENERATEMIPMAPPROC )glfwGetProcAddress( "glGenerateMipmap" );
    if ( glGenerateMipmapFunc != nullptr )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glGenerateMipmapFunc( GL_TEXTURE_2D );
    }
    else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    }

    stbi_image_free( data );

    m_texture_id = texture_id;
    m_width = width;
    m_height = height;
    m_loaded = true;

    return true;
}

void c_texture::cleanup( )
{
    if ( m_loaded && m_texture_id != 0 )
    {
        glDeleteTextures( 1, &m_texture_id );
        m_texture_id = 0;
        m_loaded = false;
    }
}
