#include <filament/Engine.h>
#include <filamentapp/FilamentApp.h>
#include <filament/Skybox.h>
#include <filament/Scene.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/Renderer.h>
#include <filament/RenderableManager.h>
#include "resources/filamentapp.h"
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/StructureOfArrays.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Allocator.h>
#include <utils/bitset.h>
#include <utils/Path.h>
#include <viewer/SimpleViewer.h>
#include <image/LinearImage.h>
#include <imageio/ImageDecoder.h>
#include <stb_image.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <imgui.h>
using namespace std;

using namespace filament;
using utils::Entity;
using namespace filament::viewer;

using HeapAllocatorArena = utils::Arena<
        utils::HeapAllocator,
        utils::LockingPolicy::Mutex,
        utils::TrackingPolicy::DebugAndHighWatermark>;


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

    //????????????????????????????????????
    auto name = soa.push_back(1, 1.1f, std::string("wyy")).back<2>();
    cout << "push_back:" << name << endl;
    soa.push_back(2, 2.2f, std::string("wdd"));
    cout << "size:" << soa.size() << endl;
    cout << "cap :" << soa.capacity() << endl;

    //??????????????????????????????
    double* nums = soa.data<1>();
    cout << "nums[0]:" << nums[0] << endl;
    cout << "nums[1]:" << nums[1] << endl;
    int64_t* Nums = soa.data<0>();
    cout << "Nums[0]:" << Nums[0] << endl;
    cout << "Nums[1]:" << Nums[1] << endl;

    //?????????????????????????????????
    for(auto it = soa.begin<1>(); it != soa.end<1>(); ++it){
        cout << *it << endl;
    }

    //??????????????????????????????????????????
    cout << "elementAt<1>(1):" << soa.elementAt<1>(1) << endl;
    cout << "elementAt<0>(0):" << soa.elementAt<0>(0) << endl;

    //???????????????????????????????????????????????? ?????????????????????
    for(auto it = soa.begin(); it != soa.end(); ++it){
        cout << "wyy test :" <<  it.get<0>() << " ," << it.get<1>() << " :" << it.get<2>()<< endl;
    }

    //?????????????????????????????????????????????
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

void test_triangle() {
    struct App {
        VertexBuffer* vb;
        IndexBuffer* ib;
        Material* mat;
        Camera* cam;
        Entity camera;
        Skybox* skybox;
        Entity renderable;
    };
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

    Config config;
    config.backend = filament::Engine::Backend::OPENGL;
    config.splitView = true;
    config.title = "yunzheng_demo";
    
    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        //view ??? ?????????
        cout << "setup" << endl;
        //??????????????????????????????????????????????????????????????????
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.5, 1.0}).build(*engine);
        
        scene->setSkybox(app.skybox);

        // ?????????????????????????????????
        app.vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
        // ?????????????????????????????????
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
        
        // ?????????????????????????????????
        app.ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        // ??????????????????????????????
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app.mat = Material::Builder()
                .package(FILAMENTAPP_BAKEDCOLOR_DATA, FILAMENTAPP_BAKEDCOLOR_SIZE)
                .build(*engine);
        
        // ??????????????????????????????????????????????????????????????????
        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.renderable);

        // ???????????????????????????????????????
        scene->addEntity(app.renderable);
    };
    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        cout << "cleanup" << endl;   
        
        engine->destroy(app.renderable);
        engine->destroy(app.mat);
        engine->destroy(app.ib);
        engine->destroy(app.vb);
        engine->destroy(app.skybox); 
    };

    FilamentApp::get().run(config, setup, cleanup);
}
namespace testImage{
struct App {
    Engine* engine;
    Config config;
    Camera* mainCamera;

    struct Scene{
        Entity imageEntity;
        VertexBuffer* imageVertexBuffer = nullptr;
        IndexBuffer* imageIndexBuffer = nullptr;
        Material* imageMaterial = nullptr;
        Texture* imageTexture = nullptr;
        Texture* defaultTexture = nullptr;
        TextureSampler sampler;
    } scene;
    bool showImage = false;
    filament::math::float3 backgroundColor = filament::math::float3(0.0f);
};
}
static void createImageRenderable(Engine* engine, Scene* scene, testImage::App& app) {
    using filament::math::float4;
    // ? ???????????????????????????????????????????????????
    // https://stackoverflow.com/questions/2588875/whats-the-best-way-to-draw-a-fullscreen-quad-in-opengl-3-2/51625078
    // https://wallisc.github.io/rendering/2021/04/18/Fullscreen-Pass.html

    static constexpr float4 sFullScreenTriangleVertices[3] = {
            { -1.0f, -1.0f, 1.0f, 1.0f },
            {  3.0f, -1.0f, 1.0f, 1.0f },
            { -1.0f,  3.0f, 1.0f, 1.0f }
    };
    static const uint16_t sFullScreenTriangleIndices[3] = { 0, 1, 2 };
    auto& em = EntityManager::get();
    Material* material = Material::Builder()
        .package(FILAMENTAPP_IMAGE_DATA, FILAMENTAPP_IMAGE_SIZE)
        .build(*engine);
    VertexBuffer * vertexBuffer = VertexBuffer::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT4, 0)
        .build(*engine);
    vertexBuffer->setBufferAt(*engine, 0, {sFullScreenTriangleVertices, sizeof(sFullScreenTriangleVertices)});

    IndexBuffer* indexBuffer = IndexBuffer::Builder()
        .indexCount(3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*engine);
    indexBuffer->setBuffer(*engine,
        {sFullScreenTriangleIndices, sizeof(sFullScreenTriangleIndices)}
    );
    Entity imageEntity = em.create();
    RenderableManager::Builder(1)
        .boundingBox({{}, {1.0f, 1.0f, 1.0f}}) // ?
        .material(0, material->getDefaultInstance())
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 3)
        .culling(false)
        .build(*engine, imageEntity);
    
    scene->addEntity(imageEntity);
    app.scene.imageEntity = imageEntity;
    app.scene.imageVertexBuffer = vertexBuffer;
    app.scene.imageIndexBuffer = indexBuffer;
    app.scene.imageMaterial = material;

    // ?????????????????????????????????????????????, Material ??? image???????????????????????????????????????????????????????????????????????? ??????
    // ???????????????????????????????????????????????????
    Texture * texture = Texture::Builder()
        .width(1)
        .height(1)
        .levels(1)
        .format(Texture::InternalFormat::RGBA8)
        .sampler(Texture::Sampler::SAMPLER_2D)
        .build(*engine);
    static uint32_t pixel = 0;
    Texture::PixelBufferDescriptor buffer(&pixel, 4, Texture::Format::RGBA, Texture::Type::UBYTE);
    texture->setImage(*engine, 0, std::move(buffer));
    app.scene.defaultTexture = texture;
}    
static void loadImage(testImage::App& app, Engine* engine, Path filename) {
    if(app.scene.imageTexture){
        engine->destroy(app.scene.imageTexture);
        app.scene.imageTexture = nullptr;
    }
    if(!filename.exists()){
        std::cerr << "The input image does not exist: " << filename << std::endl;
        return;
    }
    std::ifstream inputStream(filename, std::ios::binary);
    using image::LinearImage;
    using image::ImageDecoder;
    LinearImage * image = new LinearImage(
        ImageDecoder::decode(
            inputStream, filename, ImageDecoder::ColorSpace::SRGB
        )
    );
    if(!image->isValid()){
        std::cerr << "The input image is invalid: " << filename << std::endl;
        app.showImage = false;
        return;
    }
    inputStream.close();

    uint32_t channels = image->getChannels();
    uint32_t w = image->getWidth();
    uint32_t h = image->getHeight();
    Texture* texture = Texture::Builder()
        .width(w)
        .height(h)
        .levels(1)// ???
        .format(channels == 3? Texture::InternalFormat::RGB16F : Texture::InternalFormat::RGBA16F)
        .sampler(Texture::Sampler::SAMPLER_2D)
        .build(*engine);
    Texture::PixelBufferDescriptor::Callback freeCallback = [](void* buf, size_t, void* data) {
        delete reinterpret_cast<LinearImage*>(data);
    };
    Texture::PixelBufferDescriptor buffer(
        image->getPixelRef(),
        size_t(w*h*channels*sizeof(float)),
        channels == 3 ? Texture::Format::RGB : Texture::Format::RGBA,
        Texture::Type::FLOAT,
        freeCallback
    );
    texture->setImage(*engine, 0, std::move(buffer));
    texture->generateMipmaps(*engine); // ??

    app.scene.sampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
    app.scene.sampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
    app.scene.sampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
    app.scene.sampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);
    app.scene.imageTexture = texture;
    app.showImage = true;
}
void test_image(int argc, char** argv) {
    using namespace testImage;

    App app;
    app.config.backend = filament::Engine::Backend::OPENGL;
    //Path filename = argv[1];
    app.config.title = "Filament Image Viewer";
    //app.config.splitView = true;

    auto setup = [&](Engine*engine, View* view, Scene* scene){
        app.engine = engine;
        createImageRenderable(engine, scene, app);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.scene.imageEntity);
        engine->destroy(app.scene.imageVertexBuffer);
        engine->destroy(app.scene.imageIndexBuffer);
        engine->destroy(app.scene.imageMaterial);
        engine->destroy(app.scene.imageTexture);
        engine->destroy(app.scene.defaultTexture);
    };

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        if (app.showImage) {
            Texture *texture = app.scene.imageTexture;
            float srcWidth = texture->getWidth();
            float srcHeight = texture->getHeight();
            float dstWidth = view->getViewport().width;
            float dstHeight = view->getViewport().height;

            float srcRatio = srcWidth / srcHeight;
            float dstRatio = dstWidth / dstHeight;

            bool xMajor = dstWidth / srcWidth > dstHeight / srcHeight;

            float sx = 1.0f;
            float sy = dstRatio / srcRatio;

            float tx = 0.0f;
            float ty = ((1.0f - sy) * 0.5f) / sy;

            if (xMajor) {
                sx = srcRatio / dstRatio;
                sy = 1.0;
                tx = ((1.0f - sx) * 0.5f) / sx;
                ty = 0.0f;
            }

            //?????????????????????
            filament::math::mat3f transform(
                    1.0f / sx,  0.0f,       0.0f,
                    0.0f,       1.0f / sy,  0.0f,
                    -tx,        -ty,         1.0f
            );

            app.scene.imageMaterial->setDefaultParameter("transform", transform);
            app.scene.imageMaterial->setDefaultParameter(
                    "yunzhengInput", app.scene.imageTexture, app.scene.sampler);
        } else {
            app.scene.imageMaterial->setDefaultParameter(
                    "yunzhengInput", app.scene.defaultTexture, app.scene.sampler);
        }

        app.scene.imageMaterial->setDefaultParameter("showImage", app.showImage ? 1 : 0);
        app.scene.imageMaterial->setDefaultParameter(
                "backgroundColor", RgbType::sRGB, app.backgroundColor);
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.setDropHandler([&](std::string path) {
        loadImage(app, app.engine, Path(path));
    });

    filamentApp.run(app.config, setup, cleanup, nullptr, preRender);
    return;
}

Texture* loadTexture(const std::string& path, Engine* engine) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if(!data){
        return nullptr;
    }
    if(!(nrChannels == 3 || nrChannels == 4)){
        return nullptr;
    }
    Texture* texture = Texture::Builder()
        .width(width)
        .height(height)
        .sampler(Texture::Sampler::SAMPLER_2D)
        .levels(1)
        .format(nrChannels == 4 ? Texture::InternalFormat::RGBA8 : Texture::InternalFormat::RGB8)
        .build(*engine);
    Texture::PixelBufferDescriptor buffer(data, width*height*nrChannels*sizeof(unsigned char),
        nrChannels == 3 ? Texture::Format::RGB : Texture::Format::RGBA,
        Texture::Type::UBYTE,
        [](void* buf, size_t, void* data){
            stbi_image_free(reinterpret_cast<unsigned char *>(buf));
        });
    
    texture->setImage(*engine, 0, std::move(buffer));
    return texture;
}

void learnOpenGL() {
    Config config;
    struct App{
        VertexBuffer* vb;
        IndexBuffer* ib;
        Texture* texture;
        Texture* texture2;
        Material* material;
        Entity rectEntity;
        TextureSampler sampler;
    }app;
    float s = 1.0f;
    static float vertices[] = {
        // --??????               ------??????------           --????????????
        s,      s,      0.0f,  1.0f, 0.0f, 0.0f,         1.0f, 1.0f, // ?????????
        s,      -1*s,   0.0f,  0.0f, 1.0f, 0.0f,         1.0f, 0.0f,// ?????????
        -1*s,   -1*s,   0.0f,  0.0f, 0.0f, 1.0f,         0.0f, 0.0f,// ?????????
        -1*s,   s,      0.0f,  0.0f, 0.0f, 1.0f,         0.0f, 1.0f// ?????????
    };

    static unsigned int indices[] = { // ???????????????0??????! 
        0, 1, 3, // ??????????????????
        1, 2, 3  // ??????????????????
    };
    auto setup = [&](Engine*engine, View* view, Scene* scene){

        // ?????????????????????????????????
        app.vb = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, sizeof(float)*8)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::FLOAT3, sizeof(float)*3, sizeof(float)*8)
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, sizeof(float)*6, sizeof(float)*8)
                .build(*engine);
        // ?????????????????????????????????
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(vertices, sizeof(vertices), nullptr));
        
        // ?????????????????????????????????
        app.ib = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::UINT)
                .build(*engine);
        // ??????????????????????????????
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));

        app.material = Material::Builder()
            .package(FILAMENTAPP_HELLO_DATA, FILAMENTAPP_HELLO_SIZE)
            .build(*engine);

        filament::math::float3 color(0.0f,  0.8f,       0.0f);

        app.material->setDefaultParameter("backgroundColor", color);

        app.texture = loadTexture("/Users/yunzheng/Desktop/code/filament-yunzheng/resource/container.jpeg", engine);
        app.texture2 = loadTexture("/Users/yunzheng/Desktop/code/filament-yunzheng/resource/awesomeface.png", engine);
        assert(app.texture != nullptr);
        assert(app.texture2 != nullptr);
        app.sampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
        app.sampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
        app.sampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
        app.sampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);

        app.material->setDefaultParameter("input0", app.texture, app.sampler);
        app.material->setDefaultParameter("input1", app.texture2, app.sampler);

        app.rectEntity = EntityManager::get().create();
        RenderableManager::Builder(1)
            .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
            .material(0, app.material->getDefaultInstance())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 6)
            .culling(false)
            .build(*engine, app.rectEntity);
        
        scene->addEntity(app.rectEntity);

    };
    auto cleanup = [&app](Engine* engine, View*, Scene*){
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.material);
        engine->destroy(app.rectEntity);
        engine->destroy(app.texture);
    };
    FilamentApp::get().run(config, setup, cleanup);
}
int main(int argc, char** argv)
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

#if 0
    test_triangle();
#endif
#if 0
    test_image(argc, argv);
#endif
#if 1
    learnOpenGL();
#endif
    return 0;
}