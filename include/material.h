#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include "texture.h"
#include "cubemap.h"
#include "shader.h"

class Material
{
public:
	Material();
	void use(const Shader& shader, unsigned int &textureUnit) const;
	void setAlbedoMap(std::shared_ptr<Texture> albedoMap);
	void setNormalMap(std::shared_ptr<Texture> normalMap);
	void setMetallicMap(std::shared_ptr<Texture> metallicMap);
	void setRoughnessMap(std::shared_ptr<Texture> roughnessMap);
	void setAoMap(std::shared_ptr<Texture> aoMap);
	void setDisplacementMap(std::shared_ptr<Texture> displacementMap);
	void setTextureScale(float scaleX, float scaleY);

private:
	std::shared_ptr<Texture> mAlbedoMap = nullptr;
	std::shared_ptr<Texture> mNormalMap = nullptr;
	std::shared_ptr<Texture> mMetallicMap = nullptr;
	std::shared_ptr<Texture> mRoughnessMap = nullptr;
	std::shared_ptr<Texture> mAoMap = nullptr;
	std::shared_ptr<Texture> mDisplacementMap = nullptr;
	glm::vec2 mTextureScale = glm::vec2(1.0f, 1.0f);
};

#endif//MATERIAL_H