#include <iostream>
#include <vector>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"

namespace fs = std::filesystem;

int main( )
{
    std::string dir_path = "C:/Users/abile/.gemini/antigravity-ide/brain/416e8e42-d40c-4639-9bf6-faada125089e";
    for ( auto const& entry : fs::directory_iterator( dir_path ) )
    {
        if ( entry.is_regular_file( ) )
        {
            std::string path = entry.path( ).string( );
            int w = 0, h = 0, ch = 0;
            if ( stbi_info( path.c_str( ), &w, &h, &ch ) )
            {
                std::cout << entry.path( ).filename( ).string( ) << " -> " << w << "x" << h << " (" << ch << " channels)" << std::endl;
            }
        }
    }
    return 0;
}
