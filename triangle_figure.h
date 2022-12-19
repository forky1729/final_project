#pragma once
#include "ray.h"
#include "struct_Triangle.h"

struct RayTriangleIntersection
{
    float t, p, q;
};


inline bool happened(RayTriangleIntersection const &i)
{
    return /*i.t >= 1e-4f
        &&*/ i.p >= 0.f
        && i.q >= 0.f
        && i.p + i.q <= 1.f;
}

bool operator<(RayTriangleIntersection const &i1, RayTriangleIntersection const &i2)
{
    if(!happened(i1))
        return false;
    if(!happened(i2))
        return true;
    return i1.t < i2.t;
}

// ray: r = o + d * t
// triangle: r - r0 = p * (r1 - r0) + q * (r2 - r0)
// substitute: o + d * t - r0 = p * (r1 - r0) + q * (r2 - r0)
// o - r0 = -d * t + p * (r1 - r0) + q * (r2 - r0)
// c = -d * t + p * a + q * b
// mat3(-d, a, b) * vec3(t, p, q) = c

inline RayTriangleIntersection rayTriangleIntersection(Ray const &ray, Triangle const &triangle)
{
    vec3 const a = triangle.r1 - triangle.r0;
    vec3 const b = triangle.r2 - triangle.r0;
    vec3 const c = ray.origin - triangle.r0;
    vec3 const d = ray.direction;
    float const det0 = dot(-d, cross(a, b));
    float const det1 = dot( c, cross(a, b));
    float const det2 = dot(-d, cross(c, b));
    float const det3 = dot(-d, cross(a, c));
    return 
    {
        .t = det1 / det0, //коэффициент при луче
        .p = det2 / det0, //коэффициент при первом векторе треугольника
        .q = det3 / det0, //коэффициент при втором векторе треугольника
    };
}