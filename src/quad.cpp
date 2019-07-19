#include "quad.h"

namespace {
	Vertex make_vertex(float px, float py, float pz, float tx, float ty, float tz, float nx, float ny, float nz, float u, float v) {
		return{ glm::vec3(px, py, pz), glm::vec3(nx, ny, nz), glm::vec3(tx, ty, tz), glm::vec2(u, v) };
	}
}

Quad::Quad()
{
	std::vector<Vertex> vertices;
	float tanX = 1.0f;
	float tanY = 0.0f;
	float tanZ = 0.0f;
	vertices.push_back(make_vertex(-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
	vertices.push_back(make_vertex(-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertices.push_back(make_vertex(1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
	vertices.push_back(make_vertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

	std::vector<unsigned int> indices{
		0, 1, 2,
		0, 2, 3
	};

	setupMesh(vertices, indices);
}

Quad::~Quad()
{

}