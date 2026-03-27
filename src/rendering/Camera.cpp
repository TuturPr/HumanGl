#include "../headers/Camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: Position(position),
	  Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	  Up(up),
	  Right(glm::vec3(1.0f, 0.0f, 0.0f)),
	  WorldUp(up),
	  Target(glm::vec3(0.0f)),
	  Yaw(yaw), Pitch(pitch), Distance(5.0f),
	  MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	: Position(glm::vec3(posX, posY, posZ)),
	  Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	  Up(glm::vec3(upX, upY, upZ)),
	  Right(glm::vec3(1.0f, 0.0f, 0.0f)),
	  WorldUp(glm::vec3(upX, upY, upZ)),
	  Target(glm::vec3(0.0f)),
	  Yaw(yaw), Pitch(pitch), Distance(5.0f),
	  MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
	// Caméra regarde dans la direction Front (système normal)
	return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
	if (direction == UP)
		Position += Up * velocity;
	if (direction == DOWN)
		Position -= Up * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw   += xoffset;
	Pitch += yoffset;

	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
	Zoom -= (float)yoffset;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
	// Calculer la direction en fonction de Yaw et Pitch (système normal)
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);

	// Vecteurs de la caméra
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up    = glm::normalize(glm::cross(Right, Front));
}

void Camera::SetTarget(glm::vec3 target)
{
	Target = target;
	Up    = glm::normalize(glm::cross(Right, Front));
}

void Camera::SetDistance(float dist)
{
	Distance = dist;
	if (Distance < 0.5f)
		Distance = 0.5f;
	if (Distance > 50.0f)
		Distance = 50.0f;
}
