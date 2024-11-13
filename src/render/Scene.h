#pragma once

#include <vector>
#include <string>
#include "src/mesh/Mesh.h"
#include "src/mesh/Sphere.h"
#include "src/mesh/Square.h"
#include <random>


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

};



class Scene {
    std::vector< Mesh > meshes;
    std::vector< Sphere > spheres;
    std::vector< Square > squares;
    std::vector< Light > lights;

    std::vector< std::shared_ptr< Material > > materials;

public:

    int addMaterial(const std::shared_ptr < Material > m ){
        materials.push_back(m);
        return materials.size()-1;
    }

    const Material & getMaterial(int i) const {
        return *materials[i];
    }

    Scene() {
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

        const float min_offset = 1e-5;

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
        for (int i = 0; i<meshes.size(); ++i){

            RayTriangleIntersection intersection = meshes[i].intersect(ray);

            if (intersection.intersectionExists && intersection.t < min_dist  && intersection.t >= MIN_OFFSET_VALUE){

                // condition for backface culling
                //if (Vec3::dot(intersection.normal, ray.direction()) > 0) return result;

                result.intersectionExists = true;

                result.t = intersection.t;
                min_dist = intersection.t;
                result.rayMeshIntersection = intersection;
                result.typeOfIntersectedObject = INTERSECTION_MESH;
                result.objectIndex = i;
                
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

        return false;
    }
    const int N_OCCLUSION_RAYS = 10;

    void traceOcclusionRays(const Vec3 position, Vec3 & diffuse_contrib) const {

        for (Light l:lights){

            for (int i = 0; i < N_OCCLUSION_RAYS; ++i){

                Vec3 to = l.getRandomTarget() - position;
                if (!computeOcclusion(
                    Ray(position,to /*no need to normalize*/),
                    l
                    )){
                    diffuse_contrib += l.material * l.powerCorrection / Vec3::dot(l.pos- position, l.pos- position) / (float)N_OCCLUSION_RAYS; // light is an inverse square law
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

        Vec3 diffuse_contrib = Vec3(0, 0, 0);
        Vec3 env_contrib = Vec3(0, 0, 0);

        Mesh mesh = getObject(
            raySceneIntersection.typeOfIntersectedObject,
            raySceneIntersection.objectIndex
            );

        const Material& mat = getMaterial(mesh.material_id);

        if (mat.casts_shadows){
            traceOcclusionRays(raySceneIntersection.get_position(), diffuse_contrib);
        }
        if (update_depth) res.depth += raySceneIntersection.t;
        if (update_normal) res.normal = raySceneIntersection.get_normal();

        update_depth = update_depth && !mat.casts_shadows; // go through transparent materials?
        update_normal = update_normal && !mat.casts_shadows; // go through transparent materials?

        Vec3 scatter_direction;
        if ( mat.scatter(ray.direction(), raySceneIntersection.get_normal(), scatter_direction) )
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
        res.color = mat.computeColor(diffuse_contrib, env_contrib, lights);



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

        rayTraceRecursive(rayStart, v, 5, true, true );

        return v;
    }

    void setup_single_sphere() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        //materials
        int sphereMat = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,1.,0.3 ), Vec3( 0.2,0.2,0.2 ), 0.5
            )
        );

        int planeMat = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.8,0.8,0.8 ), Vec3( 0.2,0.2,0.2 ), 0.0
            )
        );

        {
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0. , 0. , 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material_id = sphereMat;
        }

        // added 

        {
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -1.));
            s.scale(Vec3(4., 4., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMat;
        }
        // added 

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3(0,3,0);
            light.radius = 1.0f;
            light.powerCorrection = 20.0f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

    }

    void setup_single_square() {
        
        //materials


        int planeMat = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.8,0.8,0.8 ), Vec3( 0.2,0.2,0.2 ), 0.0
            )
        );

        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3(-3,3,3);
            light.radius = 2.5f;
            light.powerCorrection = 8.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        /*
        {
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMat;
        }
        */

        //added
        Mesh mesh;
        mesh.loadOFF("/home/e20210002460/Master/Prog 3D/ray_tracing/HAI719I_Raytracer/models/tripod.off");
        mesh.build_arrays();
        //std::cout << mesh.triangles.size() << std::endl;
        mesh.material_id = planeMat;
        meshes.push_back(mesh); // copy but don't care
    }

    void setup_cornell_box(){
        
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        //materials
        int glassSphereMat = addMaterial(
            GlassMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.05
            )
        );

        int mirrorSphereMat = addMaterial(
            MirrorMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,0.3,0.9 ), Vec3( 0.2,0.2,0.2 )
            )
        );

        int planeMatRed = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 0.2
            )
        );

        int planeMatWhite = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,1.0,1.0 ), Vec3( 0.2,0.2,0.2 ), 0.5
            )
        );

        int planeMatGreen = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,1.0,0.3 ), Vec3( 0.2,0.2,0.2 ), 0.2
            )
        );

        int planeMatPurple = addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.8 ), Vec3( 0.2,0.2,0.2 ), 0.1
            )
        );

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( 0.0, 1.0, 0.0 );
            light.radius = 0.5f;
            light.powerCorrection = 5.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        
        { //Back Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWhite;
        }

        { //Left Wall

            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatRed;
        }

        { //Right Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatGreen;
        }

        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWhite;
        }

        { //Ceiling
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatPurple;
        }
        
        { //Front Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWhite;
        }
        


        { //GLASS Sphere

            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1.0, -1.25, 0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material_id = planeMatPurple;
        }


        { //MIRRORED Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material_id = mirrorSphereMat;
        }
    
    }
    

};