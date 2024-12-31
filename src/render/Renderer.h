#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <thread>
#include <random>
#include <memory>
#include <mutex>
#include "src/utils/Vec3.h"
#include "src/render/Camera.h"
#include "src/render/Scene.h"
#include "src/utils/Color.h"

#include "Postprocess.h"




class Renderer{

    friend void ray_trace_square(Renderer & renderer, const Scene & scene, int pos_x, int pos_y, int sizeX, int sizeY, std::mutex & mtx);
    friend void ray_trace_from_camera_multithreaded(Renderer & renderer, const Scene & scene);
    friend void ray_trace_from_camera_singlethreaded(Renderer & renderer, const Scene & scene);

    friend PostProcessEffect;
    friend void postProcessSquare(Renderer & renderer, PostProcessEffect & posteffect, int pos_x, int pos_y, int sizeX, int sizeY, std::mutex & mtx); // needed in postprocess

private:


    std::vector< Color > image;

    std::vector< Color > screen_space_normals;

    std::vector< float > screen_space_depth;


    std::vector< PostProcessEffect*> postProcessPipeline;

    std::vector< Color > result_image;

    std::vector< Color > workspace;
    std::vector< int > test = {1, 2, 3};

public:
    int w;
    int h;

    bool silent = false;
    
    unsigned int nsamples; 

    Renderer()
        : image( 480*480 , Vec3(0,0,0) ),
        screen_space_normals( 480*480 , Vec3(0,0,0) ),
        screen_space_depth( 480*480, INFINITY ),
        result_image( 480*480 , Vec3(0,0,0) ),
        workspace( 480*480 , Vec3(0,0,0) ),

        w(480),
        h(480),
        nsamples(50)
        {}
    
    Renderer(int width, int height, unsigned int samples_per_pixel)
        : image( width*height , Vec3(0,0,0) ),
        screen_space_normals( width*height , Vec3(0,0,0) ),
        screen_space_depth( width*height, INFINITY ),
        result_image( width*height , Vec3(0,0,0) ),
        workspace( width*height , Vec3(0,0,0) ),

        w(width),
        h(height),
        nsamples(samples_per_pixel)
        {}

    void render(Camera & camera, const Scene & scene, bool export_after = true);
    
    void postProcess();

    void export_to_file(const std::string & filename = "./rendu.ppm");

    std::vector< Color >& getImage(){ return result_image;}
    
    friend Renderer & operator<<(Renderer& renderer, PostProcessEffect* pp) {
        renderer.postProcessPipeline.push_back(
            pp
        );
        return renderer;
    }
};


