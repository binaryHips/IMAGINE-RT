#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"
#include <random>

#include <GL/glut.h>


float randomFloat()
{   // in (-1, 1)
    static thread_local std::mt19937 rng(std::random_device{}());
    return (float)(rng()) * (2.0 / (float)(rng.max())) - 1.0;
}

enum LightType {
    LightType_Spherical,
    LightType_Quad
};


struct Light {
    Vec3 material;
    bool isInCamSpace;
    LightType type;

    Vec3 pos;
    float radius;

    Mesh quad;

    float powerCorrection;

    Light() : powerCorrection(1.0) {}

    Vec3 getRandomTarget() const {
        // simple cube light
        return pos + Vec3(randomFloat(), randomFloat(), randomFloat()) * radius;
    }

    void draw() const { // simple debug draw for volume of light

        glPointSize(5);   
        glColor3f(0.1f, 0.1f, 1.0f);
        glBegin(GL_POINTS);

        for (int i = 0; i < 50; ++i){
            glVertex3fv(
                &(getRandomTarget()[0])
            );
        }
        glEnd();
    }
};

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

public:


    Scene() {
    }

    void draw() {
        // iterer sur l'ensemble des objets, et faire leur rendu :
        for( unsigned int It = 0 ; It < meshes.size() ; ++It ) {
            Mesh const & mesh = meshes[It];
            mesh.draw();
        }
        for( unsigned int It = 0 ; It < spheres.size() ; ++It ) {
            Sphere const & sphere = spheres[It];
            sphere.draw();
        }
        for( unsigned int It = 0 ; It < squares.size() ; ++It ) {
            Square const & square = squares[It];
            square.draw();
        }

        for( unsigned int It = 0 ; It < lights.size() ; ++It ) {
            Light const & light = lights[It];
            light.draw();
        }
    }

    Mesh getObject(int type, int idx) const {

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

            if (intersection.intersectionExists && intersection.t < min_dist && intersection.t >= min_offset){
                result.intersectionExists = true;

                min_dist = intersection.t;
                result.raySphereIntersection = intersection;
                result.typeOfIntersectedObject = INTERSECTION_SPHERE;
                result.objectIndex = i;
            }
        }
        // Squares 
        for (int i = 0; i<squares.size(); ++i){

            RaySquareIntersection intersection = squares[i].intersect(ray);

            if (intersection.intersectionExists && intersection.t < min_dist  && intersection.t >= min_offset){

                // condition for backface culling
                if (Vec3::dot(intersection.normal, ray.direction()) > 0) return result;

                result.intersectionExists = true;
                min_dist = intersection.t;
                result.raySquareIntersection = intersection;
                result.typeOfIntersectedObject = INTERSECTION_SQUARE;
                result.objectIndex = i;
                
            }
        }

        return result;
    }



    bool computeOcclusion(Ray const & ray) const { // TODO how to compute for transparent objects?
        const float min_offset = 1e-6;

        float power = 1.0;
        // Spheres 
        for (int i = 0; i<spheres.size(); ++i){
            
            if (spheres[i].material.type == Material_Glass) continue;
            RaySphereIntersection intersection = spheres[i].intersect(ray);

            if (intersection.intersectionExists && intersection.t >= min_offset){
                return true;
            }
        }
        // Squares 
        for (int i = 0; i<squares.size(); ++i){

            RaySquareIntersection intersection = squares[i].intersect(ray);

            if (intersection.intersectionExists && intersection.t >= min_offset){
                return true;
            }
        }

        return false;
    }


const int N_OCCLUSION_RAYS = 6; //6;
    Vec3 rayTraceRecursive( Ray const & ray , int NRemainingBounces ) const {
        
        if (NRemainingBounces == 0) return Vec3(0, 0, 0);

        Material mat;
        Vec3 diffuse_contrib;
        Vec3 env_contrib;

        RaySceneIntersection raySceneIntersection = computeIntersection(ray);
        
        if (raySceneIntersection.intersectionExists){

            Mesh mesh = getObject(
                raySceneIntersection.typeOfIntersectedObject,
                raySceneIntersection.objectIndex
                );
            mat = mesh.material;


            //Blinn-phong
            if (mat.type == Material_Diffuse_Blinn_Phong){
                
                // diffuse
                for (Light l:lights){

                    for (int i = 0; i < N_OCCLUSION_RAYS; ++i){

                        Vec3 to = l.getRandomTarget() - raySceneIntersection.get_position();

                        if (!computeOcclusion(
                            Ray(
                                raySceneIntersection.get_position(),
                                to //no need to normalize
                            )
                            )){
                            diffuse_contrib = l.material * l.powerCorrection * (1.0 / (Vec3::dot(to, to) * (float)N_OCCLUSION_RAYS)); // light is an inverse squared law

                        }
                    }
                }
                diffuse_contrib[0] *= mat.diffuse_color[0];
                diffuse_contrib[1] *= mat.diffuse_color[1];
                diffuse_contrib[2] *= mat.diffuse_color[2];

                env_contrib = rayTraceRecursive(
                    Ray(
                        raySceneIntersection.get_position(),
                        mat.scatter(ray.direction(), raySceneIntersection.get_normal())
                    ),
                    NRemainingBounces-1
                );
                env_contrib *= mat.specular;
                diffuse_contrib *= (1.0- mat.specular);
                //std::cout << "MAT_COLOr  " << mat.diffuse_color <<"  COMPUTED DIFF  " <<  diffuse_contrib << std::endl;
                return env_contrib + diffuse_contrib;

            }

            //other mats
            env_contrib = rayTraceRecursive(
                Ray(
                    raySceneIntersection.get_position(),
                    mat.scatter(ray.direction(), raySceneIntersection.get_normal())
                ),
                NRemainingBounces-1
            );
            /*
            std::cout << "\t INTERSECTION \n\t\tpos=(" << raySceneIntersection.get_position() << ")\n\t\tnormal = ("<<raySceneIntersection.get_normal()<< ")" << std::endl;
            std::cout << "\t\t incident ray: " << ray.direction() << std::endl;
            std::cout << "\t\t reflected ray: " << res << std::endl;
            std::cout << "\n\t\t env_contrib: " << env_contrib << std::endl;
            std::cout << "\n\t\t mat_color: " << mat.diffuse_color << std::endl;
            */

            //std::cout << "  ENV   " << env_contrib << "  DIFF  " <<  diffuse_contrib << std::endl;
            return env_contrib;
        }

        // simple sky
        return Vec3(0, 0, 0);

        Vec3 unit_direction = ray.direction();
        float a = 0.5*(unit_direction[1] + 1.0);
        return (1.0-a)*Vec3(1.0, 1.0, 1.0) + a*Vec3(0.5, 0.7, 1.0);
    }

    Vec3 rayTrace( Ray const & rayStart ) const {

        Vec3 color;
        //std::cout << "\n\nNEW RAY   " << std::endl;
        color = rayTraceRecursive(rayStart , 5 );
        //std::cout << color << std::endl;
        return color;
    }

    void setup_single_sphere() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0. , 0. , 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_color = Vec3( 0.3,1.,0.3 );
            s.material.specular_color = Vec3( 0.2,0.2,0.2 );
            s.material.index_medium = 1.02;
            s.material.shininess = 20;
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
            s.material.diffuse_color = Vec3( 1.0,1.0,1.0 );
            s.material.specular_color = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }

        {
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.rotate_y(30);
            s.translate(Vec3(-1.2, 0., -1.8));
            s.scale(Vec3(1., 1., 1.));
            s.build_arrays();
            s.material.diffuse_color = Vec3( 1.0,0.4,0.6 );
            s.material.specular_color = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
        {
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.rotate_y(-30);
            s.translate(Vec3(1.2, 0., -1.8));
            s.scale(Vec3(1., 1., 1.));
            s.build_arrays();
            s.material.diffuse_color = Vec3( 0.4,0.6,0.8 );
            s.material.specular_color = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
        // added 

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3(0,3,0);
            light.radius = 1.0f;
            light.powerCorrection = 30.0f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

    }

    void setup_single_square() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3(-3,3,3);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        {
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.build_arrays();
            s.material.diffuse_color = Vec3( 0.8,0.8,0.8 );
            s.material.specular_color = Vec3( 0.8,0.8,0.8 );
            s.material.shininess = 20;
        }
    }

    void setup_cornell_box(){
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize( lights.size() + 1 );
            Light & light = lights[lights.size() - 1];
            light.pos = Vec3( 0.0, 1.0, 0.0 );
            light.radius = 2.0f;
            light.powerCorrection = 25.0f;
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
            s.material.diffuse_color = Vec3( 1.,1.,1. );
            s.material.specular_color = Vec3( 1.,1.,1. );
            s.material.shininess = 16;
        }

        { //Left Wall

            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.material.diffuse_color = Vec3( 1.,0.,0. );
            s.material.specular_color = Vec3( 1.,0.,0. );
            s.material.shininess = 16;
        }

        { //Right Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.material.diffuse_color = Vec3( 0.0,1.0,0.0 );
            s.material.specular_color = Vec3( 0.0,1.0,0.0 );
            s.material.shininess = 16;
        }

        { //Floor
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_color = Vec3( 1.0,1.0,1.0 );
            s.material.specular_color = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
        
        { //Ceiling
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.material.diffuse_color = Vec3( 1.0,1.0,1.0 );
            s.material.specular_color = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
        
        
        { //Front Wall
            squares.resize( squares.size() + 1 );
            Square & s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            //s.rotate_y(-180);
            s.build_arrays();
            s.material.diffuse_color = Vec3( 1.0,1.0,1.0 );
            s.material.specular_color = Vec3( 1.0,1.0,1.0 );
            s.material.shininess = 16;
        }
        


        { //GLASS MIRRORED Sphere

            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1.0, -1.25, 0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_color = Vec3( 1.,0.,0. );
            s.material.specular_color = Vec3( 1.,0.,0. );
            s.material.shininess = 16;
            s.material.transparency = 1.0;
        }


        { //GLASS Sphere
            spheres.resize( spheres.size() + 1 );
            Sphere & s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_color = Vec3( 1.,1.,1. );
            s.material.specular_color = Vec3(  1.,1.,1. );
            s.material.shininess = 16;
            s.material.transparency = 0.;
            s.material.index_medium = 1.1;
        }

    }

};



#endif
