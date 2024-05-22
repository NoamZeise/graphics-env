#ifndef GL_MODEL_RENDER_H
#define GL_MODEL_RENDER_H

#include <resource-loaders/texture_loader.h>
#include <resource-loaders/model_loader.h>

struct GPUModelGL;

class ModelLoaderGL : public InternalModelLoader {
public:
    ModelLoaderGL(Resource::Pool pool, BasePoolManager *pools);
    ~ModelLoaderGL() override;
    void loadGPU() override;
    void clearGPU() override;
    void DrawQuad(int count);
    void DrawModel(Resource::Model model,
		   uint32_t spriteColourShaderLoc,
		   uint32_t enableTexShaderLoc);
    void DrawModelInstanced(Resource::Model model, int count,
			    uint32_t spriteColourShaderLoc,
			    uint32_t enableTexShaderLoc);
    Resource::ModelAnimation getAnimation(Resource::Model model, std::string animation) override;
    Resource::ModelAnimation getAnimation(Resource::Model model, int index) override;

private:
    
    std::vector<GPUModelGL*> models;
    void draw(Resource::Model model, int count,
	      uint32_t colLoc, uint32_t enableTexLoc);
};

#endif
