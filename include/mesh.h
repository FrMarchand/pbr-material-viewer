#ifndef MESH_H
#define MESH_H

#include <glm\glm.hpp>
#include <vector>
#include "material.h"
#include "shader.h"


struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 TexCoords;
};

class Mesh
{
public:
	Mesh();
	virtual ~Mesh() = 0;
	virtual void draw(const Shader& shader) const;

private:
	unsigned int mVAO, mVBO, mEBO;
	int mIndexCount = 0;

protected:
	GLenum mPrimitive = GL_TRIANGLES;
	void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

#endif //MESH_H