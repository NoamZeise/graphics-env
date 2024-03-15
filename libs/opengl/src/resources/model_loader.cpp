#include "model_loader.h"
#include <graphics/logger.h>

struct GLMesh : public GPUMesh {
    GLVertexData *vertexData;
    void draw(Resource::Model model, int instanceCount, BasePoolManager *pools,
	      int colLoc, int enableTexLoc);
};

struct GPUModelGL : public GPUModel {
    std::vector<GLMesh> meshes;    
    template <typename T_Vert>
    GPUModelGL(LoadedModel<T_Vert> &data);    
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
    models.resize(currentIndex);
    for(auto &model: stage2D.models)
	models[model.ID] = new GPUModelGL(model);
    for(auto &model: stage3D.models)
	models[model.ID] = new GPUModelGL(model);
    for(auto &model: stageAnim3D.models)
	models[model.ID] = new GPUModelGL(model);
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

template<typename T_Vert>
GPUModelGL::GPUModelGL(LoadedModel<T_Vert> &data) : GPUModel(data) {
    meshes.resize(data.meshes.size());
    for (int i = 0; i < meshes.size(); i++) {
	Mesh<T_Vert> *mesh = data.meshes[i];
	meshes[i].vertexData = new GLVertexData(mesh->verticies, mesh->indices);
	meshes[i].load(mesh);
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


///  ---  MESH  ---

void GLMesh::draw(Resource::Model model, int instanceCount, BasePoolManager *pools,
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
