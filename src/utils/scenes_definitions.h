#pragma once

#include "src/render/Scene.h"


static Scene sphere_and_plane() {

    Scene scene;
    scene.name = "Setup test textures";
    //materials

    int glassMat = scene.addMaterial(
    GlassMaterial::create(
        Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.4
        )  
    );

    int sphereMat = scene.addMaterial(
        PhongMaterial::create(
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
        s.material_id = sphereMat;
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
        light.pos = Vec3(0,4,0);
        light.radius = 2.0f;
        light.powerCorrection = 10.0f;
        light.type = LightType_Spherical;
        light.material = Vec3(1,1,1);
        light.isInCamSpace = false;
    }
    scene.generateKdTree();
    return scene;
}

static Scene mesh() {

        Scene scene;
        scene.name = "Suzanne (sans kdtree)";
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

static Scene mesh_kd1() {

        Scene scene;
        scene.name = "Suzanne (avec kdtree)";
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

        scene.generateKdTree();

        return scene;
}

static Scene mesh_with_kdTree() {

        Scene scene;
        scene.name = "Dagon!";

        //materials

        int mat = scene.addMaterial(
            PhongMaterial::create(
                Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.4,0.55 ), Vec3( 0.2,0.2,0.2 ), 5.0
            )
        );

        int planeMat = scene.addMaterial(
            TexturedMaterial::create("img/textureHaven/pavement_06_2k/pavement_06_diff_2k.ppm")
        );

        {
            scene.lights.resize( scene.lights.size() + 1 );
            Light & light = scene.lights[scene.lights.size() - 1];
            light.pos = Vec3(-3,3,-2);
            light.radius = 2.5f;
            light.powerCorrection = 8.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        {
            scene.lights.resize( scene.lights.size() + 1 );
            Light & light = scene.lights[scene.lights.size() - 1];
            light.pos = Vec3(3,3,3);
            light.radius = 3.5f;
            light.powerCorrection = 9.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1,1,1);
            light.isInCamSpace = false;
        }

        {
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -1.1));
        s.scale(Vec3(4., 4., 1.));
        s.rotate_x(-90);
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = planeMat;
        }

        //added
        Mesh mesh;
        mesh.loadOFF("./models/xyzrgb_dragon_100k.off");
        mesh.scale(Vec3(0.03));
        mesh.build_arrays();
        mesh.material_id = mat;
        scene.meshes.push_back(mesh); // copy but don't care



        scene.generateKdTree();
        return scene;
}

static Scene cornell_box(){
        
    Scene scene;
    scene.name = "Cornell";
    //materials
    int glassMat = scene.addMaterial(
        GlassMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.4
        )
    );

    int mirrorMat = scene.addMaterial(
        MirrorMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,0.3,0.9 ), Vec3( 0.2,0.2,0.2 )
        )
    );

    int planeMatRed = scene.addMaterial(
        PhongMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.3,0.3 ), Vec3( 0.2,0.2,0.2 ), 2.0
        )
    );

    int planeMatWhite = scene.addMaterial(
        PhongMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,1.0,1.0 ), Vec3( 0.2,0.2,0.2 ), 3.0
        )
    );

    int planeMatGreen = scene.addMaterial(
        PhongMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,1.0,0.3 ), Vec3( 0.2,0.2,0.2 ), 1.0
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

    { //GLASS Sphere
        scene.spheres.resize( scene.spheres.size() + 1 );
        Sphere & s = scene.spheres[scene.spheres.size() - 1];
        s.m_center = Vec3(1.0, -1.25, 0.5);
        s.m_radius = 0.75f;
        s.build_arrays();
        s.material_id = glassMat;
    }

    { //MIRRORED Sphere
        scene.spheres.resize( scene.spheres.size() + 1 );
        Sphere & s = scene.spheres[scene.spheres.size() - 1];
        s.m_center = Vec3(-1.0, -1.25, -0.5);
        s.m_radius = 0.75f;
        s.build_arrays();
        s.material_id = mirrorMat;
    }


    return scene;
}

Scene cornell_box_textured(){
    
    Scene scene;
    scene.name = "Cornell + textures";
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
        light.pos = Vec3( 0.0, 0.5, 1.0 );
        light.radius = 1.5f;
        light.powerCorrection = 6.5f;
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
    };

    //added
    
    Mesh mesh;
    mesh.loadOFF("./models/flamant/flamant.off");
    mesh.rotate_y(-90);
    mesh.scale(Vec3(0.004));
    mesh.translate(Vec3(0, -2, 2.5));
    mesh.build_arrays();
    mesh.material_id = mirrorMat;
    scene.meshes.push_back(mesh); // copy but don't care
    
    
    scene.generateKdTree();
    return scene;
}

Scene flamant(){
    
    Scene scene;
    scene.name = "flamant";
    //materials

    int flamantMat = scene.addMaterial(
        FlamantMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 1.0,0.0,0.0 ), Vec3( 0.2,0.2,0.2 ), 1.1
        )
    );



    int bgMat = scene.addMaterial( // https://opengameart.org/content/seamless-space-backgrounds
        TexturedMaterial::create(
            "models/flamant/Blue-Nebula-1-1024x1024.ppm", true
        )
    );

    int groundMat = scene.addMaterial(
        PhongMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 0.3,0.1,0.1 ), Vec3( 0.8, 0.5, 0.3 ), 1.0
        )
    );
    int pillarsMat = scene.addMaterial(
        PhongMaterial::create(
            Vec3( 0.0,0.0,0.0 ), Vec3( 0.1,0.1,0.1 ), Vec3( 1.0, 0.1, 0.1 ), 8.0
        )
    );

    float p = 60;

    { //bg
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -p));
        s.scale(Vec3(p , p , 1.));
        s.rotate_y(90);
        s.translate(Vec3(0., p/2.0, 0.0));
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = bgMat;
    }
    { //bg
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -p));
        s.scale(Vec3(p , p , 1.));
        s.rotate_y(0);
        s.translate(Vec3(0., p/2.0, 0.0));
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = bgMat;
    }
    { //bg
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -p));
        s.scale(Vec3(p , p , 1.));
        s.rotate_y(180);
        s.translate(Vec3(0., p/2.0, 0.0));
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = bgMat;
    }
    { //bg
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -p));
        s.scale(Vec3(p , p , 1.));
        s.rotate_y(-90);
        s.translate(Vec3(0., p/2.0, 0.0));
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = bgMat;
    }
    { //bg
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -p));
        s.scale(Vec3(p , p , 1.));
        s.rotate_x(-90);
        s.translate(Vec3(0., p/2.0, 0.0));
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = bgMat;
    }
    { //bg
        scene.squares.resize( scene.squares.size() + 1 );
        Square & s = scene.squares[scene.squares.size() - 1];
        s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
        s.translate(Vec3(0., 0., -p));
        s.scale(Vec3(p , p , 1.));
        s.rotate_x(90);
        s.build_arrays();
        s.recomputeVectors();
        s.material_id = bgMat;
    }

    //added
    
    Mesh mesh;
    mesh.loadOFF("./models/flamant/flamant.off");
    mesh.scale(Vec3(0.01));
    mesh.build_arrays();
    mesh.material_id = flamantMat;
    scene.meshes.push_back(mesh);

    mesh = Mesh();
    mesh.loadOFF("./models/flamant/ground.off");
    mesh.scale(Vec3(0.01));
    mesh.build_arrays();
    mesh.material_id = groundMat;
    scene.meshes.push_back(mesh);

    mesh = Mesh();
    mesh.loadOFF("./models/flamant/pillars.off");
    mesh.scale(Vec3(0.01));
    mesh.build_arrays();
    mesh.material_id = pillarsMat;
    scene.meshes.push_back(mesh);
    
    {
        scene.lights.resize( scene.lights.size() + 1 );
        Light & light = scene.lights[scene.lights.size() - 1];
        light.pos = Vec3( 0.0, 4.0, 0.0 );
        light.radius = 1.0f;
        light.powerCorrection = 15.f;
        light.type = LightType_Spherical;
        light.material = Vec3(1,0.2,0.3);
        light.isInCamSpace = false;
    }
    {
        scene.lights.resize( scene.lights.size() + 1 );
        Light & light = scene.lights[scene.lights.size() - 1];
        light.pos = Vec3( -8.0, 8.0, 0 );
        light.radius = 1.1f;
        light.powerCorrection = 80.f;
        light.type = LightType_Spherical;
        light.material = Vec3(1,0.2,0.3);
        light.isInCamSpace = false;
    }

    
    scene.generateKdTree();
    return scene;
}

std::vector<Scene> getScenes(){

    /* DOESNT WORK parce que KDTree utilise des std::unique_ptr qui peuvent pas être copiés par l'initialize list dans le vector.
    return {
        sphere_and_plane(),
        mesh(),
        cornell_box(),
        cornell_box_textured()
    };
    */

     std::vector<Scene> res;
    res.push_back(sphere_and_plane());
    res.push_back(mesh());
    res.push_back(mesh_kd1());
    res.push_back(mesh_with_kdTree());
    res.push_back(cornell_box());
    res.push_back(cornell_box_textured());
    res.push_back(flamant());
    return res;
}