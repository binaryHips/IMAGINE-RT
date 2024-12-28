#include "KDTree.h"



KDTree::KDTree(const std::vector< Mesh >& meshes){
    int n_nodes = 0;
    // build the array
    Vec3 AABB_v1;
    Vec3 AABB_v2;

    for (int i = 0; i < meshes.size(); ++i){
        const Mesh & mesh = meshes[i];
        for (const MeshTriangle & tri: mesh.triangles){
            Triangle t(
                mesh.vertices[ tri.v[0] ].position,
                mesh.vertices[ tri.v[1] ].position,
                mesh.vertices[ tri.v[2] ].position
            );

            set_AABB(t, AABB_v1, AABB_v2);

            tris.push_back({t, AABB_v1, AABB_v2, i});
            
        }
    }

    for (int i = 0; i < tris.size(); ++i){
        root->add_tri(&tris[i]);
    }

    std::vector < std::reference_wrapper<SpPointer> > to_process = {root};
    while (! to_process.empty()){
        SpPointer & current = to_process.back(); to_process.pop_back();
        n_nodes++;

        if (current->tris.size() <= MAX_TRIANGLES_PER_LEAF){
            current->is_leaf = true;
            continue;
        }

        int axis = current->axis;
        current->first_side_child = std::make_unique<SplittingPlane>();
        current->second_side_child = std::make_unique<SplittingPlane>();

        // compute midpoint
        float coord_mean = 0.0;
        for (KdTriangle* t_p: current->tris){
            coord_mean += t_p->triangle[0][axis];
            coord_mean += t_p->triangle[1][axis];
            coord_mean += t_p->triangle[2][axis];
        }
        coord_mean /= ((float)current->tris.size() * 3.0f);

        // separate the triangles
        
        SpPointer & child_1 = (current->first_side_child);
        SpPointer & child_2 = (current->second_side_child);
        child_1->axis = child_2->axis = (axis+1)%3;

        // TODO maybe do a branchless version based on https://stackoverflow.com/questions/38798841/how-do-i-write-a-branchless-stdvector-scan
        for (KdTriangle* t_p: current->tris){

            float center = (t_p->triangle[0][axis] + t_p->triangle[1][axis] + t_p->triangle[2][axis])/3.0f;

            if (center <= coord_mean){        // marche avec les triangles à cheval car on update l'AABB du noeud anyway
                child_1->add_tri(t_p);
            } else{
                child_2->add_tri(t_p);
            }
        }

        // safeguard for edge cases (ex: 46 triangles in exactly the same spot...)
        
        if (child_1->tris == current->tris || child_2->tris == current->tris ){
            child_1->is_leaf = child_2->is_leaf = true;

        } else {
            to_process.push_back(current->first_side_child);
            to_process.push_back(current->second_side_child);
        }
    }
    //std::cout << "nodes  " << n_nodes << std::endl;
}

KDTree::KdIntersectionResult KDTree::getIntersection(const Ray & r) const{
    return root->getIntersection(r);
}


inline void KDTree::SplittingPlane::add_tri(KdTriangle* tri){

    tris.push_back(tri);

    AABB_v1[0] = std::min(tri->AABB_v1[0], AABB_v1[0]); AABB_v1[1] = std::min(tri->AABB_v1[1], AABB_v1[1]); AABB_v1[2] = std::min(tri->AABB_v1[2], AABB_v1[2]);     
    AABB_v2[0] = std::max(tri->AABB_v2[0], AABB_v2[0]); AABB_v2[1] = std::max(tri->AABB_v2[1], AABB_v2[1]); AABB_v2[2] = std::max(tri->AABB_v2[2], AABB_v2[2]);
}

bool KDTree::SplittingPlane::collideAABB(const Ray & r) const { // https://tavianator.com/2011/ray_box.html

    // TODO optimiser ces soustractions en boucle
    Vec3 aa1_to_r = AABB_v1 - r.origin();
    Vec3 aa2_to_r = AABB_v2 - r.origin();
    Vec3 facs(1.0/r.direction()[0], 1.0 / r.direction()[1], 1.0 / r.direction()[2]);

    float tx1 = (aa1_to_r[0]) * facs[0];
    float tx2 = (aa2_to_r[0]) * facs[0];

    float tmin = std::min(tx1, tx2);
    float tmax = std::max(tx1, tx2);

    float ty1 = (aa1_to_r[1]) * facs[1];
    float ty2 = (aa2_to_r[1]) * facs[1];

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    float tz1 = (aa1_to_r[2]) * facs[2];
    float tz2 = (aa2_to_r[2]) * facs[2];

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    return tmax >= tmin;
    return tmax >= tmin;
}

const KDTree::KdIntersectionResult final_result;

KDTree::KdIntersectionResult KDTree::SplittingPlane::getIntersection(const Ray & r) const {
    if (!collideAABB(r)) return final_result;

    if (is_leaf){
        RayTriangleIntersection result;
        RayTriangleIntersection candidate;
        int meshIndex;

        for (const KdTriangle * tri: tris){
            candidate = tri->triangle.getIntersection(r);
            if (candidate.intersectionExists && candidate.t < result.t){
                result = candidate;
                meshIndex = tri->meshIndex;
            }
        }
        return {result, meshIndex};
    }
    else{
        KdIntersectionResult res1 = first_side_child->getIntersection(r);
        KdIntersectionResult res2 = second_side_child->getIntersection(r);

        if (!res1.triangleIntersection.intersectionExists) return res2;
        if (!res2.triangleIntersection.intersectionExists) return res1;
        if (res1.triangleIntersection.t <= res2.triangleIntersection.t) return res1; else return res2;

    }

}