#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <array>
#include <iterator>
#include <chrono>
#include <thread>
#include <random>
#include <memory>
#include <algorithm>
#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/Scene.h"
#include <GL/glut.h>

#include "src/matrixUtilities.h"

#include "src/imageLoader.h"



class Color{
public:
    unsigned char r;
    unsigned char g;
    unsigned char b;
    Color(unsigned char red, unsigned char green, unsigned char blue): r(red), g(green), b(blue) {}

    Color(const Vec3 & v){  // maybe saferize it ?

        r = (unsigned char)(v[0] * 255);
        g = (unsigned char)(v[1] * 255);
        b = (unsigned char)(v[2] * 255);
    }
};


class PostProcessEffect{

private:
    //imageData
    int w;
    int h;

    int repeat_mode = 0; // 0 is mirror repeat?

protected:
    inline int idx_from_coord(int x, int y){
        //put here conditions depending on repeat_mode


        return (x + y * w);
    }


    inline Color sampleBuffer(const std::vector< Color > & buffer, int x, int y){

        return buffer[idx_from_coord(x, y)];

    }

    inline void writeToBuffer(std::vector< Color > & buffer, int x, int y, Color elt){

        buffer[idx_from_coord(x, y)] = elt;

    }

public:
    static const bool needs_normals;
    static const bool needs_depth;
    

    std::vector< Vec3 > apply(Renderer & renderer){

    }

};




class Renderer{

private:
    std::vector< Color > image;

    std::vector< Color > screen_space_normals;

    std::vector< Color > screen_space_depth;


    std::vector< PostProcessEffect > postProcessPipeline;

    std::vector< Color > result_image;

    std::vector< Color > workspace;


public:
    int w;
    int h;
    
    unsigned int nsamples; 

    Renderer(int width, int height, unsigned int samples_per_pixel)
        : image( width*height , Vec3(0,0,0) ),
        screen_space_normals( width*height , Vec3(0,0,0) ),
        screen_space_depth( width*height , Vec3(0,0,0) ),
        result_image( width*height , Vec3(0,0,0) ),

        w(width),
        h(height),
        nsamples(samples_per_pixel)
        {}

    void render(Camera & camera, const Scene & scene){
        std::cout << "Ray tracing a " << w << " x " << h << " image" << std::endl;
        camera.apply();
        std::vector< Vec3 > image( w*h , Vec3(0,0,0) );

        auto start = std::chrono::system_clock::now();

        //ray_trace_from_camera_single_threaded(w, h, nsamples, image);
        ray_trace_from_camera_multithreaded(w, h, nsamples, image);

        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;
        std::clog <<  std::flush <<"\tDone in " << elapsed_seconds.count() << "s" <<std::endl;

        }
    
    void postProcess(){
        for (PostProcessEffect effect: postProcessPipeline){
            effect.apply(*this);
        }
    }

    void export_to_file(const std::string & filename = "./rendu.ppm"){
        std::string filename = "./rendu.ppm";
        std::ofstream f(filename.c_str(), std::ios::binary);
        if (f.fail()) {
            std::cout << "Could not open file: " << filename << std::endl;
            return;
        }
        f << "P3" << std::endl << w << " " << h << std::endl << 255 << std::endl;
        for (int i=0; i<w*h; i++)
            f << (int)(255.f*std::min<float>(1.f,result_image[i][0])) << " " << (int)(255.f*std::min<float>(1.f,result_image[i][1])) << " " << (int)(255.f*std::min<float>(1.f,result_image[i][2])) << " ";
        f << std::endl;
        f.close();
    }
};


inline int idx_from_coord(int x, int y, int w){
    return (x + y * w);
}


std::array< GLdouble, 16 > inv_model_view, inv_proj;
std::array< GLdouble, 2 >  near_far_planes;


void ray_trace_square(int w, int h, unsigned int nsamples, Vec3* image, int pos_x, int pos_y, int sizeX, int sizeY){

    static thread_local std::mt19937 rng(std::random_device{}());

    Vec3 pos = cameraSpaceToWorldSpace(inv_model_view.data(), Vec3(0,0,0) );
    Vec3 dir;

    int p;
    for (int y=pos_y; y<pos_y+sizeY; y++){
        for (int x = pos_x; x<pos_x+sizeX; x++) {

            p = idx_from_coord(x,y ,w);
            for( unsigned int s = 0 ; s < nsamples ; ++s ) {

                float u = ((float)(x) + (float)(rng())/(float)(rng.max())) / w;
                float v = ((float)(y) + (float)(rng())/(float)(rng.max())) / h;
                // this is a random uv that belongs to the pixel xy.
                dir = screen_space_to_worldSpace(inv_model_view.data(), inv_proj.data(), near_far_planes.data(), u,v) - pos;
                dir.normalize();
                Vec3 color = scenes[selected_scene].rayTrace( Ray(pos , dir) );
                image[p] += color;
            }
            image[p] /= nsamples;
        }
    }
}

void ray_trace_from_camera_multithreaded(int w, int h, unsigned int nsamples, std::vector < Vec3 > & image){
    const unsigned int area_size = 60;
    //std::cout << "conc  " << std::thread::hardware_concurrency() << std::endl;
    getInvModelView(inv_model_view.data());
    getInvProj(inv_proj.data());
    getNearAndFarPlanes(near_far_planes.data());
    glMatrixMode (GL_MODELVIEW);
    
    int n_square_x = h / area_size;
    int rest_x = w % area_size;

    int n_square_y = h / area_size;
    int rest_y = h % area_size;

    //TODO hello
    int n_threads = (n_square_x+1) * (n_square_y+1);
    /*
    std::cout << "nX: " << n_square_x << "   nY: " << n_square_y << std::endl;
    std::cout << "rX: " << rest_x << "   rY: " << rest_y << std::endl;
    */

    std::thread threads[n_threads];

    //std::shared_ptr<Vec3[]> im_ptr(&image[0]);
    // full squares
    for (int i = 0; i < n_square_x; i+=1){
        for (int j = 0; j < n_square_y; j+=1){
            
            threads[idx_from_coord(i, j, n_square_x+1)] = std::thread(
                ray_trace_square, w, h, nsamples,  &image[0],
                area_size * i, area_size * j, // pos of square (top left corner)
                area_size, area_size // size of square
            );
        }
    }
    
    // bottom rest
    int j_last = (n_square_y);
    for (int i = 0; i < n_square_x; i+=1){

        threads[idx_from_coord(i, j_last, n_square_x+1)] = std::thread(
            ray_trace_square, w, h, nsamples, &image[0],
            area_size * i, area_size * j_last, // pos of square (top left corner)
            area_size, rest_y // size of square
        );

    }
    // right rest
    int i_last = (n_square_x);
    for (int j = 0; j < n_square_y; j+=1){

        threads[idx_from_coord(i_last, j, n_square_x+1)] = std::thread(
            ray_trace_square, w, h, nsamples, &image[0],
            area_size * i_last, area_size * j, // pos of square (top left corner)
            rest_x, area_size // size of square
        );
    }

    // very last, bottom right square
    threads[idx_from_coord(i_last, j_last, n_square_x+1)] = std::thread(
        ray_trace_square, w, h, nsamples, &image[0],
        area_size * i_last, area_size * j_last, // pos of square (top left corner)
        rest_x, rest_y // size of square
    );
    
    // technically not an accurate countdown because threads
    std::clog << "\rBlocks remaining: " << n_threads << " / " << n_threads << ' ' << std::flush;
    for (int t = 0; t < n_threads; ++t){
        threads[t].join();
        std::clog << "\rBlocks remaining: " << n_threads - t << " / " << n_threads << ' ' << std::flush;
    }
    std::cout << std::endl;

}

void ray_trace_from_camera_single_threaded(int w, int h, unsigned int nsamples, std::vector < Vec3 > & image){
    Vec3 pos , dir;
    for (int y=0; y<h; y++){
        std::clog << "\rScanlines remaining: " << (h-y) << ' ' << std::flush;
        for (int x=0; x<w; x++) {
            for( unsigned int s = 0 ; s < nsamples ; ++s ) {
                float u = ((float)(x) + (float)(rand())/(float)(RAND_MAX)) / w;
                float v = ((float)(y) + (float)(rand())/(float)(RAND_MAX)) / h;
                // this is a random uv that belongs to the pixel xy.
                screen_space_to_world_space_ray(u,v,pos,dir);
                Vec3 color = scenes[selected_scene].rayTrace( Ray(pos , dir) );
                image[x + y*w] += color;
            }
            image[x + y*w] /= nsamples;
        }
    }
}