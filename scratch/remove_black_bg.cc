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

    char const* input_path = "c:/Users/abile/ivan4321/assets/cici.jpg";
    unsigned char* data = stbi_load( input_path, &width, &height, &channels, 0 );
    if ( data == nullptr )
    {
        std::cout << "Failed to load: " << stbi_failure_reason( ) << std::endl;
        return 1;
    }

    std::vector< unsigned char > out_data( width * height * 4 );

    for ( int y = 0; y < height; ++y )
    {
        for ( int x = 0; x < width; ++x )
        {
            int const idx = ( y * width + x ) * 4;

            float r = ( float )data[ idx + 0 ];
            float g = ( float )data[ idx + 1 ];
            float b = ( float )data[ idx + 2 ];
            float a = ( float )data[ idx + 3 ] / 255.0f;

            float const max_c = std::max( { r, g, b } );

            // Black background keying with smooth anti-aliased edge alpha transition
            if ( max_c < 18.0f )
            {
                a = 0.0f;
            }
            else if ( max_c < 45.0f )
            {
                a = ( max_c - 18.0f ) / 27.0f;
            }

            out_data[ idx + 0 ] = ( unsigned char )r;
            out_data[ idx + 1 ] = ( unsigned char )g;
            out_data[ idx + 2 ] = ( unsigned char )b;
            out_data[ idx + 3 ] = ( unsigned char )( std::clamp( a, 0.0f, 1.0f ) * 255.0f );
        }
    }

    stbi_image_free( data );

    char const* output_path = "c:/Users/abile/ivan4321/assets/cici.png";
    if ( stbi_write_png( output_path, width, height, 4, out_data.data( ), width * 4 ) != 0 )
    {
        std::cout << "Successfully made background 100% transparent for " << output_path << std::endl;
    }
    else
    {
        std::cerr << "Failed to write " << output_path << std::endl;
        return 1;
    }

    return 0;
}
