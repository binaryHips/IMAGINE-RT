# C++ CPU Ray-tracer

This is an educational project made for university. The objective was to create a simple ray-traced offline renderer in C++ ( + opengl for scene visualization)

As a bonus, I have added multi-threading, different render targets for normals, depth, etc., a post-process effect pipeline (with some filters like gaussian blur and a rough denoiser) aswell as a realtime preview mode.

As this is a 1-person project and was my learning project for C++, the codebase may not be awfully clean. Sowwy !

No generative AI was used in the making of this program.

## Features

### 3D Ray-traced rendering
Render of a dragon, 100k polys, 2 lights, 60 raysper pixel, 5 bounces per ray, 6 shadow rays per bounce
<img width="1594" height="896" alt="Render of a dragon, 100k polys, 2 lights, 60 raysper pixel, 5 bounces per ray, 6 shadow rays per bounce" src="https://github.com/user-attachments/assets/891eb2a2-0ae2-4c03-8f71-d1c61b5a1dba" />

A codebase was given with basic opengl code and structure for the non-raytraced preview. We had to create the ray-tracing system and every collision function from scratch, aswell as implement an acceleration structure for efficient triangulated meshes rendering.

Everything that comes next except the KD-Tree was not in the project's original goals, but were added by me as I felt like taking this project to the next level.

### Multi-threading
Reflexive bird, 5k polys, 1 lights, 30 raysper pixel, 5 bounces per ray, 6 shadow rays per bounce
<img width="1449" height="814" alt="Reflexive bird, 5k polys, 1 lights, 30 raysper pixel, 5 bounces per ray, 6 shadow rays per bounce" src="https://github.com/user-attachments/assets/e4b29f96-3bf9-43f5-89cb-443ee03f8594" />

The rendering takes (literally) full advantage of multi-core CPUs. Each render is broken down into small blocks of pixels, and each block is rendered by its own thread. The current setting creates rather small blocks, which saturate the CPU's scheduler, thus giving speed increases of more than the core count (while crashing everything else in the meantime... This can be removed by changing the block size).

### KD-Tree acceleration

In order to speed up complex, triangulated meshes rendering, An acceleration structure called a KD-Tree was implemented. This allows collision detection (= finding, if it exists, the nearest colliding triangle in the scene) to be done much faster, and most importantly, to increase only logarithmically when the tricount goes up (this is what keeps 100k triangles renders inside of the minute-long realm).

This implementation uses a scene-wide tree (as opposed to each triangle mesh having its own tree). This is faster, but it would not be ideal if we wanted to move meshes around or add/remove them at runtime, as the tree would need to be recomputed each update.

The following shows two screenshot of the realtime preview, same scene, first one using a naïve approach (testing each triangle intersection), second one using the acceleration structure.

naïve approach (testing each triangle intersection)            |  acceleration structure (KD-Tree)
:-------------------------:|:-------------------------:
 <img width="531" height="493" alt="image" src="https://github.com/user-attachments/assets/c95e98b9-c55b-430f-b70c-5ddbe13c9c52" /> |   <img width="531" height="493" alt="image" src="https://github.com/user-attachments/assets/a2412442-7b2a-45e2-a33c-5d0398c1cc60" />


### Post-processing pipeline

The program stores the rendered linear Z-Buffer, aswell as the screen-space normals. The programmer is able to use them inside of post-processing effects. Each renderer has a post-processing pipeline, which is an array of effects to be applied in order to the rendered image :
```cpp
realtimeRenderer = Renderer(
    360, 360, // resolution
    1 // sample count (rays per pixel)
);
realtimeRenderer.silent = true; // do not print progress while rendering

 // post processing // 
realtimeRenderer
  << postprocess::denoise::Similarity::create(1.0) // adds a base denoiser effect to the renderer
  << postprocess::color::Value::create(1.3) // adds a value offset effect to the renderer
  << postprocess::blur::Convolve::create(5, postprocess::kernel::GAUSSIAN_5_5) // adds a gaussian blur to the renderer;
  << postprocess::color::Vignette::create(0.0, 0.7) // adds a vignette to the edge of the final image
;
```

### Realtime preview

With all those optimizations and improvements, our CPU raytracer is now perfectly capable of low-resolution realtime rendering of simple scenes. (and near-realtime rendering of more complexe ones)

Video is sped up 1.5 times. FPS counter is accurate.

[![Watch the video](https://i.sstatic.net/Vp2cE.png)](https://github.com/user-attachments/assets/dfab4c2e-9bab-417f-a64b-a545e33d025c)



## How to use

First, clone and build using the makefile. You may get errors because of missing openGL dependencies. In this case, install what it asks for.

One you run the program, look at the console. an helper message shows the list of controls, and the scene statistics appear.

When you start a render, a progress update appears inside the console.

Enjoy !

