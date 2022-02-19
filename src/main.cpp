#include <filament/Engine.h>
#include <filamentapp/FilamentApp.h>
#include <filament/Skybox.h>
#include <filament/Scene.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
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
using namespace utils;

class MyListener: public EntityManager::Listener 
{
public:
    void onEntitiesDestroyed(size_t n, Entity const* entities) noexcept override{
        for(size_t i = 0; i < n; i++){
            cout << "entity " <<  entities[i].getId() << " is distoryed" << endl;
            assert(!EntityManager::get().isAlive(entities[i]));
        }
    }

};


int main()
{
    MyListener l;
    auto& m = EntityManager::get();
    m.registerListener(&l);
    auto e = m.create();
    cout << "create entity:" << e.getId() << endl;
    auto e2 = e;
    assert(e2 == e);
    cout << "e2:" << e2.getId() << endl;
    std::size_t e_hash = std::hash<Entity>{}(e);
    std::cout << "hash(" << e.getId() << ") = " << e_hash << '\n';

    assert(m.isAlive(e));
    cout << "begin destory entity:" << e.getId() << endl;
    m.destroy(e);
    cout << "end destory entity:" << e.getId() << endl;
    assert(!m.isAlive(e));
    

    
#if 0
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
#endif
    return 0;
}