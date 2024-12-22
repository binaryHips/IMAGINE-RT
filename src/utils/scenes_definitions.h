#pragma once

#include "src/render/Scene.h"


static Scene sphere_and_plane() {

        Scene scene;
        //materials

        int glassSphereMat = scene.addMaterial(
            GlassMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.05
            )
        );

        int planeMat = scene.addMaterial(
            TexturedMaterial::create("img/uv.ppm")
        );

        
        {
            scene.spheres.resize( scene.spheres.size() + 1 );
            Sphere & s = scene.spheres[scene.spheres.size() - 1];
            s.m_center = Vec3(0. , 0. , 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material_id = glassSphereMat;
        }

        // added 

        {
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -1.));
            s.scale(Vec3(4., 4., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMat;
        }
        // added 

        {
            scene.lights.resize( scene.lights.size() + 1 );
            Light & light = scene.lights[scene.lights.size() - 1];
            light.pos = Vec3(0,3,0);
            light.radius = 1.0f;
            light.powerCorrection = 20.0f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
    return scene;
}

static Scene mesh() {

        Scene scene;

        //materials

        int mat = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.8,0.8,0.8 ), Vec3( 0.2,0.2,0.2 ), 1.0
            )
        );

        {
            scene.lights.resize( scene.lights.size() + 1 );
            Light & light = scene.lights[scene.lights.size() - 1];
            light.pos = Vec3(-3,3,3);
            light.radius = 2.5f;
            light.powerCorrection = 8.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        //added
        Mesh mesh;
        mesh.loadOFF("./models/suzanne.off");
        mesh.build_arrays();
        mesh.material_id = mat;
        scene.meshes.push_back(mesh); // copy but don't care

        return scene;
}

    Scene cornell_box(){
        
        Scene scene;

        //materials
        int glassMat = scene.addMaterial(
            GlassMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.1
            )
        );

        int mirrorMat = scene.addMaterial(
            MirrorMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,0.3,0.9 ), Vec3( 0.2,0.2,0.2 )
            )
        );

        int planeMatRed = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 0.2
            )
        );

        int planeMatWhite = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,1.0,1.0 ), Vec3( 0.2,0.2,0.2 ), 0.5
            )
        );

        int planeMatGreen = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,1.0,0.3 ), Vec3( 0.2,0.2,0.2 ), 0.2
            )
        );

        int planeMatPurple = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.8 ), Vec3( 0.2,0.2,0.2 ), 0.1
            )
        );



        {
            scene.lights.resize( scene.lights.size() + 1 );
            Light & light = scene.lights[scene.lights.size() - 1];
            light.pos = Vec3( 0.0, 1.0, 0.0 );
            light.radius = 0.5f;
            light.powerCorrection = 5.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        
        { //Back Wall
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWhite;
        }

        { //Left Wall

            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatRed;
        }

        { //Right Wall
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatGreen;
        }

        { //Floor
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWhite;
        }

        { //Ceiling
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatPurple;
        }
        
        { //Front Wall
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWhite;
        }

        //added
        Mesh mesh;
        mesh.loadOFF("./models/suzanne.off");
        mesh.build_arrays();
        mesh.material_id = mirrorMat;
        scene.meshes.push_back(mesh); // copy but don't care
        

        return scene;
}

    Scene cornell_box_textured(){
        
        Scene scene;

        //materials
        int glassMat = scene.addMaterial(
            GlassMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.1
            )
        );

        int simpleMat = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.4,0.7,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.1
            )
        );

        int mirrorMat = scene.addMaterial(
            MirrorMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,0.3,0.9 ), Vec3( 0.2,0.2,0.2 )
            )
        );

        int planeMatWalls = scene.addMaterial(
            TexturedMaterial::create(
                "img/textureHaven/pavement_06_2k/pavement_06_diff_2k.ppm"
            )
        );


        int planeMatGround = scene.addMaterial(
            TexturedMaterial::create(
                "img/textureHaven/wood_floor_deck_2k/wood_floor_deck_diff_2k.ppm"
            )
        );




        {
            scene.lights.resize( scene.lights.size() + 1 );
            Light & light = scene.lights[scene.lights.size() - 1];
            light.pos = Vec3( 0.0, 1.0, 0.0 );
            light.radius = 0.5f;
            light.powerCorrection = 3.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }
        
        { //Back Wall
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWalls;
        }

        { //Left Wall

            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWalls;
        }

        { //Right Wall
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWalls;
        }

        { //Floor
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatGround;
        }

        { //Ceiling
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatGround;
        }
        
        { //Front Wall
            scene.squares.resize( scene.squares.size() + 1 );
            Square & s = scene.squares[scene.squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.recomputeVectors();
            s.material_id = planeMatWalls;
        }

        //added
        Mesh mesh;
        mesh.loadOFF("./models/suzanne.off");
        mesh.translate(Vec3(0, 0, -1.4));
        mesh.build_arrays();
        mesh.material_id = simpleMat;
        scene.meshes.push_back(mesh); // copy but don't care
        

        return scene;
}

std::vector<Scene> getScenes(){
    return {
        sphere_and_plane(),
        mesh(),
        cornell_box(),
        cornell_box_textured()
    };
}