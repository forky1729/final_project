#include <iostream>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include <vector>

struct Color
{
    unsigned char r, g, b;
};

struct Texture
{
    float r, g, b;
};

struct Return
{
    int w, h;
    std::vector<Texture> texture;
};

Return create_texture(char const * file)
{
    std::vector<Texture> result;
    int w, h, comp;
    unsigned char* image = stbi_load(file, &w, &h, &comp, 3);
    Color *color = reinterpret_cast<Color *>(image);
    for (int i=0; i<h; ++i)
    {
        for (int j=0; j<w; ++j)
        {
            result.push_back({float(static_cast<int>(color[i * w + j].b))/256.f, float(static_cast<int>(color[i * w + j].r))/256.f, float(static_cast<int>(color[i * w + j].g))/256.f});
        }
    }
    return {w, h, result};
}


