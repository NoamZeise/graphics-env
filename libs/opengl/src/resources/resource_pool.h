#ifndef OGL_ENV_RESOURCE_POOL_H
#define OGL_ENV_RESOURCE_POOL_H

#include "texture_loader.h"
#include "model_loader.h"
#include <resource_loader/font_loader.h>
#include <resource_loader/pool_manager.h>
#include <graphics/resource_pool.h>

class GLResourcePool : public ResourcePool {
public:
    GLResourcePool(Resource::Pool pool, RenderConfig config, BasePoolManager* pools);
    ~GLResourcePool();

    void loadGpu();
    void unloadStaged();
    void unloadGPU();

    ModelLoader* model() override { return modelLoader; }
    TextureLoader* tex() override { return texLoader; }
    FontLoader* font()   override { return fontLoader; }

    TextureLoaderGL* texLoader;
    InternalFontLoader* fontLoader;
    ModelLoaderGL* modelLoader;
    bool usingGPUResources = false;
};

MAKE_POOL_MANAGER(GLPoolManager, GLResourcePool)

#endif
