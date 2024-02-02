#include "chunkList.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

BlockDefs::BlockDefs(std::string dir) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        std::ifstream file(dir + "/assets/models/blocks/" + std::to_string(i) + ".json");

        json data = json::parse(file);

        blocks[i].blockName = data.at("name");
        blocks[i].isSolid = data.at("isSolid");
        blocks[i].castsAO = data.at("castsAO");
        blocks[i].model = data.at("model").get<std::vector<unsigned int>>();
        parseModel(blocks[i].model, blocks[i].blockBitMap, blocks[i].faceType);
    }
}

void BlockDefs::parseModel(std::vector<unsigned int> &model, std::vector<unsigned int> &blockBitMap, std::vector<int> &faceType) {
    for (int i = 0; i < model.size() / 9; i++) {
        int j = 9 * i;
        unsigned int dataBlock1 = (model[j + 2] << 20) + (model[j + 3] << 10) + model[j + 4];
        unsigned int dataBlock2 = (model[j + 8] << 21) + (model[j + 0] << 16) + (model[j + 1] << 11) + (model[j + 5] << 8);
        blockBitMap.push_back(dataBlock1);
        blockBitMap.push_back(dataBlock2);

        if (i % 4 == 0) {
            faceType.push_back((model[j + 6] == 1) ? model[j + 5] : 6);
        }
    }
}

BlockCoords::BlockCoords(int index) {
    x = index / (CHUNK_SIZE * CHUNK_SIZE);
    y = (index / CHUNK_SIZE) % CHUNK_SIZE;
    z = index % CHUNK_SIZE;
}

BlockCoords::BlockCoords(int _x, int _y, int _z) {
    x = _x;
    y = _y;
    z = _z;
}