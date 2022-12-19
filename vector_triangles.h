#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "vec3.h"
#include "struct_Triangle.h"


struct intvec3                  //вспомогательная структура
{
    int v, vn, vt;
};

std::vector<Triangle> triangle(std::ifstream& file)
{
    std::vector<Triangle> triangles;
    std::vector<vec3> v;
    std::vector<vec3> vn;
    std::vector<vec3> vt;
    std::vector<intvec3> f;
    std::string str;
        while (std::getline(file, str))
        {
            vec3 r;
            intvec3 v3;
            std::vector<intvec3> f0;
            char const *input = str.c_str();
            if (3==std::sscanf(input, "v %f %f %f", &r.x, &r.y, &r.z))
            {
                v.push_back(r);
            }
            if (3==std::sscanf(input, "vn %f %f %f", &r.x, &r.y, &r.z))
            {
                vn.push_back(r);
            }
            if (3==std::sscanf(input, "vt %f %f %f", &r.x, &r.y, &r.z))
            {
                vt.push_back(r);
            }
            if (input[0] == 'f')
            {
                char const* input2 = input + 2;
                int fs;
                while (3==std::sscanf(input2, "%i/%i/%i%n", &v3.v, &v3.vt, &v3.vn, &fs))
                {
                    f0.push_back(v3);
                    input2 = input2 + fs;
                }
                for (int i = 1; i < f0.size()-1; ++i)
                {
                    triangles.push_back({ v[f0[0].v-1],   v[f0[0+i].v-1],   v[f0[0+i+1].v-1],
                                         vn[f0[0].vn-1], vn[f0[0+i].vn-1], vn[f0[0+i+1].vn-1],
                                         vt[f0[0].vt-1], vt[f0[0+i].vt-1], vt[f0[0+i+1].vt-1]});
                }
            }
        }
    file.close();
    return triangles;
}