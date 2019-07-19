#ifndef CUBEMAPGENERATOR_H
#define CUBEMAPGENERATOR_H

#include <memory>
#include <glad\glad.h>
#include "cubemap.h"
#include "texture.h"
#include "quad.h"
#include "skybox.h"

class CubeMapGenerator
{
public:
	CubeMapGenerator();
	~CubeMapGenerator();
	std::shared_ptr<CubeMap> generateEnvironmentMap(const GLchar* imagePath);
	std::shared_ptr<CubeMap> generateIrradianceMap(const std::shared_ptr<CubeMap> environmentMap);
	std::shared_ptr<CubeMap> generatePreFilterMap(const std::shared_ptr<CubeMap> environmentMap);
	std::shared_ptr<Texture> generateBrdfLUT();

private:
	std::unique_ptr<Quad> mQuad = nullptr;
	std::unique_ptr<Skybox> mSkybox = nullptr;
};

#endif//CUBEMAPGENERATOR_H