#include "aabb.h"
#include "struct_Triangle.h"
#include "triangle_figure.h"
#include "hit.h"
#include <algorithm>
#include <vector>
#include <span>
#include <variant>
#include <optional>


struct Node {                                                                  //Узел дерева
    AABB box;

    struct Two_boxes{                                                                //Две коробки
        Node *right;
        Node *left;
    };
    using Info = std::variant<Two_boxes, Triangle>;                                  //Две коробки либо треугольник

    Info info;
};

using Tree = Node*;

vec3 rightup(vec3 one, vec3 two){                                              //Функция правого верхнего угла
    return{
        std::max(one.x, two.x),
        std::max(one.y, two.y),
        std::max(one.z, two.z),
    };
};

vec3 leftdown(vec3 one, vec3 two){                                             //Функция левого нижнего угла
    return{
        std::min(one.x, two.x),
        std::min(one.y, two.y),
        std::min(one.z, two.z),
    };
};

AABB Box(Triangle triangle){                                                   //Функция кладения треугольника в коробку

    AABB aabb;

    aabb.max = rightup(rightup(triangle.r0, triangle.r1), triangle.r2);
    aabb.min = leftdown(leftdown(triangle.r0, triangle.r1), triangle.r2);

    return(aabb);
}

// AABB Unite (AABB left, AABB right){                                            //Функция кладения двух корбок в одну
//     return {
//         rightup(left.max, right.max),
//         leftdown(left.min, right.min)
//     };
// }

AABB Unite(std::vector<AABB> vector){                                          //Функция кладения n корбок в одну

    AABB ans = vector[0];

    for(AABB aabb : vector){

        if (aabb.max.x > ans.max.x)
            ans.max.x = aabb.max.x;
        if (aabb.max.y > ans.max.y)
            ans.max.y = aabb.max.y;
        if (aabb.max.z > ans.max.z)
            ans.max.z = aabb.max.z;

        if (aabb.min.x < ans.min.x)
            ans.min.x = aabb.min.x;
        if (aabb.min.y < ans.min.y)
            ans.min.y = aabb.min.y;
        if (aabb.min.z < ans.min.z)
            ans.min.z = aabb.min.z;
    }
    return ans;
}

vec3 Center(AABB aabb){                                                        //Центр коробки
    return (aabb.max + aabb.min) / 2;
}

std::vector<Node> MakeNodes(std::vector<Triangle> triangles){                  //Из вектора треугольников делает узлы
    std::vector<Node> nodes;
    for(Triangle triangle : triangles){
        nodes.push_back({Box(triangle), triangle});
    }
    return nodes;
}

void swap(Node* a, Node* b){                                                   //Меняет узлы по указателям
    Node c = *a;
    *a = *b;
    *b = c;
}

auto partition(auto start, auto finish, auto predicate){                       //Тут (пока не) ясно
    for(auto *box = start; box <= finish; box ++){
        if(predicate(*box)){
            swap(box, start);
            start ++;
        }
    }
    if(!predicate(*start))
        swap(start, finish);
    return start;
}
 
bool LessThan(Node a, Node b, int ax){                                         //Одна коробка левее по оси
    if (ax == 0)
        return Center(a.box).x < Center(b.box).x;
    if (ax == 1)
        return Center(a.box).y < Center(b.box).y;
    return Center(a.box).z < Center(b.box).z;
}
 
bool Equal(Node a, Node b, int ax){                                            //Коробки по оси рядом
    if (ax == 0)
        return Center(a.box).x == Center(b.box).x;
    if (ax == 1)
        return Center(a.box).y == Center(b.box).y;
    return Center(a.box).z == Center(b.box).z;
}

void quickselect(auto start, auto finish, auto ptr, int axis) {                //Ставит коробку на место в списке
    if(finish - start < 2)
        return;
    auto pivot = *start;
    auto mid1 = partition(start, finish, [pivot, axis](const auto& em){return LessThan(em, pivot, axis);});
    auto mid2 = partition(mid1, finish, [pivot, axis](const auto& em){return Equal(em, pivot, axis);});
    if(mid1 > ptr)
        quickselect(start, mid1, ptr, axis);
    else if(mid2 <= ptr)
        quickselect(mid2, finish, ptr, axis);
}

Tree createTree(std::span<Node> nodes){                                      //Из узлов собирает дерево
    
    if(nodes.size() == 1)
        return nodes.data();
    
    std::vector<AABB> boxes;
    for(Node node : nodes)
        boxes.push_back(node.box);
    AABB big_box = Unite(boxes);

    int long_axis;
    if(big_box.max.x - big_box.min.x >= big_box.max.y - big_box.min.y && big_box.max.x - big_box.min.x >= big_box.max.z - big_box.min.z)
        long_axis = 0;
    if(big_box.max.y - big_box.min.y >= big_box.max.x - big_box.min.x && big_box.max.y - big_box.min.y >= big_box.max.z - big_box.min.z)
        long_axis = 1;
    if(big_box.max.z - big_box.min.z >= big_box.max.y - big_box.min.y && big_box.max.z - big_box.min.z >= big_box.max.x - big_box.min.x)
        long_axis = 2;

    auto mid = nodes.begin() + (nodes.end() - nodes.begin()) / 2;
    quickselect(nodes.begin(), nodes.end(), mid, long_axis);
    std::span<Node> left = {nodes.begin(), mid};
    std::span<Node> right = {mid, nodes.end()};
    return new Node {
        .box = big_box,
        .info = Node::Two_boxes{createTree(left), createTree(right)}
    };
}

bool Clother (Ray ray, vec3 point1, vec3 point2){                              //точка ближе к началу луча
    return (length(ray.origin - point1) < length(ray.origin - point2));
}

std::optional<Hit> closestHit(Ray ray,Tree tree){                              //RayTreeIntersection
    if(std::holds_alternative<Triangle>(tree->info)){
        Triangle &tri = std::get<Triangle>(tree->info);
        RayTriangleIntersection const it = rayTriangleIntersection(ray, tri);
        float const t = it.t;
        if(false == happened(it))
            return std::nullopt;
        return Hit
        {
            .position = ray.point(t),
            .normal = normalize(ray.point(t) - cross(tri.r0-tri.r1, tri.r2-tri.r1)),
            .albedo = (tri.t0*(1-it.q-it.p) + tri.t1*it.p + tri.t2*it.q),
        };
    }
    Tree left = std::get<Node::Two_boxes>(tree->info).left;
    Tree right = std::get<Node::Two_boxes>(tree->info).right;

    if(false == happened(rayAABBIntersection(ray, tree->box)))
        return std::nullopt;
    if(false == happened(rayAABBIntersection(ray, left->box)))
        return closestHit(ray, right);
    if(false == happened(rayAABBIntersection(ray, right->box)))
        return closestHit(ray, left);
    if(rayAABBIntersection(ray, left->box).tMin < rayAABBIntersection(ray, right->box).tMin){
        if (!closestHit(ray, left))
            return closestHit(ray, right);
        if (Clother(ray, closestHit(ray, left)->position, ray.point(rayAABBIntersection(ray, right->box).tMin)))
            return closestHit(ray, left);
        if (!closestHit(ray, right))
            return closestHit(ray, left);
        if (Clother(ray, closestHit(ray, left)->position, closestHit(ray, right)->position))
            return closestHit(ray, left);
        return closestHit(ray, right);
    }
    if (!closestHit(ray, right))
        return closestHit(ray, left);
    if (Clother(ray, closestHit(ray, right)->position, ray.point(rayAABBIntersection(ray, left->box).tMin)))
        return closestHit(ray, right);
    if (!closestHit(ray, left))
        return closestHit(ray, right);
    if (Clother(ray, closestHit(ray, right)->position, closestHit(ray, left)->position))
        return closestHit(ray, right);
    return closestHit(ray, left);
}