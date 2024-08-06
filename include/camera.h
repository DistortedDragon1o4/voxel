#pragma once

#include "glm/fwd.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include <gladContainer.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <math.h>
#include "fastFloat.h"

#include <iostream>

#include "shaderCompiler.h"

// this is temporary only, ps make a proper inputs file
#include <functional>

struct Camera {
    Camera( int &width, int &height, glm::dvec3 position);

    glm::dvec3 Position;
    glm::dvec3 oldPosition;
    glm::dvec3 Orientation = glm::dvec3(1.0, 0.0, 0.0);
    glm::dvec3 Up = glm::dvec3(0.0, 1.0, 0.0);
    glm::dvec2 sphericalOrientation = glm::dvec2(0.0, 0.0); // spherical coordinate system and radius is 1

    double FOVmultiplier = 1.0;

    glm::dmat4 cameraMatrix;

    int &width;
    int &height;

    bool firstClick = true;
    bool captured = false;

    double speed = 0.1;
    double sensitivity = 2.0;


    void matrix(double FOVdeg, double nearPlane, double farPlane);
    void inputs(GLFWwindow* window);

    // this is temporary , ps make a proper inputs file
    std::function<void()> breakBlock;
    std::function<void()> placeBlock;
    int* currentBlockPtr;
    int leftCount = 0;
    int rightCount = 0;

    void mouseInput(GLFWwindow* window);

    glm::dvec2 oldMousePos;
};