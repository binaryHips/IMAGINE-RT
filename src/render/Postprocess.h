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
    inline float sampleBuffer(const std::vector< float > & buffer, int x, int y) const;

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
        std::cout << "FRAGMENT FUNCTION NOT IMPLEMENTED IN POSTPROCESS" << std::endl;
    }

};

#define FRAGMENT fragment(int u, int v, Vec3 & OUT,const std::vector< Color > & IMAGE,const std::vector< Color > & RAW_IMAGE,const std::vector< Color > & NORMAL,const std::vector< float > & DEPTH)
// effects

namespace postprocess::kernel{
    const std::vector<float> GAUSSIAN_5_5(
        {0.0030,    0.0133,    0.0219,    0.0133,    0.0030,
        0.0133,    0.0596,    0.0983,    0.0596,    0.0133,
        0.0219 ,   0.0983,    0.1621,    0.0983,    0.0219,
        0.0133,    0.0596,    0.0983,    0.0596,    0.0133,
        0.0030,    0.0133,    0.0219,    0.0133 ,   0.0030}
    );

    const std::vector<float> GAUSSIAN_3_3(
        {1/16.0f, 1/8.0f, 1/16.0f, 
        1/8.0f, 1/4.0f, 1/8.0f, 
        1/16.0f, 1/8.0f, 1/16.0f}
    );
}



namespace postprocess::blur{

        class Cross_blur: public PostProcessEffect{
        public:
            int size;

            Cross_blur(int size): size(size) {}

            // needed 
            static PostProcessEffect* create(int size) {
                return new Cross_blur(size);
            }

            void FRAGMENT;
        };


        class Convolve: public PostProcessEffect{
        public:
            int size_x;
            const std::vector<float> kernel;
            Convolve(int size, const std::vector<float> & kernel): size_x(size), kernel(kernel) {}

            // needed 
            static PostProcessEffect* create(int size_x, std::vector<float> kernel) {
                return new Convolve(size_x, kernel);
            }

            void FRAGMENT;
        };

}


namespace postprocess::color{

        class Contrast: public PostProcessEffect{
        public:
            float val;

            Contrast(float v): val(v) {}

            // needed 
            static PostProcessEffect* create(float v) {
                return new Contrast(v);
            }
            
            void FRAGMENT;
        };

        class Value: public PostProcessEffect{
        public:
            float val;

            Value(float v): val(v) {}

            // needed 
            static PostProcessEffect* create(float v) {
                return new Value(v);
            }
            
            void FRAGMENT;
        };

}

namespace postprocess::utils{

    class Depth: public PostProcessEffect{

        float maxDepth = -1;

        public:
            Depth() = default;


            static PostProcessEffect* create() {
                return new Depth();
            }
            
            void FRAGMENT;
        };

    
    class Normals: public PostProcessEffect{
        public:
            Normals() = default;


            static PostProcessEffect* create() {
                return new Normals();
            }
            
            void FRAGMENT;
        };

}


namespace postprocess::denoise{

    class Similarity: public PostProcessEffect{

        float fac;
        public:
            Similarity() = default;
            explicit Similarity(float fac): fac(fac) {}

            static PostProcessEffect* create(float fac) {
                return new Similarity(fac);
            }
            
            void FRAGMENT;
        };

}