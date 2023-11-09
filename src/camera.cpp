#include "../include/camera.h"
#include "glm/geometric.hpp"
#include <GLFW/glfw3.h>
#include <numbers>

Camera::Camera(int width, int height, glm::dvec3 position) {
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::matrix(double FOVdeg, double nearPlane, double farPlane, Shader& shader, const char* uniform) {
	glm::dmat4 view = glm::dmat4(1.0f);
	glm::dmat4 projection = glm::dmat4(1.0f);

	view = glm::lookAt(Position, Position + Orientation, Up);
	projection = glm::perspective(glm::radians(FOVdeg), double(width / height), nearPlane, farPlane);

	glUniformMatrix4dv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
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
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		speed = 0.7f;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
		speed = 0.1f;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (!captured) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
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
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		double diffX = (mouseY - (height / 2));
		double diffY = (mouseX - (width / 2));

		if (abs(diffX) == 0.5)
			diffX = 0;
		if (abs(diffY) == 0.5)
			diffY = 0;

		double rotX = sensitivity * (diffX / height);	// means rotation about X axis
		double rotY = sensitivity * (diffY / width);	// means rotation about Y axis

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

		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
			glfwSetCursorPos(window, (width / 2), (height / 2));
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