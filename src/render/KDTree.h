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
            
            SpPointer & child_1 = (current->first_side_child);
            SpPointer & child_2 = (current->second_side_child);

            // TODO maybe do a branchless version based on https://stackoverflow.com/questions/38798841/how-do-i-write-a-branchless-stdvector-scan
            for (KdTriangle* t_p: current->tris){

                if (t_p->AABB_v2[axis] <= coord_mean){        // whole triangle below the mean
                    child_1->add_tri(t_p);
                } else if (t_p->AABB_v1[axis] >= coord_mean){ // whold triangle above the mean
                    child_2->add_tri(t_p);
                } else {                                      // neither lol
                    child_1->add_tri(t_p);
                    child_2->add_tri(t_p);
                }
            }

            // prepare next iteration

            to_process.push_back(current->first_side_child);
            to_process.push_back(current->second_side_child);
            axis = (axis+1)%3;
        }
    }

    RayTriangleIntersection getIntersection(const Ray & r){
        return root->getIntersection(r);
    }
};



class KDTree::SplittingPlane {
private:
    bool collideAABB(const Ray & r) const { // https://tavianator.com/2011/ray_box.html

        // TODO optimiser ces soustractions en boucle
        float tx1 = (AABB_v1[0] - r.origin()[0]) / r.direction()[0];
        float tx2 = (AABB_v2[0] - r.origin()[0]) / r.direction()[0];

        float tmin = std::min(tx1, tx2);
        float tmax = std::max(tx1, tx2);

        float ty1 = (AABB_v1[1]- r.origin()[1]) / r.direction()[1];
        float ty2 = (AABB_v2[1]- r.origin()[1]) / r.direction()[1];

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        float tz1 = (AABB_v1[2]- r.origin()[2]) / r.direction()[2];
        float tz2 = (AABB_v2[2]- r.origin()[2]) / r.direction()[2];

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        return tmax >= tmin;
    }
public:

    SpPointer first_side_child;
    SpPointer second_side_child;

    bool is_leaf = false;

    std::vector< KdTriangle* > tris;

    Vec3 AABB_v1 = Vec3(FLT_MAX);
    Vec3 AABB_v2 = Vec3(-FLT_MAX);

    SplittingPlane() = default;

    inline void add_tri(KdTriangle* tri){

        tris.push_back(tri);

        AABB_v1[0] = std::min(tri->AABB_v1[0], AABB_v1[0]); AABB_v1[1] = std::min(tri->AABB_v1[1], AABB_v1[1]); AABB_v1[2] = std::min(tri->AABB_v1[2], AABB_v1[2]);     
        AABB_v2[0] = std::max(tri->AABB_v2[0], AABB_v2[0]); AABB_v2[1] = std::max(tri->AABB_v2[1], AABB_v2[1]); AABB_v2[2] = std::max(tri->AABB_v2[2], AABB_v2[2]);
    }

    RayTriangleIntersection getIntersection(const Ray & r) const { // trying to go branchless while staying clear? idk if it helps performance here
        if (is_leaf){
            RayTriangleIntersection result;
            RayTriangleIntersection candidate;

            result.t = FLT_MAX;

            for (const KdTriangle * tri: tris){
                candidate = tri->triangle.getIntersection(r);
                result = (candidate.intersectionExists && candidate.t <= result.t)? candidate : result;
            }
            return result;
        }
        else{
            RayTriangleIntersection res1 = first_side_child->getIntersection(r);
            RayTriangleIntersection res2 = second_side_child->getIntersection(r);

            // if res1 is the winner or res2 doesnt collide anyway, we return res1. 
            // if res2 wins or res1 doesnt collide, and res2 has an intersection, then res2
            return (res1.t < res2.t && res1.intersectionExists || !res2.intersectionExists)? res1 : res2;

        }

    }


};
