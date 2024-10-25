// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------


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

using namespace std;

#include "src/imageLoader.h"

#include "src/Material.h"


// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 480;
static unsigned int SCREENHEIGHT = 480;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static unsigned int FPS = 0;
static bool fullScreen = false;

std::vector<Scene> scenes;
unsigned int selected_scene;

std::vector< std::pair< Vec3 , Vec3 > > rays;

void printUsage () {
    cerr << endl
         << "gMini: a minimal OpenGL/GLUT application" << endl
         << "for 3D graphics." << endl
         << "Author : Tamy Boubekeur (http://www.labri.fr/~boubek)" << endl << endl
         << "Usage : ./gmini [<file.off>]" << endl
         << "Keyboard commands" << endl
         << "------------------" << endl
         << " ?: Print help" << endl
         << " w: Toggle Wireframe Mode" << endl
         << " g: Toggle Gouraud Shading Mode" << endl
         << " f: Toggle full screen mode" << endl
         << " <drag>+<left button>: rotate model" << endl
         << " <drag>+<right button>: move model" << endl
         << " <drag>+<middle button>: zoom" << endl
         << " q, <esc>: Quit" << endl << endl;
}

void usage () {
    printUsage ();
    exit (EXIT_FAILURE);
}


// ------------------------------------
void initLight () {
    GLfloat light_position[4] = {0.0, 1.5, 0.0, 1.0};
    GLfloat color[4] = { 1.0, 1.0, 1.0, 1.0};
    GLfloat ambient[4] = { 1.0, 1.0, 1.0, 1.0};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    //glCullFace (GL_BACK);
    glDisable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
}


// ------------------------------------
// Replace the code of this 
// functions for cleaning memory, 
// closing sockets, etc.
// ------------------------------------

void clear () {

}

// ------------------------------------
// Replace the code of this 
// functions for alternative rendering.
// ------------------------------------


void draw () {
    glEnable(GL_LIGHTING);
    scenes[selected_scene].draw();

    // draw rays : (for debug)
    //  std::cout << rays.size() << std::endl;
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(6);
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    for( unsigned int r = 0 ; r < rays.size() ; ++r ) {
        glVertex3f( rays[r].first[0],rays[r].first[1],rays[r].first[2] );
        glVertex3f( rays[r].second[0], rays[r].second[1], rays[r].second[2] );
    }
    glEnd();
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    static float lastTime = glutGet ((GLenum)GLUT_ELAPSED_TIME);
    static unsigned int counter = 0;
    counter++;
    float currentTime = glutGet ((GLenum)GLUT_ELAPSED_TIME);
    if (currentTime - lastTime >= 1000.0f) {
        FPS = counter;
        counter = 0;
        static char winTitle [64];
        sprintf (winTitle, "Raytracer - FPS: %d", FPS);
        glutSetWindowTitle (winTitle);
        lastTime = currentTime;
    }
    glutPostRedisplay ();
}

std::array< GLdouble, 16 > inv_model_view, inv_proj;
std::array< GLdouble, 2 >  near_far_planes;

inline int idx_from_coord(int x, int y, int w){
    return (x + y * w);
}


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

void ray_trace_from_camera() {
    int w = glutGet(GLUT_WINDOW_WIDTH)  ,   h = glutGet(GLUT_WINDOW_HEIGHT);
    //w =64; h = 64;
    std::cout << "Ray tracing a " << w << " x " << h << " image" << std::endl;
    camera.apply();
    Vec3 pos , dir;
    //    unsigned int nsamples = 100;
    unsigned int nsamples = 300;
    std::vector< Vec3 > image( w*h , Vec3(0,0,0) );

    auto start = std::chrono::system_clock::now();

    //ray_trace_from_camera_single_threaded(w, h, nsamples, image);
    ray_trace_from_camera_multithreaded(w, h, nsamples, image);

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::clog <<  std::flush <<"\tDone in " << elapsed_seconds.count() << "s" <<std::endl;

    std::string filename = "./rendu.ppm";
    ofstream f(filename.c_str(), ios::binary);
    if (f.fail()) {
        cout << "Could not open file: " << filename << endl;
        return;
    }
    f << "P3" << std::endl << w << " " << h << std::endl << 255 << std::endl;
    for (int i=0; i<w*h; i++)
        f << (int)(255.f*std::min<float>(1.f,image[i][0])) << " " << (int)(255.f*std::min<float>(1.f,image[i][1])) << " " << (int)(255.f*std::min<float>(1.f,image[i][2])) << " ";
    f << std::endl;
    f.close();
}


void key (unsigned char keyPressed, int x, int y) {
    Vec3 pos , dir;
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;
    case 'q':
    case 27:
        clear ();
        exit (0);
        break;
    case 'w':
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if(polygonMode[0] != GL_FILL)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        break;

    case 'r':
        camera.apply();
        rays.clear();
        ray_trace_from_camera();
        break;
    case '+':
        selected_scene++;
        if( selected_scene >= scenes.size() ) selected_scene = 0;
        break;

    case '8':
        camera.move(0.0, 0.0, 0.1); break;

    case '5':
        camera.move(0.0, 0.0, -0.1); break;
    default:
        printUsage ();
        break;
    }
    idle ();
}

void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle ();
}

void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}





int main (int argc, char ** argv) {
    if (argc > 2) {
        printUsage ();
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("gMini");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); // for showing big points
    glEnable(GL_CULL_FACE);

    camera.move(0., 0., -3.1);
    selected_scene=0;
    scenes.resize(3);
    scenes[0].setup_single_sphere();
    scenes[1].setup_single_square();
    scenes[2].setup_cornell_box();

    glutMainLoop ();
    return EXIT_SUCCESS;
}

