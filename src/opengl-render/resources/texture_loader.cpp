#include "texture_loader.h"

namespace Resource
{

	TextureLoader::LoadedTex::LoadedTex(std::string path)
	{
#ifndef NDEBUG
		std::cout << "loading texture: " << path << std::endl;
#endif
		ID = 0;
		width = 0;
		height = 0;
		int nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cerr << "failed to load texture at " << path << std::endl;
			return;
		}

		generateTexture(data, width, height, nrChannels);

		stbi_image_free(data);
	}

	TextureLoader::LoadedTex::LoadedTex(unsigned char* data, int width, int height, int nrChannels)
	{
		generateTexture(data, width, height, nrChannels);
	}

	void TextureLoader::LoadedTex::generateTexture(unsigned char* data, int width, int height, int nrChannels)
	{
		unsigned int format = GL_RGBA;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if(nrChannels == 4)
			format = GL_RGBA;
		else
		{
			std::cerr << "failed to load texture, unsupported num of channels!" << std::endl;
			return;
		}

		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if(settings::MIP_MAPPING)
			glGenerateMipmap(GL_TEXTURE_2D);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if(settings::PIXELATED)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	TextureLoader::LoadedTex::~LoadedTex()
	{
		glDeleteTextures(1, &ID);
	}

	void TextureLoader::LoadedTex::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, ID);
	}


	TextureLoader::TextureLoader() {}

	TextureLoader::~TextureLoader()
	{
		for(unsigned int i = 0; i < textures.size(); i++)
		{
			delete textures[i];
		}
	}

	Texture TextureLoader::LoadTexture(std::string path)
	{
		textures.push_back(new LoadedTex(path));
		return Texture(textures.size() - 1, glm::vec2(textures.back()->width, textures.back()->height), path);
	}

	Texture TextureLoader::LoadTexture(unsigned char* data, int width, int height, int nrChannels)
	{
		textures.push_back(new LoadedTex(data, width, height, nrChannels));
		return Texture(textures.size() - 1, glm::vec2(textures.back()->width, textures.back()->height), "FONT");
	}

	void TextureLoader::Bind(Texture tex)
	{
		if(tex.ID >= textures.size())
		{
			std::cerr << "texture ID out of range" << std::endl;
			return;
		}
		textures[tex.ID]->Bind();
	}

}
