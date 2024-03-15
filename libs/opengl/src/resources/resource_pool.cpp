#include "resource_pool.h"

#include <graphics/logger.h>

GLResourcePool::GLResourcePool(Resource::Pool pool, RenderConfig config, BasePoolManager* pools) {
    this->pool = pool;
    texLoader = new TextureLoaderGL(pool, config);
    modelLoader = new ModelLoaderGL(pool, pools);
    fontLoader = new InternalFontLoader(pool, texLoader);
}

GLResourcePool::~GLResourcePool() {
    delete texLoader;
    delete fontLoader;
    delete modelLoader;
}

void GLResourcePool::loadGpu() {
    LOG("loading to GPU");
    texLoader->loadGPU();
    fontLoader->loadGPU();
    modelLoader->loadGPU();
    LOG("finished loading to GPU");
    usingGPUResources = true;
}

void GLResourcePool::unloadStaged() {
    texLoader->clearStaged();
    fontLoader->clearStaged();
    modelLoader->clearStaged();
}

void GLResourcePool::unloadGPU() {
    texLoader->clearGPU();
    fontLoader->clearGPU();
    modelLoader->clearGPU();
    usingGPUResources = false;
}
