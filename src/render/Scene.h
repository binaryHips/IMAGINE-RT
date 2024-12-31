#pragma once

#include <vector>
#include <string>
#include "src/mesh/Mesh.h"
#include "src/mesh/Sphere.h"
#include "src/mesh/Square.h"
#include <random>
#include "src/render/KDTree.h"

#include <GL/glut.h>


struct RayResult {
    Vec3 color = Vec3(0, 0, 0);
    Vec3 normal = Vec3(0, 0, 0);
    float depth = 0;

    RayResult() = default;
};

// struct light defined in Material


enum INTERSECTION_TYPE {
    INTERSECTION_MESH,
    INTERSECTION_SPHERE,
    INTERSECTION_SQUARE,
    INTERSECTION_LIGHT
};

class RaySceneIntersection{
    public:
        bool intersectionExists;
        unsigned int typeOfIntersectedObject;
        unsigned int objectIndex;
        float t;

        RayTriangleIntersection rayMeshIntersection;
        RaySphereIntersection raySphereIntersection;
        RaySquareIntersection raySquareIntersection;

        RaySceneIntersection() : intersectionExists(false) , t(FLT_MAX) {}

        const Vec3& get_normal() const{
            switch (typeOfIntersectedObject){
                case INTERSECTION_MESH:
                    return rayMeshIntersection.normal;
                case INTERSECTION_SPHERE:
                    return raySphereIntersection.normal;
                case INTERSECTION_SQUARE:
                    return raySquareIntersection.normal;
            }
        }

        const Vec3& get_position() const{
            switch (typeOfIntersectedObject){
                case INTERSECTION_MESH:
                    return rayMeshIntersection.intersection;
                case INTERSECTION_SPHERE:
                    return raySphereIntersection.intersection;
                case INTERSECTION_SQUARE:
                    return raySquareIntersection.intersection;
            }
        }
        const Vec2& get_uv() const{
            switch (typeOfIntersectedObject){
                case INTERSECTION_MESH:
                    return rayMeshIntersection.uv;
                case INTERSECTION_SPHERE:
                    return raySphereIntersection.uv;
                case INTERSECTION_SQUARE:
                    return raySquareIntersection.uv;
            }
        }

};



class Scene {
public:
    std::vector< std::shared_ptr< Material > > materials;
    std::vector< Mesh > meshes;
    std::vector< Sphere > spheres;
    std::vector< Square > squares;
    std::vector< Light > lights;

    bool useKdTree = false;
    KDTree kdTree;

    std::string name = "unnamed scene";

    int addMaterial(const std::shared_ptr < Material > m ){
        materials.push_back(m);
        return materials.size()-1;
    }

    const Material & getMaterial(int i) const {
        return *materials[i];
    }

    Scene() = default;

    void generateKdTree(){
        if (!meshes.empty()){
            useKdTree = true;
            kdTree = KDTree(meshes);
        }
    }

    void print_scene_data(bool remove_old = true) const {

        int tri_count = 0;
        for (const Mesh & mesh:meshes) tri_count += mesh.triangles.size();

        if (remove_old){

            for (int i = 0; i < 8; ++i){
                std::cout << "\r\033[Am\r                                                                                    " << std::flush;
            }
        }

        printf( //Ansi escape codes (it's fucking ugly here but pretty as a print!) TODO maybe use a library for pretty printing idk
"\n\n\033[36m### Scene \033[31m%s\033[36m ###\
\n\t\033[4m|spheres | quads | meshes (triangles) | lights |\
\n\t\033[4m|%7i |%6i |%7i %10i  |%7i |\033[0m\
\n\t\033[36mMesh mode: \033[31m%s\n\
\n\033[36m###--------%s###\033[0m\n",
            name.c_str(),
            spheres.size(), squares.size(), meshes.size(), tri_count, lights.size(),
            useKdTree ? "Scene-wide KdTree" : "AABB",
            std::string(name.size(), '-').c_str()
            );
    }

    void draw() {
        // iterer sur l'ensemble des objets, et faire leur rendu :
        for( unsigned int It = 0 ; It < meshes.size() ; ++It ) {
            Mesh const & mesh = meshes[It];
            mesh.draw(getMaterial(mesh.material_id));
        }
        for( unsigned int It = 0 ; It < spheres.size() ; ++It ) {
            Sphere const & sphere = spheres[It];
            sphere.draw(getMaterial(sphere.material_id));
        }
        for( unsigned int It = 0 ; It < squares.size() ; ++It ) {
            Square const & square = squares[It];
            square.draw(getMaterial(square.material_id));
        }

        for( unsigned int It = 0 ; It < lights.size() ; ++It ) {
            Light const & light = lights[It];
            light.draw(); //for now the light material isn't reworked.
        }
    }

    const Mesh & getObject(int type, int idx) const {

        switch (type){
            case 0:
                return meshes[idx];
            case 1:
                return spheres[idx];
            case 2:
                return squares[idx];
        }
    } 


    RaySceneIntersection computeIntersection(Ray const & ray) const {
        RaySceneIntersection result;
        result.intersectionExists = false;


        float min_dist = std::numeric_limits<float>::infinity();

        // Spheres 
        for (int i = 0; i<spheres.size(); ++i){

            RaySphereIntersection intersection = spheres[i].intersect(ray);

            if (intersection.intersectionExists && intersection.t < min_dist && intersection.t >= MIN_OFFSET_VALUE){
                result.intersectionExists = true;

                min_dist = intersection.t;
                result.t = intersection.t;
                result.raySphereIntersection = intersection;
                result.typeOfIntersectedObject = INTERSECTION_SPHERE;
                result.objectIndex = i;
            }
        }
        // Squares 
        for (int i = 0; i<squares.size(); ++i){

            RaySquareIntersection intersection = squares[i].intersect(ray);

            if (intersection.intersectionExists && intersection.t < min_dist  && intersection.t >= MIN_OFFSET_VALUE && (Vec3::dot(intersection.normal, ray.direction()) < 0)){ // condition for backface culling

                result.intersectionExists = true;

                result.t = intersection.t;
                min_dist = intersection.t;
                result.raySquareIntersection = intersection;
                result.typeOfIntersectedObject = INTERSECTION_SQUARE;
                result.objectIndex = i;
                
            }
        }
        
        // Meshes 

        if (!useKdTree){
            for (int i = 0; i<meshes.size(); ++i){

                RayTriangleIntersection intersection = meshes[i].intersect(ray);

                if (intersection.intersectionExists && intersection.t < min_dist  && intersection.t >= MIN_OFFSET_VALUE){

                    result.intersectionExists = true;

                    result.t = intersection.t;
                    min_dist = intersection.t;
                    result.rayMeshIntersection = intersection;
                    result.typeOfIntersectedObject = INTERSECTION_MESH;
                    result.objectIndex = i;
                    
                }
            }
        } else {
            KDTree::KdIntersectionResult intersection = kdTree.getIntersection(ray);
            if (intersection.triangleIntersection.intersectionExists &&
                intersection.triangleIntersection.t < min_dist  &&
                intersection.triangleIntersection.t >= MIN_OFFSET_VALUE)
                {

                    result.intersectionExists = true;

                    result.t = intersection.triangleIntersection.t;
                    //min_dist = intersection.triangleIntersection.t;
                    result.rayMeshIntersection = intersection.triangleIntersection;
                    result.typeOfIntersectedObject = INTERSECTION_MESH;
                    result.objectIndex = intersection.meshIndex;
                    if (intersection.meshIndex>0)
                        std::cout <<std::endl<< intersection.meshIndex <<  std::endl << std::endl ;
                }
        }
        return result;
    }



    bool computeOcclusion(Ray const & ray, const Light & light) const { // TODO how to compute for transparent objects?

        float dist_to_light = (light.pos - ray.origin()).norm();
        float power = 1.0;
        // Spheres 
        for (int i = 0; i<spheres.size(); ++i){
            
            if (! getMaterial(spheres[i].material_id).casts_shadows ) continue;
            RaySphereIntersection intersection = spheres[i].intersect(ray);

            if (intersection.intersectionExists &&
                intersection.t >= MIN_OFFSET_VALUE &&
                intersection.t < dist_to_light
                ){
                return true;
            }
        }
        // Squares 
        for (int i = 0; i<squares.size(); ++i){

            if (! getMaterial(squares[i].material_id).casts_shadows ) continue;
            RaySquareIntersection intersection = squares[i].intersect(ray);

            if (intersection.intersectionExists &&
                intersection.t >= MIN_OFFSET_VALUE &&
                intersection.t < dist_to_light
                ){
                return true;
            }
        }
        // meshes 
        for (int i = 0; i<squares.size(); ++i){

            if (! getMaterial(squares[i].material_id).casts_shadows ) continue;
            RaySquareIntersection intersection = squares[i].intersect(ray);

            if (intersection.intersectionExists &&
                intersection.t >= MIN_OFFSET_VALUE &&
                intersection.t < dist_to_light
                ){
                return true;
            }
        }
        if (!useKdTree){
            for (int i = 0; i<meshes.size(); ++i){

                RayTriangleIntersection intersection = meshes[i].intersect(ray);

                if (intersection.intersectionExists &&
                intersection.t >= MIN_OFFSET_VALUE &&
                intersection.t < dist_to_light
                ){
                    return true;
                }
            }
        } else {
            KDTree::KdIntersectionResult intersection = kdTree.getIntersection(ray);
            if (intersection.triangleIntersection.intersectionExists &&
                intersection.triangleIntersection.t >= MIN_OFFSET_VALUE &&
                intersection.triangleIntersection.t < dist_to_light
                ){
                    return true;
                }
        }
        return false;
    }
    const int N_OCCLUSION_RAYS = 8;

    void traceOcclusionRays(const Vec3 position, std::vector<float> & res) const {
        for (int l_idx = 0; l_idx < lights.size(); l_idx++){
            const Light & l = lights[l_idx];
            for (int i = 0; i < N_OCCLUSION_RAYS; ++i){

                Vec3 to = l.getRandomTarget() - position;
                if (!computeOcclusion(
                    Ray(position,to /*no need to normalize*/),
                    l
                    )){
                    res[l_idx] += l.powerCorrection / Vec3::dot(l.pos- position, l.pos- position) / (float)N_OCCLUSION_RAYS; // light is an inverse square law
                }
            }
        }
    }

    void rayTraceRecursive( Ray const & ray , RayResult & res, int NRemainingBounces, bool update_depth = true, bool update_normal = true ) const {
        if (NRemainingBounces == 0) return;

        RaySceneIntersection raySceneIntersection = computeIntersection(ray);
        
        if (!raySceneIntersection.intersectionExists){ // if no collision

            // sky
            float a = 0.5*(ray.direction()[1] + 1.0);

            res.color = (1.0-a)*Vec3(1.0, 1.0, 1.0) + a*Vec3(0.5, 0.7, 1.0);
            if (update_depth) res.depth = -1;
            return ;
            
        }

        //if collision

        Vec3 env_contrib(0, 0, 0);
        const Mesh &mesh = getObject(
            raySceneIntersection.typeOfIntersectedObject,
            raySceneIntersection.objectIndex
            );
        const Material& mat = getMaterial(mesh.material_id);

        std::vector<float> lights_contrib(lights.size(), 0.0);

        traceOcclusionRays(raySceneIntersection.get_position(), lights_contrib);

        if (update_depth) res.depth += raySceneIntersection.t;
        if (update_normal) res.normal = raySceneIntersection.get_normal();

        update_depth = update_depth && !mat.casts_shadows; // go through transparent materials?
        update_normal = update_normal && !mat.casts_shadows; // go through transparent materials?

        Vec3 scatter_direction;
        if ( mat.scatter(ray.direction(), raySceneIntersection.get_normal(), scatter_direction) )
            //std::cout << scatter_direction << std::endl;
            rayTraceRecursive(
                Ray(
                    raySceneIntersection.get_position(),
                    scatter_direction
                ),
                res,
                NRemainingBounces-1,
                update_depth,
                update_normal
            );
        res.color += mat.computeColor(LightingData(raySceneIntersection.get_position(), raySceneIntersection.get_normal(), raySceneIntersection.get_uv(), ray.direction(), env_contrib, lights, lights_contrib));



        /*
        std::cout << "\t INTERSECTION \n\t\tpos=(" << raySceneIntersection.get_position() << ")\n\t\tnormal = ("<<raySceneIntersection.get_normal()<< ")" << std::endl;
        std::cout << "\t\t incident ray: " << ray.direction() << std::endl;
        std::cout << "\t\t reflected ray: " << res << std::endl;
        std::cout << "\n\t\t env_contrib: " << env_contrib << std::endl;
        std::cout << "\n\t\t mat_color: " << mat.diffuse_color << std::endl;
        */
    }

    RayResult rayTrace( Ray const & rayStart ) const {

        RayResult v; // struct defined in renderer.h

        rayTraceRecursive(rayStart, v, 10, true, true );

        return v;
    }

};