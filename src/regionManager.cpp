#include "chunkDataContainer.h"
#include "chunkList.h"
#include "coordinateContainers.h"
#include "fastFloat.h"
#include <cstring>

long RegionContainer::coordsToKey(const ChunkCoords coords) {
	long result = (fastFloat::mod(coords.x, 2 * RENDER_DISTANCE) * 4 * RENDER_DISTANCE * RENDER_DISTANCE) + (fastFloat::mod(coords.y, 2 * RENDER_DISTANCE) * 2 * RENDER_DISTANCE) + fastFloat::mod(coords.z, 2 * RENDER_DISTANCE);
    return result;
}

int RegionContainer::getIndex(const ChunkCoords chunkCoord) {
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        // return -1;

        for (int i = 0; i < regions.size(); i++) {
        	if (regions.at(i).empty)
        		return i;
        }
        return -1;
    }
    return index;
}

int RegionContainer::getIndex(const int chunkCoordX, const int chunkCoordY, const int chunkCoordZ) {
    ChunkCoords chunkCoord;
    chunkCoord.x = chunkCoordX;
    chunkCoord.y = chunkCoordY;
    chunkCoord.z = chunkCoordZ;
    int index;
    try {
        index = coordToIndexMap.at(coordsToKey(chunkCoord));
    } catch (const std::out_of_range& oor) {
        //std::cerr << "Out of Range error: " << oor.what() << "\n";
        return -1;
    }
    return index;
}



void Region::uploadRegion() {
	if (readyToUpload && mesh.size() > 0) {
		vbo.upload(mesh);

        ready = true;
        readyToUpload = false;
    }
}

void Region::compileMesh() {
	if (shouldCompile) {
	    ready = false;
		int crntBaseVertex = 0;
	    bool shallI = false;

	    for (int i = 0; i < REGION_SIZE * REGION_SIZE * REGION_SIZE; i++) {
	    	baseVertex[i] += crntBaseVertex;

		    indexOffset[i] = BUFFER_OFFSET(6 * (baseVertex[i] / 8) * sizeof(int));

	    	if (isChunkNull.test(i)) {
	    		if (chunksInRegion[i]->redoRegionMesh) {
	    			
	    			mesh.erase(mesh.begin() + baseVertex[i], mesh.begin() + baseVertex[i] + (8 * (count[i] / 6)));
	    			mesh.insert(mesh.begin() + baseVertex[i], chunksInRegion[i]->mesh.begin(), chunksInRegion[i]->mesh.end());

	    			crntBaseVertex += chunksInRegion[i]->mesh.size() - (8 * (count[i] / 6));
	    			count[i] = 6 * (chunksInRegion[i]->mesh.size() / 8);

	    			chunksInRegion[i]->mesh.clear();

	    			chunksInRegion[i]->redoRegionMesh = false;

	    			shallI = true;
	    		}
	    	}
	    }

	    ready = !shallI;
	    readyToUpload = shallI;
	    shouldCompile = false;
	}
}