#pragma once

#include <vector>
#include <memory>
#include "src/mesh/Mesh.h"
#include "src/mesh/Triangle.h"
#include <utility>
#include <functional>
#include <array>

static inline void set_AABB(const Triangle& t, Vec3 & res1, Vec3 & res2){

    res1 = t[0];
    res2 = t[0];
    for (const Vec3 & vertex: {t[1], t[2]}){
        res1[0] = std::min(vertex[0], res1[0]); res1[1] = std::min(vertex[1], res1[1]); res1[2] = std::min(vertex[2], res1[2]);     
        res2[0] = std::max(vertex[0], res2[0]); res2[1] = std::max(vertex[1], res2[1]); res2[2] = std::max(vertex[2], res2[2]);
    }
}


class KDTree{
protected:
    class SplittingPlane;
    struct KdTriangle {Triangle triangle; Vec3 AABB_v1; Vec3 AABB_v2; const int meshIndex;};

    using SpPointer = std::unique_ptr<SplittingPlane>;

    static const int MAX_TRIANGLES_PER_LEAF = 3; //optimum apparement
public:
    struct KdIntersectionResult {RayTriangleIntersection triangleIntersection; int meshIndex;};
    
    SpPointer root = std::make_unique<SplittingPlane>();

    std::vector< KdTriangle > tris;

    KDTree() = default;

    explicit KDTree(const std::vector< Mesh >& meshes);

    KdIntersectionResult getIntersection(const Ray & r) const;
};



class KDTree::SplittingPlane {
private:
    bool collideAABB(const Ray & r) const;
public:

    SpPointer first_side_child;
    SpPointer second_side_child;

    bool is_leaf = false;
    int axis = 0;

    std::vector< KdTriangle* > tris;

    Vec3 AABB_v1 = Vec3(FLT_MAX);
    Vec3 AABB_v2 = Vec3(-FLT_MAX);

    SplittingPlane() = default;

    void add_tri(KdTriangle* tri);

    KdIntersectionResult getIntersection(const Ray & r) const;


};
