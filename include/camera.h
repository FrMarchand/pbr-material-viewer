#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace {
	// Default camera values
	const float SPEED = 0.25f;
	const float DISTANCE = 2.5f;
	const float SENSITIVITY = 0.25f;
	const float ZOOM = 45.0f;
}

class Camera
{
public:
	glm::vec3 mPosition;
	glm::vec3 mUp;
	glm::vec3 mRight;
	glm::vec3 mWorldUp;
	glm::vec3 mTarget;

	float mMovementSpeed;
	float mMouseSensitivity;
	float mZoom;

	bool mDragging = false;
	bool mPanning = false;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.3f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f));
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float targetX, float targetY, float targetZ);


	glm::mat4 getViewMatrix();
	void processMouseMovement(float xoffset, float yoffset);
	void processMousePress(bool leftPressed, bool rightPressed);
	void processMouseScroll(float yoffset);

private:
	void updateCameraVectors();
};

#endif //CAMERA_H
