#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../src/stb_image_write.h"

int main( )
{
    int width = 0;
    int height = 0;
    int channels = 0;

    char const* path = "c:/Users/abile/ivan4321/assets/cici.jpg";
    unsigned char* data = stbi_load( path, &width, &height, &channels, 3 );
    if ( data == nullptr )
    {
        std::cerr << "Failed to load " << path << " -> " << stbi_failure_reason( ) << std::endl;
        return 1;
    }

    std::vector< unsigned char > out_data( width * height * 4 );

    for ( int y = 0; y < height; ++y )
    {
        for ( int x = 0; x < width; ++x )
        {
            int const in_idx = ( y * width + x ) * 3;
            int const out_idx = ( y * width + x ) * 4;

            float r = ( float )data[ in_idx + 0 ];
            float g = ( float )data[ in_idx + 1 ];
            float b = ( float )data[ in_idx + 2 ];

            // Key out white/light background with smooth alpha transition
            float const brightness = ( r + g + b ) / 3.0f;
            float const max_c = std::max( { r, g, b } );
            float const min_c = std::min( { r, g, b } );
            float const sat = max_c - min_c;

            float alpha = 1.0f;
            if ( brightness > 235.0f && sat < 20.0f )
            {
                alpha = 0.0f;
            }
            else if ( brightness > 215.0f && sat < 30.0f )
            {
                alpha = 1.0f - ( brightness - 215.0f ) / 20.0f;
            }

            // Subtle color grading towards UI accent color #7B73AD (r:123, g:115, b:173)
            float const tint_factor = 0.08f;
            r = r * ( 1.0f - tint_factor ) + 123.0f * tint_factor;
            g = g * ( 1.0f - tint_factor ) + 115.0f * tint_factor;
            b = b * ( 1.0f - tint_factor ) + 173.0f * tint_factor;

            r = std::clamp( r, 0.0f, 255.0f );
            g = std::clamp( g, 0.0f, 255.0f );
            b = std::clamp( b, 0.0f, 255.0f );
            alpha = std::clamp( alpha, 0.0f, 1.0f );

            out_data[ out_idx + 0 ] = ( unsigned char )r;
            out_data[ out_idx + 1 ] = ( unsigned char )g;
            out_data[ out_idx + 2 ] = ( unsigned char )b;
            out_data[ out_idx + 3 ] = ( unsigned char )( alpha * 255.0f );
        }
    }

    stbi_image_free( data );

    if ( stbi_write_png( "assets/cici.png", width, height, 4, out_data.data( ), width * 4 ) != 0 )
    {
        std::cout << "Successfully processed assets/cici.jpg -> assets/cici.png (" << width << "x" << height << ")" << std::endl;
    }
    else
    {
        std::cerr << "Failed to write assets/cici.png" << std::endl;
        return 1;
    }

    return 0;
}
