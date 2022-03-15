#include <filament/Engine.h>
#include <filamentapp/FilamentApp.h>
#include <filament/Skybox.h>
#include <filament/Scene.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/StructureOfArrays.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Allocator.h>
#include <utils/bitset.h>
#include <cassert>

#include <iostream>
using namespace std;

using namespace filament;

using HeapAllocatorArena = utils::Arena<
        utils::HeapAllocator,
        utils::LockingPolicy::Mutex,
        utils::TrackingPolicy::DebugAndHighWatermark>;

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
    A(double n , const std::string& s): num(n), str(s)
    {
        cout << "A created" << endl;
    }
    ~A(){
        cout << "A destoryd" << endl;
    }
    double num;
    std::string str;
};

void testAllocator(){
    HeapAllocatorArena heapAllocator;
    double* d = static_cast<double*>(heapAllocator.alloc(sizeof(double)));
    *d = 1.0;
    heapAllocator.free(d, sizeof(*d));
    
    A* a = heapAllocator.make<A>(1.0f, "aaaa");
    heapAllocator.destroy(a);
}

void testBitset()
{
    utils::bitset8 s;
    assert(s.all() == false);
    assert(s.any() == false);
    assert(!s[0]);
    s.set(0);
    assert(s[0]);


    assert(s.all() == false);
    assert(s.any() == true);
    assert(s.count() == 1);
    s.set(7);
    s.set(6);
    s.set(5);
    s.set(4);
    s.set(3);
    s.set(2);
    s.set(1);
    assert(s.count() == 8);
    assert(s.all() == true);
    assert(s.any() == true);
    s.flip(0);
    assert(s.count() == 7);
    assert(s.all() == false);
    assert(s.any() == true);
}
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

#if 0
    testAllocator();
#endif

#if 0
    testBitset();
#endif
    struct Vertex {
        filament::math::float2 position;
        uint32_t color;
    };
    static const Vertex TRIANGLE_VERTICES[3] = {
        {{1, 0}, 0xffff0000u},
        {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
        {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
    };
    static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };
#if 0


    Engine* engine = Engine::create();

    //申请顶点缓冲
    VertexBuffer* vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
    //设置顶点缓冲
    vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));

    IndexBuffer* ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
    ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
    assert(sizeof(TRIANGLE_INDICES) == 6);
    engine->destroy(vb);
    Engine::destroy(&engine);
#endif

#if 1
    Config config;
    config.title = "yunzheng_demo";
    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        //view 是 主视图
        cout << "setup" << endl;
        //场景内设置一个天空盒的颜色，也就设置了背景色
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.5, 1.0}).build(*engine);
        scene->setSkybox(app.skybox);

        // 创建三角形的顶点缓冲区
        app.vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
        // 将数据拷贝到顶点缓冲区
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
        
        // 创建三角形的索引缓冲区
        app.ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        // 设置索引缓冲区的数据
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        
    };
    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        cout << "cleanup" << endl;   
        engine->destroy(app.skybox); 
    };

    FilamentApp::get().run(config, setup, cleanup);
#endif
    return 0;
}