#include <iostream>
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb_image.h"

int main( )
{
    char const* files[] = {
        "C:/Users/abile/.gemini/antigravity-ide/brain/416e8e42-d40c-4639-9bf6-faada125089e/uploaded_media_1784995309993.img",
        "C:/Users/abile/.gemini/antigravity-ide/brain/416e8e42-d40c-4639-9bf6-faada125089e/uploaded_media_1785017508748.img",
        "C:/Users/abile/.gemini/antigravity-ide/brain/416e8e42-d40c-4639-9bf6-faada125089e/uploaded_media_1785020997939.img"
    };

    for ( char const* f : files )
    {
        int w = 0, h = 0, ch = 0;
        if ( stbi_info( f, &w, &h, &ch ) )
        {
            printf( "FILE: %s -> %dx%d (%d channels)\n", f, w, h, ch );
            fflush( stdout );
        }
    }
    return 0;
}
