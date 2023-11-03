#include "../include/HUD.h"

void HUD::generateMesh() {
    std::vector<float> box;
    std::vector<uint> ebo;
    int index = 0;
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).visible == true) {
            box.insert(box.end(), list.at(i).box.begin(), list.at(i).box.end());
            std::vector<uint> crntEBO;
            for (int j = 0; j < 6; j++)
                crntEBO.push_back(list.at(i).ebo.at(j) + index);
            ebo.insert(ebo.end(), crntEBO.begin(), crntEBO.end());
            EBOsize += crntEBO.size();
            index += 4;
        }
    }
    for (int i = 0; i < textList.size(); i++) {
        if (textList.at(i).visible == true) {
            box.insert(box.end(), textList.at(i).box.begin(), textList.at(i).box.end());
            std::vector<uint> crntEBO;
            for (int j = 0; j < 6; j++)
                crntEBO.push_back(textList.at(i).ebo.at(j) + index);
            ebo.insert(ebo.end(), crntEBO.begin(), crntEBO.end());
            EBOsize += crntEBO.size();
            index += 4;
        }
    }
    // std::cout << box.at(0) << " " << box.at(1) << " " << EBOsize << "\n";
    // std::cout << box.at(6) << " " << box.at(7) << " " << ebo.at(6) << "\n";
    boxVAO.Bind();

    VBO VBO1;
    EBO EBO1;

    VBO1.Gen(box);
    EBO1.Gen(ebo);

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
    box = {
        float(width / float(windowWidth)) + float(position.x / float(windowWidth)), float(height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
        float(-width / float(windowWidth)) + float(position.x / float(windowWidth)), float(height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
        float(width / float(windowWidth)) + float(position.x / float(windowWidth)), float(-height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
        float(-width / float(windowWidth)) + float(position.x / float(windowWidth)), float(-height / float(windowHeight)) + float(position.y / float(windowHeight)), color.x, color.y, color.z, 0,
    };
    // std::cout << box.at(0) << " " << box.at(1) << "\n";
    ebo = {0, 1, 3, 0, 3, 2};
}

void TextItem::genBox() {
    width = (height / 9) * 7;
    box = {
        float(width / float(windowWidth)) + float(2 * position.x / float(windowWidth)), float(height / float(windowHeight)) + float(2 * position.y / float(windowHeight)), color.x, color.y, color.z, float(letter - 32),
        float(-width / float(windowWidth)) + float(2 * position.x / float(windowWidth)), float(height / float(windowHeight)) + float(2 * position.y / float(windowHeight)), color.x, color.y, color.z, float(letter - 32),
        float(width / float(windowWidth)) + float(2 * position.x / float(windowWidth)), float(-height / float(windowHeight)) + float(2 * position.y / float(windowHeight)), color.x, color.y, color.z, float(letter - 32),
        float(-width / float(windowWidth)) + float(2 * position.x / float(windowWidth)), float(-height / float(windowHeight)) + float(2 * position.y / float(windowHeight)), color.x, color.y, color.z, float(letter - 32),
    };
    ebo = {0, 1, 3, 0, 3, 2};
}