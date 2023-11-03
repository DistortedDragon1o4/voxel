#ifndef HUD_CLASS_H
#define HUD_CLASS_H

#include <iostream>
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
    std::vector<float> box;
    std::vector<uint> ebo;
};

class TextItem {
public:
    int windowWidth;
    int windowHeight;
    int letter = 32;
    int width;
    int height;
    bool visible = true;
    glm::ivec2 position;
    glm::vec3 color;
    void genBox();
    std::vector<float> box;
    std::vector<uint> ebo;
};

class HUD {
public:
    VAO boxVAO;
    void generateMesh();
    void uploadTexture();
    std::vector<GUIItem> list;
    std::vector<TextItem> textList;
    int EBOsize;
};

#endif