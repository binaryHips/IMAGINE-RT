#pragma once

#include "src/utils/Vec3.h"
#include <cmath>
#include <memory>
#include <random>
#include <GL/glut.h>
#include "src/utils/Texture.h"



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

struct LightingData{
    Vec3 position;
    Vec3 normal;
    Vec3 tangent;
    Vec3 bitangent;
    Vec2 uv;
    Vec3 view;
    Vec3 scatter_result;
    const std::vector< Light > & lights;
    const std::vector< float > & lights_contrib;

public:
    LightingData(Vec3 position, Vec3 normal, Vec2 uv, Vec3 view, Vec3 scatter_result, const std::vector< Light > & lights, const std::vector< float > & lc)
    : position(position),
    normal(normal),
    uv(uv),
    view(view),
    scatter_result(scatter_result),
    lights(lights),
    lights_contrib(lc)
    {}

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
        virtual ~Material() = default;

        // returns true if the material needs a scatter ray.
        virtual bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const{
            res = incident.reflect(normal);
            return true;
        };


        virtual Vec3 computeColor(const LightingData& l) const{
            return Vec3(1, 1, 1);
        }


};



class PhongMaterial: public Material{

    public:

        float shininess;
        PhongMaterial() = default;
        PhongMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float shininess){
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
            this->shininess = shininess;
            casts_shadows = false; //FIXME
        }

        static std::shared_ptr< PhongMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color, float shininess) {
            return std::make_shared< PhongMaterial >(ambient_color, diffuse_color, specular_color, shininess);
        }

        inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {
            return false;
        }


        inline Vec3 computeColor(const LightingData& l) const override{

            // Phong code given by Kai Nigh and modified by me
            Vec3 result = ambient_color;
            for(int i = 0; i < l.lights.size(); ++i){

                const Light & light = l.lights[i];

                // Diffuse 

                Vec3 lightDir = (light.pos - l.position).normalized();
                float diff = std::max(Vec3::dot(l.normal, lightDir), 0.0f);
                Vec3 diffuse;
                diffuse[0] = light.material[0] * (diff * diffuse_color[0]);
                diffuse[1] = light.material[1] * (diff * diffuse_color[1]);
                diffuse[2] = light.material[2] * (diff * diffuse_color[2]);

                // Specular

                Vec3 reflectDir = lightDir.reflect(l.normal);
                float spec = std::pow(std::max(Vec3::dot(l.view, reflectDir), 0.0f), shininess);
                Vec3 specular = (spec * specular_color);

                result +=  (diffuse + specular) * l.lights_contrib[i];
            }
            

            return result;
        }
};


class TexturedMaterial: public PhongMaterial{
public:
        std::unique_ptr<Texture> albedo_map;
        std::unique_ptr<Texture> normal_map;
        TexturedMaterial() = default;
        
        TexturedMaterial(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color){
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
            casts_shadows = true;
        }

        explicit TexturedMaterial(const std::string & path_to_albedo){
            albedo_map = ImageTexture::fromPPM(path_to_albedo);
            casts_shadows = true;
        }
        
        TexturedMaterial(TexturedMaterial&& other) noexcept
            : albedo_map(move(other.albedo_map)),
            normal_map(move(other.normal_map))
        {
            this->ambient_color = ambient_color;
            this->diffuse_color = diffuse_color;
            this->specular_color = specular_color;
            casts_shadows = true;
        }

        static std::shared_ptr< TexturedMaterial > create(Vec3 ambient_color, Vec3 diffuse_color, Vec3 specular_color) {
            return std::make_shared< TexturedMaterial >(ambient_color, diffuse_color, specular_color);
        }

        static std::shared_ptr< TexturedMaterial > create(const std::string & path_to_albedo) {
            auto t = std::make_shared< TexturedMaterial >(path_to_albedo);

            return t;
        }

        inline bool scatter(Vec3 incident, Vec3 normal, Vec3 & res) const override {

            res = incident.reflect(normal);
            return false;
        }

        inline Vec3 computeColor(const LightingData & l) const final {

            //Vec3 tex_normal = (normal_map != nullptr)? normal_map->sampleVector(l.uv): ambient_color;
            
            Vec3 diffuse_val = (albedo_map != nullptr)? albedo_map->sampleVector(l.uv): ambient_color;

            Vec3 result = ambient_color;
            for(int i = 0; i < l.lights.size(); ++i){

                const Light & light = l.lights[i];
                // Diffuse 
                Vec3 lightDir = (light.pos - l.position).normalized();
                float diff = std::max(Vec3::dot(l.normal, lightDir), 0.0f);

                Vec3 diffuse = Vec3::compProduct(light.material, diff * diffuse_val);
                // Specular

                Vec3 reflectDir = lightDir.reflect(l.normal);
                float spec = std::pow(std::max(Vec3::dot(l.view, reflectDir), 1.0f), shininess);
                Vec3 specular = (spec * specular_color);

                result +=  (diffuse + specular) * l.lights_contrib[i];
            }

            return result;
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

        inline Vec3 computeColor(const LightingData & l) const override{
            return l.scatter_result;
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

    inline Vec3 computeColor(const LightingData & l) const override{
        return l.scatter_result;
    }
};
