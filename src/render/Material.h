#ifndef MATERIAL_H
#define MATERIAL_H

#include "src/utils/imageLoader.h"
#include "src/utils/Vec3.h"
#include <cmath>
#include <memory>
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

        float transparency;

        bool casts_shadows = true;

        Material() {

            transparency = 0.0;
            ambient_color = Vec3(0., 0., 0.);
            diffuse_color = Vec3(1.0, 1.0, 1.0);
        }

        virtual Vec3 scatter(Vec3 incident, Vec3 normal) const{
            return incident.reflect(normal);
        };

        virtual Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result) const{
            return Vec3(1, 1, 1);
        }
};



class PhongMaterial: public Material{


    public:

        float shininess;

        PhongMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float shininess){
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
            this->shininess = shininess;
        }

        static std::shared_ptr< PhongMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float shininess) {
            return std::make_shared< PhongMaterial >(ambient_color, diffuse_color, specular_color, shininess);
        }

        inline Vec3 scatter(Vec3 incident, Vec3 normal) const override {

            return incident.reflect(normal);
        }

        inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result) const override{
            diffuse_contrib[0] *= diffuse_color[0];
            diffuse_contrib[1] *= diffuse_color[1];
            diffuse_contrib[2] *= diffuse_color[2];

            scatter_result[0] *= specular_color[0];
            scatter_result[1] *= specular_color[1];
            scatter_result[2] *= specular_color[2];

            return ambient_color + diffuse_contrib + scatter_result * shininess;
        }



};

class TexturedMaterial: public Material{


};

class MirrorMaterial: public Material{
public:
        MirrorMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color){
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
        }

        static std::shared_ptr< MirrorMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color) {
            return std::make_shared< MirrorMaterial >(ambient_color, diffuse_color, specular_color);
        }


        inline Vec3 scatter(Vec3 incident, Vec3 normal) const override {

            return incident.reflect(normal);
        }

        inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result) const override{
            return scatter_result;
        }


};

class GlassMaterial: public Material{

    float index_medium;
public:
    GlassMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float index_medium){
        this->ambient_color = ambient_color;
        this->diffuse_color = diffuse_color;
        this->specular_color = specular_color;
        this->index_medium = index_medium;
        this->casts_shadows = false;
    }

    static std::shared_ptr< GlassMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float index_medium) {
        return std::make_shared< GlassMaterial >(ambient_color, diffuse_color, specular_color, index_medium);
    }


    inline Vec3 scatter(Vec3 incident, Vec3 normal) const override {

        return incident.refract(normal, 1.0003, index_medium);
    }

    inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result) const override{
        return scatter_result;
    }
};

#endif