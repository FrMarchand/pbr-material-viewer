#include "cubemapgenerator.h"
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmrc\cmrc.hpp>
CMRC_DECLARE(resources);

#include "cubemap.h"
#include "shader.h"

namespace {
	const int envRes = 1024;
	const int irrRes = 64;
	const int prefilterRes = 1024;
	const int brdfLUTRes = 512;

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
}

CubeMapGenerator::CubeMapGenerator()
{
	mSkybox = std::make_unique<Skybox>();
	mQuad = std::make_unique<Quad>();
}

CubeMapGenerator::~CubeMapGenerator()
{
}

std::shared_ptr<CubeMap> CubeMapGenerator::generateEnvironmentMap(const GLchar* imagePath)
{
	std::shared_ptr<CubeMap> environmentMap = std::make_shared<CubeMap>();

	// Load equirectangular hdr image
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float *data = stbi_loadf(imagePath, &width, &height, &nrComponents, 3);

	if (!data) {
		// Fall back to embedded resources
		auto fs = cmrc::resources::get_filesystem();
		if (fs.exists(imagePath)) {
			auto envRes = fs.open(imagePath);
			std::string str(envRes.begin(), envRes.size());
			data = stbi_loadf_from_memory((const unsigned char*)str.c_str(), str.size(), &width, &height, &nrComponents, 3);
		}
	}

	if (data)
	{
		// Framebuffer setup
		unsigned int captureFBO, captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, envRes, envRes);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		unsigned int hdrTextureID;
		glGenTextures(1, &hdrTextureID);
		glBindTexture(GL_TEXTURE_2D, hdrTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Generate environment cubemap
		environmentMap->bind(GL_TEXTURE0);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, envRes, envRes, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		mSkybox->setEnvironmentMap(environmentMap);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// convert HDR equirectangular environment map to cubemap equivalent
		Shader equirectangularToCubemapShader("shaders/shadercubemap.vs", "shaders/shaderequirectangular.fs");
		equirectangularToCubemapShader.use();
		equirectangularToCubemapShader.setInt("equirectangularMap", 0);
		equirectangularToCubemapShader.setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTextureID);

		glViewport(0, 0, envRes, envRes);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);

		for (unsigned int i = 0; i < 6; ++i)
		{
			equirectangularToCubemapShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentMap->getId(), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			mSkybox->draw(equirectangularToCubemapShader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		environmentMap->bind(GL_TEXTURE0);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// Clean up
		glDeleteTextures(1, &hdrTextureID);
		glDeleteRenderbuffers(1, &captureRBO);
		glDeleteFramebuffers(1, &captureFBO);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}

	stbi_image_free(data);

	return environmentMap;
}

std::shared_ptr<CubeMap> CubeMapGenerator::generateIrradianceMap(const std::shared_ptr<CubeMap> environmentMap)
{
	std::shared_ptr<CubeMap> irradianceMap = std::make_shared<CubeMap>();

	irradianceMap->bind(GL_TEXTURE0);

	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irrRes, irrRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Framebuffer setup
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irrRes, irrRes);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// convert HDR equirectangular environment map to cubemap equivalent
	Shader irradianceShader("shaders/shadercubemap.vs", "shaders/shaderirradiance.fs");
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);
	irradianceShader.setMat4("projection", captureProjection);

	mSkybox->setEnvironmentMap(environmentMap);

	glViewport(0, 0, irrRes, irrRes); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	for (unsigned int i = 0; i < 6; ++i) {
		irradianceShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap->getId(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mSkybox->draw(irradianceShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clean up
	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return irradianceMap;
}

std::shared_ptr<CubeMap> CubeMapGenerator::generatePreFilterMap(const std::shared_ptr<CubeMap> environmentMap)
{
	std::shared_ptr<CubeMap> preFilterMap = std::make_shared<CubeMap>();

	preFilterMap->bind(GL_TEXTURE0);

	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilterRes, prefilterRes, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Framebuffer setup
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, prefilterRes, prefilterRes);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// convert HDR equirectangular environment map to cubemap equivalent
	Shader prefilterShader("shaders/shadercubemap.vs", "shaders/shaderprefilter.fs");
	prefilterShader.use();
	prefilterShader.setInt("environmentMap", 0);
	prefilterShader.setMat4("projection", captureProjection);
	prefilterShader.setInt("envRes", envRes);

	mSkybox->setEnvironmentMap(environmentMap);

	glViewport(0, 0, prefilterRes, prefilterRes); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(prefilterRes * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(prefilterRes * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.setFloat("roughness", roughness);

		for (unsigned int i = 0; i < 6; ++i) {
			prefilterShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preFilterMap->getId(), mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			mSkybox->draw(prefilterShader);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clean up
	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return preFilterMap;
}

std::shared_ptr<Texture> CubeMapGenerator::generateBrdfLUT()
{
	std::shared_ptr<Texture> brdfMap = std::make_shared<Texture>();
	brdfMap->bind(GL_TEXTURE0);

	// pre-allocate enough memory for the LUT texture.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfLUTRes, brdfLUTRes, 0, GL_RG, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Framebuffer setup
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdfLUTRes, brdfLUTRes);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfMap->getId(), 0);

	glViewport(0, 0, brdfLUTRes, brdfLUTRes);

	// Render quad
	Shader brdfShader("shaders/shaderbrdf.vs", "shaders/shaderbrdf.fs");
	brdfShader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mQuad->draw(brdfShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clean up
	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);

	return brdfMap;
}