#ifndef MATERIAL_H
#define MATERIAL_H

#include "imageLoader.h"
#include "Vec3.h"
#include <cmath>

#include <GL/glut.h>

enum MaterialType {
    Material_Diffuse_Blinn_Phong ,
    Material_Glass,
    Material_Mirror
};



class Material {

    public:
        Vec3 ambient_color;
        Vec3 diffuse_color;
        Vec3 specular_color;



        double shininess;

        float index_medium;
        float transparency;

        MaterialType type;

        Material() {
            type = Material_Diffuse_Blinn_Phong;
            shininess = 0.5;
            transparency = 0.0;
            index_medium = 1.0;
            ambient_color = Vec3(0., 0., 0.);
            diffuse_color = Vec3(1.0, 1.0, 1.0);
        }


        Vec3 scatter(Vec3 incident, Vec3 normal){
            switch (type){

                case Material_Mirror:
                    return incident.reflect(normal);
                
                case Material_Glass:
                    return incident.refract(normal, 1, index_medium);

                case Material_Diffuse_Blinn_Phong:
                    return incident.reflect(normal);


            }
        }
};
#endif