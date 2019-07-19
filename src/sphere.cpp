#include "sphere.h"

namespace {
	const unsigned int SECTOR_COUNT = 256;
	const unsigned int STACK_COUNT = 256;
	const float RADIUS = 1.0f;
	const double PI = 3.1415926535897932384626433832795;

	Vertex make_vertex(float px, float py, float pz, float tx, float ty, float tz, float nx, float ny, float nz, float u, float v) {
		return{ glm::vec3(px, py, pz), glm::vec3(nx, ny, nz), glm::vec3(tx, ty, tz), glm::vec2(u, v) };
	}
}

Sphere::Sphere()
{
	std::vector<Vertex> vertices;

	float x, y, z, xz;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / RADIUS;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = static_cast<float>(2 * PI / SECTOR_COUNT);
	float stackStep = static_cast<float>(PI / STACK_COUNT);
	float sectorAngle, stackAngle;

	for (int i = 0; i <= STACK_COUNT; ++i)
	{
		stackAngle = static_cast<float>(PI / 2 - i * stackStep);  // starting from pi/2 to -pi/2
		xz = RADIUS * cosf(stackAngle);                           // r * cos(u)
		y = RADIUS * sinf(stackAngle);                            // r * sin(u)

													// add (sectorCount+1) vertices per stack
													// the first and last vertices have same position and normal, but different tex coords
		float midpoint = RADIUS * cosf(0.0f);
		for (int j = 0; j <= SECTOR_COUNT; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

													// vertex position (x, y, z)
			x = xz * sinf(sectorAngle);             // r * cos(u) * sin(v)
			z = xz * cosf(sectorAngle);             // r * cos(u) * cos(v)

													// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / SECTOR_COUNT;
			t = (float)i / STACK_COUNT;

			glm::vec3 up(0.0, 1.0, 0.0);
			glm::vec3 midpointVector(midpoint * sinf(sectorAngle), 0.0f, midpoint * cosf(sectorAngle));
			glm::vec3 tan = glm::cross(up, midpointVector);

			vertices.push_back(make_vertex(x, y, z, tan.x, tan.y, tan.z, nx, ny, nz, s, t));
		}
	}

	std::vector<unsigned int> indices;
	int k1, k2;
	for (int i = 0; i < STACK_COUNT; ++i)
	{
		k1 = i * (SECTOR_COUNT + 1);     // beginning of current stack
		k2 = k1 + SECTOR_COUNT + 1;      // beginning of next stack

		for (int j = 0; j < SECTOR_COUNT; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (STACK_COUNT - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	setupMesh(vertices, indices);
}

Sphere::~Sphere()
{

}