#ifndef MESHPBR_H
#define MESHPBR_H

#include "mesh.h"
#include "material.h"

class MeshPBR : public Mesh
{
public:
	MeshPBR();
	virtual ~MeshPBR() = 0;
	void draw(const Shader& shader) const override;

	void setAlbedoMap(std::shared_ptr<Texture> albedoMap);
	void setNormalMap(std::shared_ptr<Texture> normalMap);
	void setMetallicMap(std::shared_ptr<Texture> metallicMap);
	void setRoughnessMap(std::shared_ptr<Texture> roughnessMap);
	void setAoMap(std::shared_ptr<Texture> aoMap);
	void setDisplacementMap(std::shared_ptr<Texture> displacementMap);
	void setTextureScale(float scaleX, float scaleY);

	void setBrdfLUT(std::shared_ptr<Texture> brdfLUT);
	void setIrradianceMap(std::shared_ptr<CubeMap> irradianceMap);
	void setPreFilterMap(std::shared_ptr<CubeMap> preFilterMap);

private:
	Material mMaterial;

	std::shared_ptr<Texture> mBrdfLUT;
	std::shared_ptr<CubeMap> mIrradianceMap;
	std::shared_ptr<CubeMap> mPreFilterMap;
};

#endif//MESHPBR_H