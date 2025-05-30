#include "model_loader.h"

#include "vertex_data.h"
#include <graphics/logger.h>

struct GPUMeshGL : public GPUMesh {
    GPUMeshGL(){}
    GPUMeshGL(MeshData* mesh) : GPUMesh(mesh) {}
    
    GLVertexData *vertexData;
    void draw(Resource::Model model, int instanceCount, BasePoolManager *pools,
	      int colLoc, int enableTexLoc);
};

struct GPUModelGL : public GPUModel {
    std::vector<GPUMeshGL> meshes;    
    GPUModelGL(ModelData *data);  
    ~GPUModelGL();
    void draw(Resource::Model model, int instanceCount, BasePoolManager *pools, int colLoc,
	      int enableTexLoc);
};

ModelLoaderGL::ModelLoaderGL(Resource::Pool pool, BasePoolManager *pools)
    : InternalModelLoader(pool, pools) {}

ModelLoaderGL::~ModelLoaderGL() {
    clearGPU();
}

void ModelLoaderGL::clearGPU() {
    for (GPUModelGL *model : models)
	delete model;
    models.clear();
}

void ModelLoaderGL::loadGPU() {
    clearGPU();
    loadQuad();
    models.resize(staged.size());
    for(int i = 0; i < staged.size(); i++)
	models[i] = new GPUModelGL(staged[i]);
    clearStaged();
}

  void ModelLoaderGL::DrawQuad(int count) {
      models[quad.ID]->meshes[0].vertexData->DrawInstanced(GL_TRIANGLES, count);
  }

void ModelLoaderGL::DrawModel(Resource::Model model, uint32_t spriteColourShaderLoc, uint32_t enableTexShaderLoc) {
    draw(model, 1, spriteColourShaderLoc, enableTexShaderLoc);
  }

void ModelLoaderGL::DrawModelInstanced(Resource::Model model,
				       int count, uint32_t spriteColourShaderLoc,
				       uint32_t enableTexShaderLoc) {
    draw(model, count, spriteColourShaderLoc, enableTexShaderLoc);
}

void ModelLoaderGL::draw(Resource::Model model, int count,
                         uint32_t colLoc, uint32_t enableTexLoc) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in draw with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return;
    }
    models[model.ID]->draw(model, count, pools, colLoc, enableTexLoc);

}

Resource::ModelAnimation ModelLoaderGL::getAnimation(Resource::Model model,
                                                     std::string animation) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in getAnimation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    return models[model.ID]->getAnimation(animation);
}

Resource::ModelAnimation ModelLoaderGL::getAnimation(Resource::Model model, int index) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in getAnimation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    return models[model.ID]->getAnimation(index);
}


/// --- Model ---

GPUModelGL::GPUModelGL(ModelData *data) : GPUModel(data) {
    meshes.resize(data->meshes.size());
    for (int i = 0; i < meshes.size(); i++) {
	meshes[i] = GPUMeshGL(data->meshes[i]);
	meshes[i].vertexData = new GLVertexData(
		data->format,
		data->meshes[i]->vertices,
		data->meshes[i]->vertexCount,
		data->meshes[i]->indices);
    }
}

GPUModelGL::~GPUModelGL() {
    for (auto &mesh : meshes)
	delete mesh.vertexData;
}

void GPUModelGL::draw(Resource::Model model, int instanceCount, BasePoolManager *pools,
		      int colLoc, int enableTexLoc) {
    for (auto& mesh: meshes)
	mesh.draw(model, instanceCount, pools, colLoc, enableTexLoc);
}


///  ---  Mesh  ---

void GPUMeshGL::draw(Resource::Model model, int instanceCount, BasePoolManager *pools,
		  int colLoc, int enableTexLoc) {
    glActiveTexture(GL_TEXTURE0);
    int texID = modelGetTexID(model, texture, pools);
    if(texID != -1) {		
	glUniform1i(enableTexLoc, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D, texID);
    } else {
	glUniform1i(enableTexLoc, GL_FALSE);
    }
    glUniform4fv(colLoc, 1,
		 model.colour.a == 0.0f ? &diffuseColour[0] :
		 &model.colour[0]);
    
    if(instanceCount > 1)
	vertexData->DrawInstanced(GL_TRIANGLES, instanceCount);
    else if(instanceCount == 1)
	vertexData->Draw(GL_TRIANGLES);
}
