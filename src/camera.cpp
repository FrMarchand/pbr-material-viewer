#include "camera.h"

namespace {
	const float minDistance = 0.01f;
	const float maxDistance = 50.0f;
	const float maxPitch = 89.0f;
	const float minPitch = -89.0f;
}

Camera::Camera(glm::vec3 position, glm::vec3 up, glm::vec3 target)
	: mMovementSpeed(SPEED)
	, mMouseSensitivity(SENSITIVITY)
	, mZoom(ZOOM)
{
	mPosition = position;
	mWorldUp = up;
	mTarget = target;
	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float targetX, float targetY, float targetZ)
	: mMovementSpeed(SPEED)
	, mMouseSensitivity(SENSITIVITY)
	, mZoom(ZOOM)
{
	mPosition = glm::vec3(posX, posY, posZ);
	mWorldUp = glm::vec3(upX, upY, upZ);
	mTarget = glm::vec3(targetX, targetY, targetZ);
	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(mPosition, mTarget, mUp);
}

void Camera::processMouseMovement(float xoffset, float yoffset)
{
	if (mDragging) {
		xoffset *= mMouseSensitivity;
		yoffset *= mMouseSensitivity;

		float yaw = -xoffset;
		float pitch = -yoffset;

		glm::vec3 focusVector = mPosition - mTarget;

		// pitch constraints
		float currentPitch = glm::degrees(glm::asin(focusVector.y / glm::length(focusVector)));
		float newPitch = currentPitch + pitch;

		if (newPitch > maxPitch) {
			pitch = maxPitch - currentPitch;
		}

		if (newPitch < minPitch) {
			pitch = minPitch - currentPitch;
		}

		glm::mat4 rotation(1.0f);
		rotation = glm::rotate(rotation, glm::radians(pitch), mRight);
		rotation = glm::rotate(rotation, glm::radians(yaw), mWorldUp);

		focusVector = rotation * glm::vec4(focusVector, 1.0f);

		mPosition = focusVector + mTarget;
		updateCameraVectors();
	}
	else if (mPanning) {
		float distance = glm::length(mPosition - mTarget);
		float sensitivity = distance / 1000.0f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		glm::vec3 pan = (glm::normalize(mRight) * xoffset) - (glm::normalize(mUp) * yoffset);
		mTarget += pan;
		mPosition += pan;
	}
}

void Camera::processMousePress(bool leftPressed, bool rightPressed)
{
	if (leftPressed && rightPressed) {
		mDragging = false;
		mPanning = false;
	}
	else {
		mDragging = leftPressed;
		mPanning = rightPressed;
	}
}

void Camera::processMouseScroll(float yoffset)
{
	yoffset *= -SPEED;
	glm::vec3 delta = glm::normalize(mPosition - mTarget) * yoffset;
	glm::vec3 limit = mPosition - mTarget;
	glm::vec3 newPosition = mPosition + delta;

	if (yoffset < 0.0f) {
		// Dolly in
		if (glm::length(delta) >= glm::length(limit) // camera overshoots target
			|| glm::length(newPosition - mTarget) < minDistance)  // camera closer than min distance
		{
			// Set position to minimum distance
			mPosition = (glm::normalize(mPosition - mTarget) * minDistance) + mTarget;
		}
		else {
			mPosition = newPosition;
		}
	}
	else if (yoffset > 0.0f) {
		// Dolly out
		if (glm::length(newPosition - mTarget) > maxDistance) { // camera further than max distance
			// Set position to maximum distance
			mPosition = (glm::normalize(mPosition - mTarget) * maxDistance) + mTarget;
		}
		else {
			mPosition = newPosition;
		}
	}
}

void Camera::updateCameraVectors()
{
	glm::vec3 focusVector = glm::normalize(mPosition - mTarget);
	mRight = glm::normalize(glm::cross(focusVector, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, focusVector));
}
