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
    for (const Vec3 & vertex: {t[0], t[1]}){
        res1[0] = std::min(vertex[0], res1[0]); res1[1] = std::min(vertex[1], res1[1]); res1[2] = std::min(vertex[2], res1[2]);     
        res2[0] = std::min(vertex[0], res2[0]); res2[1] = std::min(vertex[1], res2[1]); res2[2] = std::min(vertex[2], res2[2]);
    }
}


class KDTree{
protected:
    class SplittingPlane;
    struct KdTriangle {Triangle triangle; Vec3 AABB_v1; Vec3 AABB_v2; const Mesh& mesh;};

    using SpPointer = std::unique_ptr<SplittingPlane>;

public:
    
    SpPointer root;

    std::vector< KdTriangle > tris;

    explicit KDTree(const std::vector< Mesh >& meshes){

        // build the array
        Vec3 AABB_v1;
        Vec3 AABB_v2;

        for (const Mesh & mesh: meshes) for (const MeshTriangle & tri: mesh.triangles){
            Triangle t(
                mesh.vertices[ tri.v[0] ].position,
                mesh.vertices[ tri.v[1] ].position,
                mesh.vertices[ tri.v[2] ].position
            );

            set_AABB(t, AABB_v1, AABB_v2);

            tris.push_back({t, AABB_v1, AABB_v2, mesh});
            root->tris.push_back(&tris[tris.size()-1]);
        }

        int axis = 0;


        std::vector < std::reference_wrapper<SpPointer> > to_process = {root};
        while (! to_process.empty()){
            SpPointer & current = to_process.back(); to_process.pop_back();


            // compute midpoint
            float coord_mean = 0.0;
            for (KdTriangle* t_p: current->tris){
                coord_mean += t_p->triangle[0][axis];
                coord_mean += t_p->triangle[1][axis];
                coord_mean += t_p->triangle[2][axis];
            }
            coord_mean /= (current->tris.size() * 3.0);

            // separate the triangles
            
            SplittingPlane & child_1 = *(current->first_side_child);
            SplittingPlane & child_2 = *(current->first_side_child);

            

            // prepare next iteration

            to_process.push_back(current->first_side_child);
            to_process.push_back(current->second_side_child);
            axis = (axis+1)%3;
        }
    }
};



class KDTree::SplittingPlane {
public:

    SpPointer first_side_child;
    SpPointer second_side_child;

    bool is_leaf = false;

    std::vector< KdTriangle* > tris;

    Vec3 AABB_v1 = Vec3(FLT_MAX);
    Vec3 AABB_v2 = Vec3(FLT_MIN);

    SplittingPlane() = default;


};
