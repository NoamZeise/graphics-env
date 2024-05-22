#ifndef RENDER_API_MODEL_LOADER_H
#define RENDER_API_MODEL_LOADER_H

#include "../resources.h"
#include "../model/animation.h"
#include "../pipeline.h"
#include "../default_vertex_types.h"


class ModelLoader {
    
    const std::string DEFAULT_TEXTURE_PATH = "textures/";
    
 public:

    /// ----- Load Models from ModelInfo::Model -----
    
    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modelType,
			 ModelInfo::Model  model,
			 std::string       textureFolder,
			 std::vector<Resource::ModelAnimation>* pAnimations);

    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modeltype,
			 ModelInfo::Model  model) {
	return load(modeltype, model, DEFAULT_TEXTURE_PATH, nullptr);
    }
 
    
    Resource::Model load(ModelInfo::Model &modelData) {
	return load(vertex::v3D, modelData);
    }

    /// -----  Load Models from file -----

    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modeltype,
			 std::string       path,
			 std::string       textureFolder,
			 std::vector<Resource::ModelAnimation>* pAnimations) {
	ModelInfo::Model model = loadModelData(path);
	return load(modeltype, model, textureFolder, pAnimations);
    }
    
    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modeltype,
			 std::string       path,
			 std::string       textureFolder) {
	return load(modeltype, path, textureFolder, nullptr);
    }

    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modeltype,
			 std::string       path,
			 std::vector<Resource::ModelAnimation>* pAnimations) {
	return load(modeltype, path, DEFAULT_TEXTURE_PATH, pAnimations);
    }   
    
    template <typename T_Vert>
    Resource::Model load(ModelType<T_Vert> modeltype,
			 std::string       path) {
	return load(modeltype, path, DEFAULT_TEXTURE_PATH, nullptr);
    }

    Resource::Model load(std::string path) {
	return load(vertex::v3D, path);
    }
    
    /// Load Model from file into a ModelInfo::Model variable.
    /// Can then be inspected and modified before being loaded to GPU
    virtual ModelInfo::Model loadModelData(std::string path) = 0;

    
    /// ----- Animation Info -----
    
    virtual Resource::ModelAnimation getAnimation(Resource::Model model, std::string animation) = 0;


    virtual Resource::ModelAnimation getAnimation(Resource::Model model, int index) = 0;

protected:
    
    virtual Resource::Model loadData(PipelineInput format,
				     ModelInfo::Model &model,
				     std::vector<void*> &meshVertData,
				     std::string textureFolder,
				     std::vector<Resource::ModelAnimation> *pAnimations) = 0;
};



template <typename T_Vert>
Resource::Model ModelLoader::load(ModelType<T_Vert> modelType,
				  ModelInfo::Model model,
				  std::string textureFolder,
				  std::vector<Resource::ModelAnimation>* pAnimations) {

    std::vector<void*> meshVertData(model.meshes.size());
    
    for(int i = 0; i < model.meshes.size(); i++) {

	ModelInfo::Mesh* m = &model.meshes[i];
	T_Vert* vertices = (T_Vert*)
	    std::malloc(modelType.input.size * m->verticies.size());
	
	for(int j = 0; j < m->verticies.size(); j++)
	    vertices[j] = modelType.vertexLoader(m->verticies[j], m->bindTransform);

	meshVertData[i] = vertices;
    }
    
    return loadData(modelType.input, model, meshVertData, textureFolder, pAnimations);
}

#endif /* RENDER_API_MODEL_LOADER_H */
