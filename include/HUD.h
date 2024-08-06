#pragma once

#include <iostream>
#include "buffers.h"
#include "glm/glm.hpp"
#include <vector>
#include "VAO.h"
#include "texture.h"

class GUIItem {
public:
    int windowWidth;
    int windowHeight;
    int width;
    int height;
    bool visible = true;
    glm::ivec2 position;
    glm::vec3 color;
    void genBox();
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

class HUD {
public:
    VAO boxVAO;
    void generateMesh();
    void uploadTexture();
    std::vector<GUIItem> list;
    int EBOsize = 0;
};