
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <functional>
#include <array>
#include <iterator>
#include <chrono>
#include <thread>
#include <random>
#include <memory>
#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/Scene.h"
#include "Color.h"

#include "src/matrixUtilities.h"


void Renderer::render(Camera & camera, const Scene & scene, bool export_after /*= true*/){
    std::cout << "Ray tracing a " << w << " x " << h << " (x " << nsamples << " samples) image" << std::endl;
    camera.apply();


    auto start = std::chrono::system_clock::now();

    //ray_trace_from_camera_singlethreaded(*this, scene);
    ray_trace_from_camera_multithreaded(*this, scene);

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::clog <<  std::flush <<"\tDone in " << elapsed_seconds.count() << "s" <<std::endl;

    result_image = image;
    if (!postProcessPipeline.empty()){
        std::cout << "Applying post-processing" << std::endl;
        start = std::chrono::system_clock::now();
        postProcess();
        end = std::chrono::system_clock::now();
        elapsed_seconds = end-start;
        std::clog <<  std::flush <<"\tDone in " << elapsed_seconds.count() << "s" <<std::endl;
    }

    if (export_after) export_to_file();
}

void Renderer::postProcess(){
    for (int i =0; i < postProcessPipeline.size(); ++i){

        (*postProcessPipeline.at(0)).apply(*this); // computes the result inside workspace
        result_image = std::move(workspace);
        workspace = std::vector< Color >(w*h, Color());
    }
}

void Renderer::export_to_file(const std::string & filename /*= "./rendu.ppm"*/){
    std::ofstream f(filename.c_str(), std::ios::binary);
    if (f.fail()) {
        std::cout << "Could not open file: " << filename << std::endl;
        return;
    }
    f << "P3" << std::endl << w << " " << h << std::endl << 255 << std::endl;
    for (int i=0; i<w*h; i++)
        f << (int)result_image[i][0] << " " << (int)result_image[i][1] << " " << (int)result_image[i][2] << " ";
    f << std::endl;
    f.close();
}

//friends 

inline int idx_from_coord(int x, int y, int w){
    return (x + y * w);
}


std::array< GLdouble, 16 > inv_model_view, inv_proj;
std::array< GLdouble, 2 >  near_far_planes;


void ray_trace_square(Renderer & renderer, const Scene & scene, int pos_x, int pos_y, int sizeX, int sizeY){

    static thread_local std::mt19937 rng(std::random_device{}());

    Vec3 pos = cameraSpaceToWorldSpace(inv_model_view.data(), Vec3(0,0,0) );
    Vec3 dir;
    int p;
    Vec3 acc;
    for (int y=pos_y; y<pos_y+sizeY; y++){
        for (int x = pos_x; x<pos_x+sizeX; x++) {
            acc = Vec3(0, 0, 0);
            p = idx_from_coord(x,y , renderer.w);

            for( unsigned int s = 0 ; s < renderer.nsamples ; ++s ) {

                float u = ((float)(x) + (float)(rng())/(float)(rng.max())) / renderer.w;
                float v = ((float)(y) + (float)(rng())/(float)(rng.max())) / renderer.h;
                // this is a random uv that belongs to the pixel xy.
                dir = screen_space_to_worldSpace(inv_model_view.data(), inv_proj.data(), near_far_planes.data(), u,v) - pos;
                
                dir.normalize();
                Vec3 color = scene.rayTrace( Ray(pos , dir) );
                acc += color;
            }
            renderer.image[p] = Color(acc / renderer.nsamples);
        }
    }
}

void ray_trace_from_camera_multithreaded(Renderer & renderer, const Scene & scene){
    const unsigned int area_size = 60;
    //std::cout << "conc  " << std::thread::hardware_concurrency() << std::endl;

    getInvModelView(inv_model_view.data());
    getInvProj(inv_proj.data());
    getNearAndFarPlanes(near_far_planes.data());
    glMatrixMode (GL_MODELVIEW);
    
    int n_square_x = renderer.h / area_size;
    int rest_x = renderer.w % area_size;

    int n_square_y = renderer.h / area_size;
    int rest_y = renderer.h % area_size;

    int n_threads = (n_square_x+1) * (n_square_y+1);
    /*
    std::cout << "nX: " << n_square_x << "   nY: " << n_square_y << std::endl;
    std::cout << "rX: " << rest_x << "   rY: " << rest_y << std::endl;
    */

    std::thread threads[n_threads];

    // full squares
    for (int i = 0; i < n_square_x; i+=1){
        for (int j = 0; j < n_square_y; j+=1){
            
            threads[idx_from_coord(i, j, n_square_x+1)] = std::thread(
                ray_trace_square,
                std::ref(renderer), std::cref(scene),
                area_size * i, area_size * j, // pos of square (top left corner)
                area_size, area_size // size of square
            );
        }
    }
    
    // bottom rest
    int j_last = (n_square_y);
    for (int i = 0; i < n_square_x; i+=1){

        threads[idx_from_coord(i, j_last, n_square_x+1)] = std::thread(
            ray_trace_square,
            std::ref(renderer), std::cref(scene),
            area_size * i, area_size * j_last, // pos of square (top left corner)
            area_size, rest_y // size of square
        );

    }
    // right rest
    int i_last = (n_square_x);
    for (int j = 0; j < n_square_y; j+=1){

        threads[idx_from_coord(i_last, j, n_square_x+1)] = std::thread(
            ray_trace_square,
            std::ref(renderer), std::cref(scene),
            area_size * i_last, area_size * j, // pos of square (top left corner)
            rest_x, area_size // size of square
        );
    }

    // very last, bottom right square
    threads[idx_from_coord(i_last, j_last, n_square_x+1)] = std::thread(
        ray_trace_square,
        std::ref(renderer), std::cref(scene),
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

void ray_trace_from_camera_singlethreaded(Renderer & renderer, const Scene & scene){

    Vec3 pos , dir;
    for (int y=0; y<renderer.h; y++){
        std::clog << "\rScanlines remaining: " << (renderer.h-y) << ' ' << std::flush;
        for (int x=0; x<renderer.w; x++) {
            for( unsigned int s = 0 ; s < renderer.nsamples ; ++s ) {
                float u = ((float)(x) + (float)(rand())/(float)(RAND_MAX)) / renderer.w;
                float v = ((float)(y) + (float)(rand())/(float)(RAND_MAX)) / renderer.h;
                // this is a random uv that belongs to the pixel xy.
                screen_space_to_world_space_ray(u,v,pos,dir);
                Vec3 color = scene.rayTrace( Ray(pos , dir) );
                renderer.image.at(x + y*renderer.w) += Color(color);
            }
            renderer.image.at(x + y*renderer.w)  /= renderer.nsamples;
        }
    }
}

