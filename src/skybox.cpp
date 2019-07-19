#include "skybox.h"
#include "shader.h"
#include <glad\glad.h>

namespace {
	Vertex make_vertex(float px, float py, float pz, float tx, float ty, float tz, float nx, float ny, float nz, float u, float v) {
		return{ glm::vec3(px, py, pz), glm::vec3(nx, ny, nz), glm::vec3(tx, ty, tz), glm::vec2(u, v) };
	}
}

Skybox::Skybox()
{
	std::vector<Vertex> vertices;
	vertices.push_back(make_vertex(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));    // top left (front) -- back
	vertices.push_back(make_vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));   // bottom left (front)
	vertices.push_back(make_vertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));   // bottom right (front)
	vertices.push_back(make_vertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));   // top right (front)

	vertices.push_back(make_vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));    // top left (right) --left
	vertices.push_back(make_vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));   // bottom left (right)
	vertices.push_back(make_vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));   // bottom right (right)
	vertices.push_back(make_vertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));   // top right (right)

	vertices.push_back(make_vertex(1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));    // top left (back) -- front
	vertices.push_back(make_vertex(1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));   // bottom left (back)
	vertices.push_back(make_vertex(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));   // bottom right (back)
	vertices.push_back(make_vertex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));   // top right (back)

	vertices.push_back(make_vertex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));    // top left (left) --right
	vertices.push_back(make_vertex(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));   // bottom left (left)
	vertices.push_back(make_vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));   // bottom right (left)
	vertices.push_back(make_vertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));   // top right (left)

	vertices.push_back(make_vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f));    // top left (top)
	vertices.push_back(make_vertex(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f));   // bottom left (top)
	vertices.push_back(make_vertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f));   // bottom right (top)
	vertices.push_back(make_vertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f));   // top right (top)

	vertices.push_back(make_vertex(1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f));    // top left (bottom)
	vertices.push_back(make_vertex(1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f));   // bottom left (bottom)
	vertices.push_back(make_vertex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f));   // bottom right (bottom)
	vertices.push_back(make_vertex(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f));   // top right (bottom)

	std::vector<unsigned int> indices{
		0, 1, 2,    // triangle 1 (front)
		0, 2, 3,    // triangle 2 (front)
		4, 5, 6,    // triangle 3 (right)
		4, 6, 7,    // triangle 4 (right)
		8, 9, 10,   // triangle 5 (back)
		8, 10, 11,  // triangle 6 (back)
		12, 13, 14, // triangle 7 (left)
		12, 14, 15, // triangle 8 (left)
		16, 17, 18, // triangle 9 (top)
		16, 18, 19, // triangle 10 (top)
		20, 21, 22, // triangle 11 (bottom)
		20, 22, 23  // triangle 12 (bottom)
	};

	setupMesh(vertices, indices);
}

void Skybox::setEnvironmentMap(std::shared_ptr<CubeMap> environmentMap)
{
	mEnvironmentMap = environmentMap;
}

void Skybox::draw(const Shader& shader) const
{
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are lower or equal to depth buffer's content

	if (mEnvironmentMap != nullptr) {
		mEnvironmentMap->bind(GL_TEXTURE0);
	}

	Mesh::draw(shader);
	glDepthFunc(GL_LESS); // set depth function back to default
}