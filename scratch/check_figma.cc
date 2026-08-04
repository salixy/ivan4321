#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"

int main( )
{
    char const* files[] = { "assets/figma1.png", "assets/figma2.png", "assets/figma3.png" };
    for ( char const* f : files )
    {
        int w = 0, h = 0, ch = 0;
        if ( stbi_info( f, &w, &h, &ch ) )
        {
            std::cout << f << ": " << w << "x" << h << " (" << ch << " ch)" << std::endl;
        }
    }
    return 0;
}
