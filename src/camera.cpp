#include "../include/camera.h"
#include "glm/geometric.hpp"
#include <GLFW/glfw3.h>
#include <numbers>

Camera::Camera(int &width, int &height, glm::dvec3 position) : width(width), height(height) {
	Position = position;
}

void Camera::matrix(double FOVdeg, double nearPlane, double farPlane) {
	glm::dmat4 view = glm::dmat4(1.0f);
	glm::dmat4 projection = glm::dmat4(0.0);

	view = glm::lookAt(glm::dvec3(0), glm::dvec3(0) + Orientation, Up);
	// projection = glm::perspectiveFovZO(glm::radians(FOVmultiplier * FOVdeg), double(width), double(height), nearPlane, farPlane);

	double fov = glm::radians(FOVmultiplier * FOVdeg);
	double h = glm::cos(0.5 * fov) / glm::sin(0.5 * fov);
	double w = h * height / width;

	projection[0][0] = w;
	projection[1][1] = h;
	projection[2][2] = nearPlane / (farPlane - nearPlane);
	projection[3][2] = (farPlane * nearPlane) / (farPlane - nearPlane);
	projection[2][3] = -1.0;

	cameraMatrix = projection * view;

	// glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(glm::mat4(cameraMatrix)));
}

void Camera::inputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		Position += speed * glm::normalize(glm::dvec3(Orientation.x, 0.0f, Orientation.z));
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Position -= speed * glm::normalize(glm::cross(Orientation, Up));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		Position -= speed * glm::normalize(glm::dvec3(Orientation.x, 0.0f, Orientation.z));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		Position += speed * Up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		Position -= speed * Up;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		FOVmultiplier = 0.25;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
		FOVmultiplier = 1.0;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		speed = 0.7f;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
		speed = 0.1f;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (!captured) {
			// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			captured = true;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		captured = false;
		firstClick = true;
	}

	if (captured) {
		if (firstClick) {
			glfwGetCursorPos(window, &oldMousePos.x, &oldMousePos.y);
			firstClick = false;
		}

		glm::dvec2 mousePos;
		glfwGetCursorPos(window, &mousePos.x, &mousePos.y);

		glm::dvec2 diff = mousePos - oldMousePos;

		double rotX = sensitivity * (diff.y / double(height));	// means rotation about X axis
		double rotY = sensitivity * (diff.x / double(width));	// means rotation about Y axis

		sphericalOrientation.x += rotY;
		if (sphericalOrientation.x > std::numbers::pi)
			sphericalOrientation.x -= 2 * std::numbers::pi;
		else if (sphericalOrientation.x <= -std::numbers::pi)
			sphericalOrientation.x += 2 * std::numbers::pi;

		sphericalOrientation.y -= rotX;
		if (sphericalOrientation.y >= (std::numbers::pi / 2))
			sphericalOrientation.y = (std::numbers::pi / 2) - 0.000001;
		else if (sphericalOrientation.y < -(std::numbers::pi / 2))
			sphericalOrientation.y = -(std::numbers::pi / 2) + 0.000001;

		Orientation.x = cos(sphericalOrientation.x) * cos(sphericalOrientation.y);
		Orientation.y = sin(sphericalOrientation.y);
		Orientation.z = sin(sphericalOrientation.x) * cos(sphericalOrientation.y);

		oldMousePos = mousePos;

		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
			oldPosition = Position;
		}
	}



	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		std::cout << "X: " << Position.x << "\n";
		std::cout << "Y: " << Position.y << "\n";
		std::cout << "Z: " << Position.z << "\n";
		std::cout << "\n";
		std::cout << "chunkX: " << fastFloat::fastFloor(Position.x / 16) << "\n";
		std::cout << "chunkY: " << fastFloat::fastFloor(Position.y / 16) << "\n";
		std::cout << "chunkZ: " << fastFloat::fastFloor(Position.z / 16) << "\n";
		std::cout << "\n\n\n";
	}
}

void Camera::mouseInput(GLFWwindow *window) {
	if (captured) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			if (leftCount % 8 == 0)
				breakBlock();
			leftCount++;
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
			leftCount = 0;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			if (rightCount % 8 == 0)
				placeBlock();
			rightCount++;
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
			rightCount = 0;
	}
}