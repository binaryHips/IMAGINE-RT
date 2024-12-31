#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <algorithm>
#include <thread>
#include <random>
#include "src/utils/Vec3.h"
#include "src/utils/Color.h"

#include "Postprocess.h"
#include "Renderer.h"

static int total_threads_n;
static int count = 0;
static inline void print_advancement(){
    std::cout << "\r\t\033[36mBlocks remaining: \033[31m" << total_threads_n - count << " / " << total_threads_n << "            \033[36m" << std::flush;
}

inline int thread_idx_from_coord(int x, int y, int w){
    return (x + y * w);
}

void postProcessSquare(Renderer & renderer, PostProcessEffect & posteffect, int pos_x, int pos_y, int sizeX, int sizeY, std::mutex & mtx){


    for (int u=pos_y; u<pos_y+sizeY; u++){
        for (int v = pos_x; v<pos_x+sizeX; v++) {
            Vec3 out(0, 0, 0);
            posteffect.fragment(
                u, v, out,
                renderer.result_image,
                renderer.image,
                renderer.screen_space_normals,
                renderer.screen_space_depth
            );
            renderer.workspace[u + v * posteffect.w] = Color(out);
        }
    }
    std::scoped_lock<std::mutex> lock(mtx);
    ++count;
    if (!renderer.silent) print_advancement();
}

void PostProcessEffect::postProcessMultithreaded(Renderer & renderer){
    const unsigned int area_size = 40;
    if (!renderer.silent) std::cout << "Number of cores:  \033[31m" << std::thread::hardware_concurrency() << "\033[36m"<< std::endl;

    int n_square_x = renderer.h / area_size;
    int rest_x = renderer.w % area_size;

    int n_square_y = renderer.h / area_size;
    int rest_y = renderer.h % area_size;

    int n_threads = (n_square_x+1) * (n_square_y+1);

    std::thread threads[n_threads];

    std::mutex threads_finished_count_mutex;

    total_threads_n = n_threads;
    count = 0;
    if (!renderer.silent) print_advancement();

    // full squares
    for (int i = 0; i < n_square_x; i+=1){
        for (int j = 0; j < n_square_y; j+=1){
            
            threads[thread_idx_from_coord(i, j, n_square_x+1)] = std::thread(
                postProcessSquare,
                std::ref(renderer), std::ref(*this),
                area_size * i, area_size * j, // pos of square (top left corner)
                area_size, area_size, // size of square
                std::ref(threads_finished_count_mutex)
            );
        }
    }
    
    // bottom rest
    int j_last = (n_square_y);
    for (int i = 0; i < n_square_x; i+=1){

        threads[thread_idx_from_coord(i, j_last, n_square_x+1)] = std::thread(
            postProcessSquare,
            std::ref(renderer), std::ref(*this),
            area_size * i, area_size * j_last, // pos of square (top left corner)
            area_size, rest_y, // size of square
            std::ref(threads_finished_count_mutex)
        );

    }
    // right rest
    int i_last = (n_square_x);
    for (int j = 0; j < n_square_y; j+=1){

        threads[thread_idx_from_coord(i_last, j, n_square_x+1)] = std::thread(
            postProcessSquare,
            std::ref(renderer), std::ref(*this),
            area_size * i_last, area_size * j, // pos of square (top left corner)
            rest_x, area_size, // size of square
            std::ref(threads_finished_count_mutex)
        );
    }

    // very last, bottom right square
    threads[thread_idx_from_coord(i_last, j_last, n_square_x+1)] = std::thread(
        postProcessSquare,
        std::ref(renderer), std::ref(*this),
        area_size * i_last, area_size * j_last, // pos of square (top left corner)
        rest_x, rest_y, // size of square
        std::ref(threads_finished_count_mutex)
    );
    
    for (int t = 0; t < n_threads; ++t){
        threads[t].join();
    }
}


void PostProcessEffect::postProcessSinglethreaded(Renderer& renderer){
    for (int v = 0; v < h; ++v){

        if (!renderer.silent) std::clog << "\r\tScanlines remaining: " << (renderer.h-v) << ' ' << std::flush;

        Vec3 out(0, 0, 0);
        for (int u = 0; u < w; ++u){
            fragment(
                u, v, out,
                renderer.result_image,
                renderer.image,
                renderer.screen_space_normals,
                renderer.screen_space_depth
            );
            renderer.workspace[u + v * w] = Color(out);
        }
    }
}

void PostProcessEffect::apply(Renderer & renderer){
    w = renderer.w;
    h = renderer.h;

    
    //postProcessSinglethreaded(renderer);
    postProcessMultithreaded(renderer);

}



inline int PostProcessEffect::idx_from_coord(int u, int v) const{


    u = u % (w * 2 );
    v = v % (h * 2 );

    if (u<0) u += w;
    if (v<0) v += h;

    if (u >= w) u = 2*w - u;
    if (v >= h) v = 2*h - v;
    return u + v * w;
}
inline Vec3 PostProcessEffect::sampleBuffer(const std::vector< Color > & buffer, int x, int y) const {
    auto v = buffer[idx_from_coord(x, y)];

    return Vec3(v.r / 255.0, v.g / 255.0, v.b / 255.0);
}

inline float PostProcessEffect::sampleBuffer(const std::vector< float > & buffer, int x, int y) const{
    return buffer[idx_from_coord(x, y)];
}

// not private juste in case but don't go around writing in the wrong buffers!
inline void PostProcessEffect::writeToBuffer(std::vector< Color > & buffer, int x, int y, Color elt) const {

    buffer[idx_from_coord(x, y)] = elt;
}





// effects


void postprocess::blur::Cross_blur::FRAGMENT{
    OUT = sampleBuffer(IMAGE, u, v);

    for (int i = 1; i <= size; ++i){
        OUT += sampleBuffer(IMAGE, u+i, v);
        OUT += sampleBuffer(IMAGE, u-i, v);
        OUT += sampleBuffer(IMAGE, u, v+i);
        OUT += sampleBuffer(IMAGE, u, v-i);
    }
    OUT /= 4 * size + 1 ;
}

void postprocess::blur::Convolve::FRAGMENT{ //TODO vérifier que ça marche
    int size_y = kernel.size()/size_x;
    
    Vec3 s(0.0, 0.0, 0.0);
    for (int v_i = 0; v_i < size_x; ++v_i){
        for (int v_j = 0; v_j < size_y; ++v_j){
            
            s += sampleBuffer(IMAGE, u + (v_i - size_x/2), v + (v_j - size_y/2)) * kernel[v_i + v_j * size_x];
        }
    }
    OUT = s;
}



void postprocess::color::Contrast::FRAGMENT{
    OUT = sampleBuffer(IMAGE, u, v);
    // first clamp it
    OUT[0] = std::clamp(OUT[0], 0.0f, 1.0f);
    OUT[1] = std::clamp(OUT[1], 0.0f, 1.0f);
    OUT[2] = std::clamp(OUT[2], 0.0f, 1.0f);
    //contrast
    OUT[0] = val * (OUT[0]-0.5) + 0.5;
    OUT[1] = val * (OUT[1]-0.5) + 0.5;
    OUT[2] = val * (OUT[2]-0.5) + 0.5;
}


void postprocess::color::Value::FRAGMENT{
    OUT = sampleBuffer(IMAGE, u, v) * val;
}




void postprocess::utils::Depth::FRAGMENT{

    if (maxDepth == -1) maxDepth = *std::max_element(DEPTH.begin(), DEPTH.end());

    float res = sampleBuffer(DEPTH, u, v);

    if (res < 0){  //sky
        OUT = Vec3(0, 0, 0);
        return;
    }

    res = 1- res / maxDepth;
    OUT = Vec3(res, res, res);
}

void postprocess::utils::Normals::FRAGMENT{

    OUT = sampleBuffer(NORMAL, u, v);
}


void postprocess::denoise::Similarity::FRAGMENT{

    static const int size = 2;
    static const float norm_thresh = 0.8f;
    static const float color_thresh = 0.3f;
    static const float depth_thresh = 0.2f;

    OUT = sampleBuffer(IMAGE, u, v);


    Vec3 mean_color_around  = OUT;
    Vec3 n = sampleBuffer(NORMAL, u, v);
    float d = sampleBuffer(DEPTH, u, v);
    float dist_to_mean_luminance = 0;

    int count = 1;

    for (int i = -size; i <= size; ++i) for (int j = -size; j <= size; ++j){
        auto col = sampleBuffer(IMAGE, u+i, v+j);
        auto col_diff = col - OUT;
        if (
            abs(sampleBuffer(DEPTH, u+i, v+j) -d) <= depth_thresh&&
            //abs(Vec3::dot(sampleBuffer(NORMAL, u+i, v+j), n)) >= norm_thresh &&
            Vec3::dot(col_diff, col_diff)<= std::pow(color_thresh,2.0) ||
            (abs(i) + abs(j) < 2) // AA
        ){
            mean_color_around += col;
            dist_to_mean_luminance += col.luminance();
            ++count;
        }
    }
    mean_color_around /= (float)count;
    dist_to_mean_luminance = OUT.luminance() - (dist_to_mean_luminance / count);

    OUT = OUT * (1.0f - fac) + mean_color_around * fac;
    
}
