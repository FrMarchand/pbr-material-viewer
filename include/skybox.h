#ifndef SKYBOX_H
#define SKYBOX_H

#include <memory>
#include "cubemap.h"
#include "shader.h"
#include "mesh.h"

class Skybox : public Mesh
{
public:
	Skybox();
	void draw(const Shader& shader) const override;
	void setEnvironmentMap(std::shared_ptr<CubeMap> environmentMap);

private:
	std::shared_ptr<CubeMap> mEnvironmentMap = nullptr;
};

#endif //SKYBOX_H