#include <iostream>
#include "Application.h"
#include "Renderer.h"
#include "Shader.h"
#define app Application::GetInstance()
int main()
{
    app->init("OpenSoftRenderer");//初始化Application

    app->addObjToWorld(std::string("african_head.obj"));
    app->addObjToWorld(std::string("floor.obj"));
    app->addObjToWorld(std::string("tri0.obj"));
    app->loadDiffTexture(std::string("checker.png"));
    app->loadSpecTexture(std::string("african_head_spec.tga"));
    app->loadNormalTexture(std::string("african_head_nm.tga"));

    app->run();
    
    return 0;
}