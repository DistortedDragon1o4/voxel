#include "../include/HUD.h"

void HUD::generateMesh() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int index = 0;
    EBOsize = 0;
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).visible == true) {
            vertices.insert(vertices.end(), list.at(i).vertices.begin(), list.at(i).vertices.end());
            std::vector<unsigned int> crntEBO;
            for (int j = 0; j < 6; j++)
                crntEBO.push_back(list.at(i).indices.at(j) + index);
            indices.insert(indices.end(), crntEBO.begin(), crntEBO.end());
            EBOsize += crntEBO.size();
            index += 4;
        }
    }
    // std::cout << box.at(0) << " " << box.at(1) << " " << EBOsize << "\n";
    // std::cout << box.at(6) << " " << box.at(7) << " " << ebo.at(6) << "\n";
    boxVAO.Bind();

    VBO VBO1;
    EBO EBO1;

    VBO1.Gen(vertices);
    EBO1.Gen(indices);

    boxVAO.LinkAttribPointer(VBO1, 0, 2, GL_FLOAT, 6 * sizeof(float), (void*)0);
    boxVAO.LinkAttribPointer(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    boxVAO.LinkAttribPointer(VBO1, 2, 1, GL_FLOAT, 6 * sizeof(float), (void*)(5 * sizeof(float)));

    boxVAO.Unbind();

    VBO1.Unbind();
    EBO1.Unbind();

    VBO1.Delete();
    EBO1.Delete();
}

void GUIItem::genBox() {
    vertices = {
        float(width / float(windowWidth)) + float(position.x / float(windowWidth)), float(height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
        float(-width / float(windowWidth)) + float(position.x / float(windowWidth)), float(height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
        float(width / float(windowWidth)) + float(position.x / float(windowWidth)), float(-height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
        float(-width / float(windowWidth)) + float(position.x / float(windowWidth)), float(-height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
    };
    // std::cout << box.at(0) << " " << box.at(1) << "\n";
    indices = {0, 1, 3, 0, 3, 2};
}