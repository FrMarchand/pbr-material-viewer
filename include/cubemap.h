#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <glad/glad.h>
#include <string>
#include <vector>

class CubeMap
{
public:
	CubeMap();
	~CubeMap();

	//Delete the copy constructor/assignment.
	CubeMap(const CubeMap &) = delete;
	CubeMap &operator=(const CubeMap &) = delete;

	CubeMap(CubeMap &&other) : mID(other.mID)
	{
		other.mID = 0; //Use the "null" texture for the old object.
	}

	CubeMap &operator=(CubeMap &&other)
	{
		//ALWAYS check for self-assignment.
		if (this != &other)
		{
			release();
			//ID is now 0.
			std::swap(mID, other.mID);
		}
	}

	GLuint getId() const;
	void bind(GLenum textureUnit) const;

private:
	GLuint mID = 0;

	void release();
};

#endif //CUBEMAP_H