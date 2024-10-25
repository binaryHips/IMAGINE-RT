#pragma once


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <thread>
#include <random>
#include "src/utils/Vec3.h"
#include "src/utils/Color.h"


class Renderer;

class PostProcessEffect{


public:
    int repeat_mode = 0; // 0 is mirror repeat?
    int w = 0;
    int h = 0;

    inline int idx_from_coord(int u, int v) const;


    inline Vec3 sampleBuffer(const std::vector< Color > & buffer, int x, int y) const;

    // not private juste in case but don't go around writing in the wrong buffers!
    inline void writeToBuffer(std::vector< Color > & buffer, int x, int y, Color elt) const;


    static const bool needs_normals = false;
    static const bool needs_depth = false;
    
    virtual ~PostProcessEffect() = default;

    void apply(Renderer & renderer);

    virtual void fragment(
        int u, int v, Vec3 & OUT,
        const std::vector< Color > & IMAGE,
        const std::vector< Color > & RAW_IMAGE,
        const std::vector< Color > & NORMAL,
        const std::vector< float > & DEPTH
    ) {
        std::cout << "YOLO" << std::endl;
    }

};


//more practical
#define FRAGMENT void fragment(int u, int v, Vec3 & OUT,const std::vector< Color > & IMAGE,const std::vector< Color > & RAW_IMAGE,const std::vector< Color > & NORMAL,const std::vector< float > & DEPTH)

namespace postprocess{

    namespace blur{

        class Cross_blur: public PostProcessEffect{
        public:
            int size;

            Cross_blur(int size): size(size) {}

            // needed 
            static PostProcessEffect* create(int size);
            FRAGMENT;
        };
    }
}
