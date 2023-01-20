#include "model_render.h"

#include <assimp/material.h>
#include <assimp/types.h>
#include <assimp/matrix4x4.h>
#include <iostream>

namespace Resource
{

    const auto IMPORT_PROPS =
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_LimitBoneWeights;


GLModelRender::GLModelRender()
{

}

GLModelRender::~GLModelRender()
{
	for(unsigned int i = 0; i < loadedModels.size(); i++)
	{
		delete loadedModels[i];
	}
}

Model GLModelRender::LoadModel(std::string path, GLTextureLoader* texLoader)
{
#ifndef NO_ASSIMP

#ifndef NDEBUG
	std::cout << "loading model: " << path << std::endl;
#endif

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, IMPORT_PROPS);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		throw std::runtime_error("failed to load model at \"" + path + "\" assimp error: " + importer.GetErrorString());

	loadedModels.push_back(new LoadedModel());
	LoadedModel* ldModel = loadedModels[loadedModels.size() - 1];
	ldModel->directory = path.substr(0, path.find_last_of('/'));

	processNode(ldModel, scene->mRootNode, scene, texLoader, aiMatrix4x4());

	return Model(loadedModels.size() - 1);
#else
	throw std::runtime_error("tried to load model but NO_ASSIMP is defined!");
#endif
}

  void GLModelRender::DrawModel(Model model, GLTextureLoader* texLoader, uint32_t spriteColourShaderLoc)
{
	if(model.ID >= loadedModels.size())
	{
		std::cerr << "model ID out of range" << std::endl;
		return;
	}

	for (auto& mesh: loadedModels[model.ID]->meshes)
	{
		glActiveTexture(GL_TEXTURE0);
		texLoader->Bind(mesh.texture);
        glUniform4fv(spriteColourShaderLoc, 1, &mesh.diffuseColour[0]);
        
		mesh.vertexData->Draw(GL_TRIANGLES);
	}
}

  void GLModelRender::DrawModelInstanced(Model model, GLTextureLoader* texLoader, int count, uint32_t spriteColourShaderLoc, uint32_t enableTexShaderLoc)
{
	if(model.ID >= loadedModels.size())
	{
		std::cerr << "model ID out of range" << std::endl;
		return;
	}

	for (auto& mesh: loadedModels[model.ID]->meshes)
	{
		glActiveTexture(GL_TEXTURE0);
		texLoader->Bind(mesh.texture);
        glUniform4fv(spriteColourShaderLoc, 1, &mesh.diffuseColour[0]);
        if(mesh.texture.ID == 0)
          glUniform1i(enableTexShaderLoc, GL_FALSE);
        else
          glUniform1i(enableTexShaderLoc, GL_TRUE);
		mesh.vertexData->DrawInstanced(GL_TRIANGLES, count);
	}
}

#ifndef NO_ASSIMP
void GLModelRender::processNode(LoadedModel* model, aiNode* node, const aiScene* scene, GLTextureLoader* texLoader, aiMatrix4x4 parentTransform)
{
	aiMatrix4x4 transform = parentTransform * node->mTransformation;
    
	for(size_t i = 0; i < node->mNumMeshes; i++)
	{
	    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		model->meshes.push_back(Mesh());
		processMesh(&model->meshes[model->meshes.size() - 1], mesh, scene, texLoader, transform);
	}
	for(size_t i = 0; i < node->mNumChildren; i++)
	{
		processNode(model, node->mChildren[i], scene, texLoader, transform);
	}
}
void GLModelRender::processMesh(Mesh* mesh, aiMesh* aimesh, const aiScene* scene, GLTextureLoader* texLoader, aiMatrix4x4 transform)
{
	loadMaterials(mesh, scene->mMaterials[aimesh->mMaterialIndex], texLoader);
    

	//vertcies
	std::vector<GLVertex3D> verticies;
	for(unsigned int i = 0; i < aimesh->mNumVertices;i++)
	{
		aiVector3D transformedVertex = transform * aimesh->mVertices[i];
		GLVertex3D vertex;
		vertex.position.x = transformedVertex.x;
		vertex.position.y = transformedVertex.y;
		vertex.position.z = transformedVertex.z;
		if(aimesh->HasNormals())
		{
			vertex.normal.x = aimesh->mNormals[i].x;
			vertex.normal.y = aimesh->mNormals[i].y;
			vertex.normal.z = aimesh->mNormals[i].z;
		}
		else
			vertex.normal = glm::vec3(0);
		if(aimesh->mTextureCoords[0])
		{
			vertex.texCoords.x = aimesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = aimesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texCoords = glm::vec2(0);

		verticies.push_back(vertex);
	}
	//indicies
	std::vector<unsigned int> indicies;
	for(unsigned int i = 0; i < aimesh->mNumFaces; i++)
	{
		aiFace face = aimesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indicies.push_back(face.mIndices[j]);
	}
	mesh->vertexData = new GLVertexData(verticies, indicies);
}
void GLModelRender::loadMaterials(Mesh* mesh, aiMaterial* material, GLTextureLoader* texLoader)
{
  aiColor3D diffuseColour;
  if(material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColour) != AI_SUCCESS)
  {
    std::cout << "Warning : failed to get model's diffuse colour\n";
  }
  else
  {
    mesh->diffuseColour = glm::vec4(diffuseColour.r, diffuseColour.g, diffuseColour.b, 1);
  }
  
	for(unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString aistring;
		material->GetTexture(aiTextureType_DIFFUSE, i, &aistring);
		std::string texLocation = aistring.C_Str();
		for(int i = 0; i < texLocation.size(); i++) {
		    if(texLocation[i] == '\\')
			texLocation[i] = '/';
		}
		texLocation = "textures/" + texLocation;

		bool skip = false;
		for(unsigned int j = 0; j < loadedTextures.size(); j++)
		{
			if(std::strcmp(loadedTextures[j].path.data(), texLocation.c_str()) == 0)
			{
				mesh->texture = loadedTextures[j];
				skip = true;
				break;
			}
		}
		if(!skip)
		{

			mesh->texture = texLoader->LoadTexture(texLocation);
#ifndef NDEBUG
			std::cout << "^ for model" << std::endl;
#endif
			//mesh->texture.type = TextureType::Diffuse; //attention
			loadedTextures.push_back(mesh->texture);
		}
	}
}
#endif

}
