#ifndef GL_MODEL_RENDER_H
#define GL_MODEL_RENDER_H

#include "vertex_data.h"
#include <resource_loader/texture_loader.h>
#include <resource_loader/model_loader.h>
#include <resource_loader/vertex_model.h>

struct GPUModelGL;

class ModelLoaderGL : public InternalModelLoader {
public:
    ModelLoaderGL(Resource::Pool pool, BasePoolManager *pools);
    ~ModelLoaderGL();
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
