#ifndef MATERIAL_H
#define MATERIAL_H

#include "src/utils/imageLoader.h"
#include "src/utils/Vec3.h"
#include <cmath>
#include <memory>
#include <random>
#include <GL/glut.h>


static float randomUnitFloat()
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

    float powerCorrection;

    Light() : powerCorrection(1.0) {}

    Vec3 getRandomTarget() const {
        // simple cube light
        return pos + Vec3(randomUnitFloat(), randomUnitFloat(), randomUnitFloat()) * radius;
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

        // returns true if the material needs a scatter ray.
        virtual bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const{
            res = incident.reflect(normal);
            return true;
        };

        virtual Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result, const std::vector< Light > & lights) const{
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

        inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {
            return false;
        }

        inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result, const std::vector< Light > & lights) const override{
            diffuse_contrib[0] *= diffuse_color[0];
            diffuse_contrib[1] *= diffuse_color[1];
            diffuse_contrib[2] *= diffuse_color[2];

            scatter_result[0] *= specular_color[0];
            scatter_result[1] *= specular_color[1];
            scatter_result[2] *= specular_color[2];

            return ambient_color + diffuse_contrib + scatter_result * shininess;
        }
};

class ReflectiveMaterial: public Material{


    public:

        float shininess;

        ReflectiveMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float shininess){
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
            this->shininess = shininess;
        }

        static std::shared_ptr< ReflectiveMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float shininess) {
            return std::make_shared< ReflectiveMaterial >(ambient_color, diffuse_color, specular_color, shininess);
        }

        inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {
            res = incident.reflect(normal);
            return true;
        }

        inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result, const std::vector< Light > & lights) const override{
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
public:
        TexturedMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color){
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
        }

        static std::shared_ptr< TexturedMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color) {
            return std::make_shared< TexturedMaterial >(ambient_color, diffuse_color, specular_color);
        }


        inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {

            res = incident.reflect(normal);
            return true;
        }

        inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result, const std::vector< Light > & lights) const override{
            return scatter_result;
        }



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


        inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {

            res = incident.reflect(normal);
            return true;
        }

        inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result, const std::vector< Light > & lights) const override{
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

    inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {
        res = incident.refract(normal, 1.0003, index_medium);
        return true;
    }

    inline Vec3 computeColor(Vec3 diffuse_contrib, Vec3 scatter_result, const std::vector< Light > & lights) const override{
        return scatter_result;
    }
};

#endif