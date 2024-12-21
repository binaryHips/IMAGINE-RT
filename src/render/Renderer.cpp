
#pragma once
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
#include "src/utils/Vec3.h"
#include "Camera.h"
#include "Scene.h"
#include "src/utils/Color.h"

#include "src/utils/matrixUtilities.h"


void Renderer::render(Camera & camera, const Scene & scene, bool export_after /*= true*/){
    std::cout << "Ray tracing a " << w << " x " << h << " (x " << nsamples << " samples) image" << std::endl;
    camera.apply();


    auto start = std::chrono::system_clock::now();

    //ray_trace_from_camera_singlethreaded(*this, scene);
    ray_trace_from_camera_multithreaded(*this, scene);

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::clog <<"\r\tDone in " << elapsed_seconds.count() << "s              " << std::flush << std::endl; //spaces to overwrite

    result_image = image;

    if (!postProcessPipeline.empty()){
        std::cout << "Applying post-processing" << std::endl;
        start = std::chrono::system_clock::now();
        postProcess();
        end = std::chrono::system_clock::now();
        elapsed_seconds = end-start;
        std::clog <<"\r\tDone in " << elapsed_seconds.count() << "s              " << std::flush << std::endl; //spaces to overwrite
    }

    if (export_after) export_to_file();
}

void Renderer::postProcess(){

    for (int i =0; i < postProcessPipeline.size(); ++i){
        postProcessPipeline[i]->apply(*this); // computes the result inside workspace
        result_image = workspace;//std::move(workspace);
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
    RayResult acc;
    for (int y=pos_y; y<pos_y+sizeY; y++){
        for (int x = pos_x; x<pos_x+sizeX; x++) {
            acc = RayResult();
            p = idx_from_coord(x,y , renderer.w);

            for( unsigned int s = 0 ; s < renderer.nsamples ; ++s ) {

                float u = ((float)(x) + (float)(rng())/(float)(rng.max())) / renderer.w;
                float v = ((float)(y) + (float)(rng())/(float)(rng.max())) / renderer.h;
                // this is a random uv that belongs to the pixel xy.
                dir = screen_space_to_worldSpace(inv_model_view.data(), inv_proj.data(), near_far_planes.data(), u,v) - pos;
                
                dir.normalize();
                RayResult res = scene.rayTrace( Ray(pos , dir) );
                acc.color += res.color;
                acc.normal += res.normal;
                acc.depth += res.depth;
            }

            
            renderer.image[p] = Color(acc.color / renderer.nsamples);
            renderer.screen_space_normals[p] = Color(acc.normal / renderer.nsamples);
            renderer.screen_space_depth[p] = acc.depth / renderer.nsamples;
        }
    }
}

void ray_trace_from_camera_multithreaded(Renderer & renderer, const Scene & scene){
    const unsigned int area_size = 60;
    std::cout << "Number of cores:  " << std::thread::hardware_concurrency() << std::endl;

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
    std::clog << "\r\tBlocks remaining: " << n_threads << " / " << n_threads << ' ' << std::flush;
    for (int t = 0; t < n_threads; ++t){
        threads[t].join();
        std::clog << "\r\tBlocks remaining: " << n_threads - t << " / " << n_threads << ' ' << std::flush;
    }
}

//DOESNT FILL SCREEN NORMALS AND DEPTH FOR NOW
void ray_trace_from_camera_singlethreaded(Renderer & renderer, const Scene & scene){

    Vec3 pos , dir;
    RayResult acc;
    size_t p;
    for (int y=0; y<renderer.h; y++){
        std::clog << "\r\tScanlines remaining: " << (renderer.h-y) << ' ' << std::flush;
        for (int x=0; x<renderer.w; x++) {
            acc = RayResult();
            p = idx_from_coord(x,y , renderer.w);
            for( unsigned int s = 0 ; s < renderer.nsamples ; ++s ) {
                float u = ((float)(x) + (float)(rand())/(float)(RAND_MAX)) / renderer.w;
                float v = ((float)(y) + (float)(rand())/(float)(RAND_MAX)) / renderer.h;
                // this is a random uv that belongs to the pixel xy.
                screen_space_to_world_space_ray(u,v,pos,dir);

                RayResult res = scene.rayTrace( Ray(pos , dir) );
                acc.color += res.color;
                acc.normal += res.normal;
                acc.depth += res.depth;
            }

            
            renderer.image[p] = Color(acc.color / renderer.nsamples);
            renderer.screen_space_normals[p] = Color(acc.normal / renderer.nsamples);
            renderer.screen_space_depth[p] = acc.depth / renderer.nsamples;
        }
    }
}

