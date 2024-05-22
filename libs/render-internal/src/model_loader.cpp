#include <resource-loaders/model_loader.h>

#include <graphics/logger.h>
#include "assimp_loader.h"
#include <cstdlib>

int modelGetTexID(Resource::Model model, Resource::Texture texture, BasePoolManager* pools) {
    Resource::Texture meshTex = model.overrideTexture.ID == Resource::NULL_ID ?
	texture : model.overrideTexture;    
    return meshTex.ID != Resource::NULL_ID ?
	pools->tex(meshTex)->getViewIndex(meshTex) : -1;
}

/// ------- Internal Model Loader --------

InternalModelLoader::InternalModelLoader(Resource::Pool pool, BasePoolManager* pools) {
    this->pool = pool;
    this->pools = pools;
    loader = new AssimpLoader();
}

InternalModelLoader::~InternalModelLoader() {
    clearStaged();
    delete loader;
}

void InternalModelLoader::clearStaged() {
    for(auto &s: staged)
	delete s;
    staged.clear();
}

ModelInfo::Model InternalModelLoader::loadModelData(std::string path) {
    return loader->LoadModel(path);
}

ModelData::ModelData(ModelInfo::Model &model,
		     PipelineInput format,
		     std::vector<void*> meshVertData,
		     std::string texturePath,
		     TextureLoader* tex) {
    this->format = format;

    if(meshVertData.size() != model.meshes.size())
	throw std::runtime_error(
		"Internal Model Loader: "
		"Mesh Vertex data array was not the same size as mesh array!");
    meshes.resize(model.meshes.size());
    for(int i = 0; i < model.meshes.size(); i++)
	meshes[i] = new MeshData(model.meshes[i], meshVertData[i], texturePath, tex);
    
    for(ModelInfo::Animation &anim: model.animations) {
	if(model.bones.size() >= Resource::MAX_BONES)
	    LOG_CERR("Model had more bones than MAX_BONES, "
		     "consider upping the shader max bones. "
		     "Ignoring excess bones", "Warning: ");
	animations.push_back(Resource::ModelAnimation(model.bones, anim));
    }
}

Resource::Model InternalModelLoader::loadData(
	PipelineInput format,
	ModelInfo::Model &model,
	std::vector<void*> &meshVertData,
	std::string textureFolder,
	std::vector<Resource::ModelAnimation> *pAnimations) {
    
    if(textureFolder.size() > 0 && textureFolder[textureFolder.size() - 1] != '/')
	textureFolder.push_back('/');
	
    Resource::Model usermodel(staged.size(), format, pool);
    staged.push_back(new ModelData(model, format, meshVertData,
				   //temp
				   textureFolder,
				   pools->tex(pool)));
    if(pAnimations != nullptr)
	*pAnimations = staged.back()->animations;       
    LOG("Model Loaded " <<
	" - pool: " << pool.ID <<
	" - id: " << usermodel.ID);
    return usermodel;
}

void InternalModelLoader::loadQuad() {
    ModelInfo::Mesh mesh;
    mesh.verticies.resize(4);
    mesh.verticies[0].Position = {0.0f, 0.0f, 0.0f};
    mesh.verticies[0].TexCoord = {0.0f, 0.0f};
    mesh.verticies[1].Position = {1.0f, 0.0f, 0.0f};
    mesh.verticies[1].TexCoord = {1.0f, 0.0f};
    mesh.verticies[2].Position = {1.0f, 1.0f, 0.0f};
    mesh.verticies[2].TexCoord = {1.0f, 1.0f};
    mesh.verticies[3].Position = {0.0f, 1.0f, 0.0f};
    mesh.verticies[3].TexCoord = {0.0f, 1.0f};
    mesh.indices = { 0, 3, 2, 2, 1, 0};
    ModelInfo::Model quad;
    
    quad.meshes.push_back(mesh);
    
    this->quad = ModelLoader::load(vertex::v2D, quad, "", nullptr);
}


/// ------- Model and Mesh Staging Data -------


ModelData::~ModelData() {
    for(auto& m: meshes)
	delete m;
}

MeshData::MeshData(ModelInfo::Mesh &mesh,
		   void* vertexData,
		   std::string texturePath,
		   TextureLoader* tex) {
    this->vertices = vertexData;
    this->vertexCount = mesh.verticies.size();
    this->indices = mesh.indices;

    //TODO: remove after pipeline update
    this->diffuseColour = mesh.diffuseColour;
    if(mesh.diffuseTextures.size() > 0)
	this->texture = tex->load(texturePath + mesh.diffuseTextures[0]);
}

MeshData::~MeshData() {
    std::free(vertices);
}


/// ------ Model and Mesh GPU Data -------


GPUMesh::GPUMesh(MeshData* mesh) {
    diffuseColour = mesh->diffuseColour;
    texture = mesh->texture;
}

GPUModel::GPUModel(ModelData* model) {
    vertType = model->format;
    animations.resize(model->animations.size());
    for (int i = 0; i < model->animations.size(); i++) {
	animations[i] = model->animations[i];
	animationMap[model->animations[i].getName()] = i;
    }
}

Resource::ModelAnimation GPUModel::getAnimation(int index) {
    if (index >= animations.size()) {
	LOG_ERROR("Model animation index was out of range. "
		  "animation index: " << index
		  << " - size: " << animations.size());
	return Resource::ModelAnimation();
    }
    return animations[index];
}

Resource::ModelAnimation GPUModel::getAnimation(std::string animation) {
    if (animationMap.find(animation) == animationMap.end()) {
	LOG_ERROR("No animation called " << animation << " could be found in the"
		  " animation map for model");
	return Resource::ModelAnimation();
    }        
    return getAnimation(animationMap[animation]);  
}    
