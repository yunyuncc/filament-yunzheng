#include <filament/Engine.h>
#include <filamentapp/FilamentApp.h>
#include <filament/Skybox.h>
#include <filament/Scene.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/StructureOfArrays.h>
#include <utils/SingleInstanceComponentManager.h>
#include <cassert>

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

void testEntityManager(){
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
}

void testStructureOfArrays(){

    StructureOfArrays<int64_t, double, std::string> soa(5);

    cout << "ArrayCount:" << soa.getArrayCount() << endl;
    cout << "0 NeededSize:" << soa.getNeededSize(0) << endl;
    cout << "1 NeededSize:" << soa.getNeededSize(1) << endl;
    cout << "2 NeededSize:" << soa.getNeededSize(2) << endl;
    cout << "3 NeededSize:" << soa.getNeededSize(3) << endl;
    cout << "4 NeededSize:" << soa.getNeededSize(4) << endl;
    soa.setCapacity(10);
    cout << "size:" << soa.size() << endl;
    cout << "cap :" << soa.capacity() << endl;

    //往每个数组里面塞两个元素
    auto name = soa.push_back(1, 1.1f, std::string("wyy")).back<2>();
    cout << "push_back:" << name << endl;
    soa.push_back(2, 2.2f, std::string("wdd"));
    cout << "size:" << soa.size() << endl;
    cout << "cap :" << soa.capacity() << endl;

    //取得某个数组的首地址
    double* nums = soa.data<1>();
    cout << "nums[0]:" << nums[0] << endl;
    cout << "nums[1]:" << nums[1] << endl;
    int64_t* Nums = soa.data<0>();
    cout << "Nums[0]:" << Nums[0] << endl;
    cout << "Nums[1]:" << Nums[1] << endl;

    //遍历某个数组的所有元素
    for(auto it = soa.begin<1>(); it != soa.end<1>(); ++it){
        cout << *it << endl;
    }

    //直接取第几个数组的第几个元素
    cout << "elementAt<1>(1):" << soa.elementAt<1>(1) << endl;
    cout << "elementAt<0>(0):" << soa.elementAt<0>(0) << endl;

    //按索引遍历，可以得到所有数组内的 某个索引下的值
    for(auto it = soa.begin(); it != soa.end(); ++it){
        cout << "wyy test :" <<  it.get<0>() << " ," << it.get<1>() << " :" << it.get<2>()<< endl;
    }

    //按类型遍历，遍历每个类型的数组
    size_t s = soa.size();
    soa.forEach([s](auto*p){
        for(size_t i = 0; i < s; i++){
            cout << "for each" << p[i] <<  endl;
        }
        
    });
}

void testSingleInstanceComponentManager(){
    //name, width, height
    SingleInstanceComponentManager<std::string, uint32_t, uint32_t> rectManager;
    
    auto e = EntityManager::get().create();
    auto e2 = EntityManager::get().create();
    auto instance = rectManager.addComponent(e);
    cout << "componentCount:" << rectManager.getComponentCount() << endl;

    rectManager.elementAt<0>(instance) = "wyy";
    rectManager.elementAt<1>(instance) = 1920;
    rectManager.elementAt<2>(instance) = 1080;

    cout << rectManager.elementAt<0>(instance) << "," << rectManager.elementAt<1>(instance) << "," << rectManager.elementAt<2>(instance) << endl;
    assert(rectManager.hasComponent(e));
    assert(!rectManager.hasComponent(e2));
}

void testNameComponentManager()
{
    auto names = NameComponentManager(EntityManager::get());
    auto e = EntityManager::get().create();
    names.addComponent(e);
    auto i = names.getInstance(e);
    names.setName(i, "wyy");
    cout << names.getName(i) << endl;
}

class A{
public:
    // prevent heap allocation
    static void *operator new     (size_t) = delete;
    static void *operator new[]   (size_t) = delete;
    static void  operator delete  (void*)  = delete;
    static void  operator delete[](void*)  = delete;
    // disallow copy and assignment
    //A(A const&) = delete;
    //A(A&&) = delete;
    //A& operator=(A const&) = delete;
    //A& operator=(A&&) = delete;
protected:
    ~A() = default;
    A() = default;
};

class B:public A{

};

int main()
{
#if 0
    testEntityManager();
    std::shared_ptr<NameComponentManager> names = std::make_shared<NameComponentManager>(EntityManager::get());
#endif

#if 0
    testStructureOfArrays();
#endif
#if 0
    testSingleInstanceComponentManager();
#endif

#if 0
    testNameComponentManager();
#endif

    //FilamentAPI api;
    
    B b;
    std::vector<B> vb;
    vb.push_back(b);

    size_t alignment = alignof(std::max_align_t);
    cout << "max alignment:" << alignment << endl;
    cout << "min sizeof(void*):" << sizeof(void*) << endl;

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