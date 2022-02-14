#include <filament/Engine.h>
#include <filamentapp/FilamentApp.h>
#include <filament/Skybox.h>
#include <filament/Scene.h>
#include <iostream>
using namespace std;

using namespace filament;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    //Entity camera;
    Skybox* skybox;
    //Entity renderable;
};

int main()
{
    Config config;
    config.title = "yunzheng_demo";
    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        cout << "setup" << endl;
        //场景内设置一个天空盒的颜色，也就设置了背景色
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.5, 1.0}).build(*engine);
        scene->setSkybox(app.skybox);
    };
    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        cout << "cleanup" << endl;   
        engine->destroy(app.skybox); 
    };

    FilamentApp::get().run(config, setup, cleanup);
    return 0;
}