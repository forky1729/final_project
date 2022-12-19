#include <iostream>
#include "vector_triangles.h"
#include "camera.h"
#include "hit.h"
#include "texture.h"
#include <cmath>
#include "tree.h"

int main()
{
    using u32 = unsigned int;

    u32 const width  = 1920u;
    u32 const height = 1080u;

    float const fwidth  =  width;
    float const fheight = height;

    Camera const camera =
    {
        /*
        position — точка, в которой находится камера
        at — точка, на которую камера направлена (то есть, лежащая на оси камеры)
        up — направление "вверх"; по сути, фиксирует собственное вращение вокруг оси камеры
        aspectRatio — соотношение сторон прямоугольника камеры, width / height
        verticalFOV — тангенс угла от оси камеры до верхней стороны прямоугольника камеры
        
        //дельфин
        .position = {200.f, -200.f, 100.f},
        .at = {-200.f, 200.f, 50.f},
        .up = {1.f, -1.f, 4.f},
        .aspectRatio = fwidth / fheight,
        .verticalFOV =0.5f,
        */
        
        .position = {150.f, -150.f, 50.f},
        .at = {-200.f, 200.f, 50.f},
        .up = {1.f, -1.f, 4.f},
        .aspectRatio = fwidth / fheight,
        .verticalFOV =0.5f,
        
    };

    std::ifstream file("Penguin.obj");
    std::vector<Triangle> triangles = triangle(file);

    char const * picture = "Penguin.png";
    int width_png = create_texture(picture).w;
    int height_png = create_texture(picture).h;
    std::vector<Texture> texture = create_texture(picture).texture;

    for (int i=0; i<triangles.size(); ++i)
    {
        float t0x = triangles[i].t0.x;
        float t0y = triangles[i].t0.y;
        triangles[i].t0.x = texture[round(t0x*width_png)+height_png-1-round(t0y*height_png)*width_png].r;
        triangles[i].t0.y = texture[round(t0x*width_png)+height_png-1-round(t0y*height_png)*width_png].g;
        triangles[i].t0.z = texture[round(t0x*width_png)+height_png-1-round(t0y*height_png)*width_png].b;

        float t1x = triangles[i].t1.x;
        float t1y = triangles[i].t1.y;
        triangles[i].t1.x = texture[round(t1x*width_png)+height_png-1-round(t1y*height_png)*width_png].r;
        triangles[i].t1.y = texture[round(t1x*width_png)+height_png-1-round(t1y*height_png)*width_png].g;
        triangles[i].t1.z = texture[round(t1x*width_png)+height_png-1-round(t1y*height_png)*width_png].b;

        float t2x = triangles[i].t2.x;
        float t2y = triangles[i].t2.y;
        triangles[i].t2.x = texture[round(t2x*width_png)+height_png-1-round(t2y*height_png)*width_png].r;
        triangles[i].t2.y = texture[round(t2x*width_png)+height_png-1-round(t2y*height_png)*width_png].g;
        triangles[i].t2.z = texture[round(t2x*width_png)+height_png-1-round(t2y*height_png)*width_png].b;
    }

    auto nodes = MakeNodes(triangles);
    Tree tree = createTree(nodes);

    auto const tonemap = [](vec3 const c)
    {
        float const exposure = 1.f;            //!!!!
        float const almost256 = 255.999f;
        return vec3
        {
            almost256 * (1.f - std::exp(-exposure * c.x)),
            almost256 * (1.f - std::exp(-exposure * c.y)),
            almost256 * (1.f - std::exp(-exposure * c.z)),
        };
    };

    auto const trace = [&tree](Ray const &ray)
    {
        vec3 const lightDir   = normalize({200.f, -200.f, 200.f});
        vec3 const  backColor = {0.9f, 0.9f, 0.9f};
        vec3 const lightColor = {1.00f, 1.00f, 0.97f};

        std::optional<Hit> const hit = closestHit(ray, tree);
        if(!hit)
            return dot(ray.direction, lightDir) < 0.999f
                ? backColor
                : lightColor * 2.f;

        Ray const shadowRay =
        {
            .origin = hit->position,
            .direction = lightDir
        };
        std::optional<Hit> const shadowHit = closestHit(shadowRay, tree);
        float const NL = std::max(0.f, dot(hit->normal, lightDir));

        vec3 const color = !shadowHit
            ? lightColor * NL
            : vec3{0.f, 0.f, 0.f};

        return (backColor * 0.3f + color) * hit->albedo;
    };

    std::cout << "P3\n" << width << " " << height << "\n255\n";
    for(u32 y = 0u; y < height; ++y)
    for(u32 x = 0u; x <  width; ++x)
    {
        float const u = -1.f + 2.f * float(x) / fwidth;
        float const v = -1.f + 2.f * float(y) / fheight;
        float const du = 1.f / fwidth;
        float const dv = 1.f / fheight;

        // 4 rays for antialiasing
        vec3 const color = tonemap
        (
            trace(camera.castRay(u - du, -v - dv))
            + trace(camera.castRay(u - du, -v + dv))
            + trace(camera.castRay(u + du, -v - dv))
            + trace(camera.castRay(u + du, -v + dv))
        );

            std::cout << u32(color.x) << " ";
            std::cout << u32(color.y) << " ";
            std::cout << u32(color.z) << " ";
        }
}