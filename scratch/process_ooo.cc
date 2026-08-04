#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <queue>

#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../src/stb_image_write.h"

int main( )
{
    int width = 0;
    int height = 0;
    int channels = 0;

    char const* path = "C:/Users/abile/.gemini/antigravity-ide/brain/416e8e42-d40c-4639-9bf6-faada125089e/media__1785111528130.jpg";
    unsigned char* data = stbi_load( path, &width, &height, &channels, 3 );

    if ( data == nullptr )
    {
        std::cerr << "Failed to load " << path << " -> " << stbi_failure_reason( ) << std::endl;
        return 1;
    }

    std::cout << "Loaded new black-bg character image: " << width << "x" << height << " (" << channels << " channels)" << std::endl;

    // Grid to track exterior black background pixels via BFS Flood Fill
    std::vector< bool > is_bg( width * height, false );
    std::queue< std::pair< int, int > > q;

    auto is_black_pixel = [ data, width ]( int x, int y ) -> bool
    {
        int const idx = ( y * width + x ) * 3;
        float r = ( float )data[ idx + 0 ];
        float g = ( float )data[ idx + 1 ];
        float b = ( float )data[ idx + 2 ];

        float const avg = ( r + g + b ) / 3.0f;
        return ( avg < 22.0f ); // Pure black or near-black background
    };

    // Seed BFS queue with outer border pixels
    for ( int x = 0; x < width; ++x )
    {
        if ( is_black_pixel( x, 0 ) ) { is_bg[ 0 * width + x ] = true; q.push( { x, 0 } ); }
        if ( is_black_pixel( x, height - 1 ) ) { is_bg[ ( height - 1 ) * width + x ] = true; q.push( { x, height - 1 } ); }
    }

    for ( int y = 0; y < height; ++y )
    {
        if ( is_black_pixel( 0, y ) && !is_bg[ y * width + 0 ] ) { is_bg[ y * width + 0 ] = true; q.push( { 0, y } ); }
        if ( is_black_pixel( width - 1, y ) && !is_bg[ y * width + ( width - 1 ) ] ) { is_bg[ y * width + ( width - 1 ) ] = true; q.push( { width - 1, y } ); }
    }

    // BFS Flood-Fill to traverse only contiguous exterior black background
    int const dx[] = { 0, 0, -1, 1 };
    int const dy[] = { -1, 1, 0, 0 };

    while ( !q.empty( ) )
    {
        auto [ cx, cy ] = q.front( );
        q.pop( );

        for ( int i = 0; i < 4; ++i )
        {
            int nx = cx + dx[ i ];
            int ny = cy + dy[ i ];

            if ( nx >= 0 && nx < width && ny >= 0 && ny < height )
            {
                int const idx = ny * width + nx;
                if ( !is_bg[ idx ] && is_black_pixel( nx, ny ) )
                {
                    is_bg[ idx ] = true;
                    q.push( { nx, ny } );
                }
            }
        }
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

            float alpha = 1.0f;
            if ( is_bg[ y * width + x ] )
            {
                // Only exterior connected black background is made transparent!
                alpha = 0.0f;
            }

            // Apply exact UI accent color tint grade #7B73AD (r:123, g:115, b:173) matching Window 1
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

    if ( stbi_write_png( "assets/ooo.png", width, height, 4, out_data.data( ), width * 4 ) != 0 )
    {
        std::cout << "BFS Flood-Fill successfully removed ONLY exterior black background! Character body & shirt 100% preserved!" << std::endl;
    }
    else
    {
        std::cerr << "Failed to write assets/ooo.png" << std::endl;
        return 1;
    }

    return 0;
}
