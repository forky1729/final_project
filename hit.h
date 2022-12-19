#pragma once
#include "triangle_figure.h"
#include <algorithm>
#include <optional>
#include <vector>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "picture.h"


struct Hit
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
};

std::optional<Hit> closestHit(Ray const &ray, std::vector<Triangle> const &triangle)
{
    std::vector<RayTriangleIntersection> intersection;
    for(auto const &tr : triangle)
    {
        intersection.push_back(rayTriangleIntersection(ray, tr));
    }

    auto const it = std::min_element(intersection.begin(), intersection.end());

    if(false == happened(*it))
        return std::nullopt;
    
    float const t = it->t;
    auto const i = it - intersection.begin();

    return Hit
    {
        .position = ray.point(t),
        .normal   = normalize(ray.point(t) - cross(triangle[i].r0-triangle[i].r1, triangle[i].r2-triangle[i].r1)),
        .albedo   = (triangle[i].t0*(1-it->q-it->p) + triangle[i].t1*it->p + triangle[i].t2*it->q),
    };
}




