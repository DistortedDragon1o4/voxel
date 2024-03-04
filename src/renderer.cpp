#include "../include/chunkList.h"
#include "buffers.h"
#include "chunkDataContainer.h"
#include "coordinateContainers.h"

Renderer::Renderer(Shader &voxelShader, TextureArray &voxelBlockTextureArray, Sampler &voxelBlockTextureSampler, Camera &camera, WorldContainer &worldContainer, RegionContainer &regionContainer, ChunkLighting &lighting, BlockDefs &blocks, std::string dir) :
	voxelShader(voxelShader),
	voxelBlockTextureArray(voxelBlockTextureArray),
	voxelBlockTextureSampler(voxelBlockTextureSampler),
	frustumCuller(dir + "/shaders/frustumCullingCompute.glsl"),
	orderingDrawCalls(dir + "/shaders/orderingDrawCalls.glsl"),
	drawCallConstructor(dir + "/shaders/drawCallConstructor.glsl"),
	camera(camera),
	worldContainer(worldContainer),
	regionContainer(regionContainer),
	lighting(lighting),
	blocks(blocks) {

		voxelWorldVertexArray.create();
		voxelWorldVertexArray.bind();

		glEnableVertexArrayAttrib(voxelWorldVertexArray.ID, 0);
		glEnableVertexArrayAttrib(voxelWorldVertexArray.ID, 1);

		glVertexAttribIFormat(0, 1, GL_INT, 0);
		glVertexAttribIFormat(1, 1, GL_INT, sizeof(unsigned int));

		glVertexArrayAttribBinding(voxelWorldVertexArray.ID, 0, 0);
		glVertexArrayAttribBinding(voxelWorldVertexArray.ID, 1, 0);

		voxelWorldVertexArray.unbind();

		// chunkPropertiesBuffer.create(5, regionContainer.regions.size());

		locCamPos = glGetUniformLocation(voxelShader.ID, "camPos");
		locCamDir = glGetUniformLocation(voxelShader.ID, "camDir");
		locSunDir = glGetUniformLocation(voxelShader.ID, "sunDir");

		copyBuffer.create();
		copyBuffer.alloc(ALLOC_1MB_OF_VERTICES);

		ibo.create();
		glVertexArrayElementBuffer(voxelWorldVertexArray.ID, ibo.ID);

		commandBuffer.create();
		commandBuffer.allocate((((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2)) * sizeof(DrawCommandContainer), GL_DYNAMIC_STORAGE_BIT);
		commandBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);

		alternateCommandBuffer.create();
		alternateCommandBuffer.allocate((((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2)) * sizeof(DrawCommandContainer), 0);
		alternateCommandBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);

		regionDataBuffer.create();
		regionDataBuffer.allocate((((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2)) * sizeof(RegionDataContainer), GL_DYNAMIC_STORAGE_BIT);
		regionDataBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);

		chunkViewableBuffer.create();
		// We give 64 bits of data per region
		chunkViewableBuffer.allocate((((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2)) * 64 * sizeof(unsigned int), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
		chunkViewableBuffer.map(GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		chunkViewableBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);

		locCamPosFRC = glGetUniformLocation(frustumCuller.ID, "camPos");
		locCamDirFRC = glGetUniformLocation(frustumCuller.ID, "camDir");
		locFrustumOffset = glGetUniformLocation(frustumCuller.ID, "frustumOffset");
		locCosineModifiedHalfFOV = glGetUniformLocation(frustumCuller.ID, "cosineModifiedHalfFOV");

		for (Region &region : regionContainer.regions) {
			region.vbo.create();
			// region.vbo.alloc(ALLOC_1MB_OF_VERTICES);
		}
	}

void Renderer::generateIBO() {
	indices.clear();
	for (int i = 0; i < maxQuads; i++) {
		std::array<unsigned int, 6> crntIBO = blocks.solidBlock.dataEBO;
	    for (int j = 0; j < 6; j++) {
	        crntIBO[j] += i * 4;
	    }
	    indices.insert(indices.end(), crntIBO.begin(), crntIBO.end());
	}
	ibo.upload(indices);
}

void Renderer::regionCompileRoutine() {
	oldMaxQuads = maxQuads;

	std::array<unsigned int, 64 * (((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2) * ((RENDER_DISTANCE / 4) + 2))> chunkVisibleArr;

	for (int i = 0; i < regionContainer.regions.size(); i++) {
		Region &region = regionContainer.regions[i];
		if (region.empty == false) {
			region.compileMesh();
			if (region.readyToUpload) {
				populateRegionData(region, i);
			}
			region.uploadRegion();

			int numQuads = region.mesh.size() / 8;
			if (numQuads > maxQuads)
				maxQuads = numQuads;

			region.numFilledChunks = 0;
			for (int j = 0; j < REGION_SIZE * REGION_SIZE * REGION_SIZE; j++) {
				if (region.isChunkNull.test(j))
					chunkVisibleArr[(64 * i) + j] = region.chunksInRegion[j]->meshSize > 0 && !region.chunksInRegion[j]->unCompiledChunk/* && region.chunksInRegion[j]->occlusionUnCulled*/ && region.ready && !region.empty;
				else
					chunkVisibleArr[(64 * i) + j] = false;
			}
		}
	}

	GLenum waitReturn = GL_UNSIGNALED;
	while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
	    waitReturn = glClientWaitSync(chunkVisibleArrSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);

	std::memcpy(chunkViewableBuffer.persistentMappedPtr, &chunkVisibleArr, sizeof(chunkVisibleArr));

	glFlushMappedNamedBufferRange(chunkViewableBuffer.ID, 0, sizeof(chunkVisibleArr));

	if (maxQuads > oldMaxQuads)
		generateIBO();
}

void Renderer::uploadMeshes(Region &region) {
	if (region.shouldCompile) {
		region.ready = false;

		int crntBaseVertex = 0;

		for (int i = 0; i < REGION_SIZE * REGION_SIZE * REGION_SIZE; i++) {
	    	region.baseVertex.at(i) += crntBaseVertex;

		    // region.indexOffset.at(i) = BUFFER_OFFSET(6 * (region.baseVertex.at(i) / 8) * sizeof(int));

	    	if (region.isChunkNull.test(i)) {
	    		if (region.chunksInRegion.at(i)->redoRegionMesh) {

	    			crntBaseVertex += region.chunksInRegion.at(i)->mesh.size() - (8 * (region.count.at(i) / 6));

	    			int offset = (region.baseVertex.at(i) + (8 * (region.count.at(i) / 6))) / 2;
	    			int size = region.numFilledVertices - ((region.baseVertex.at(i) + (8 * (region.count.at(i) / 6))) / 2);
	    			int shiftBy = (region.chunksInRegion.at(i)->mesh.size() - (8 * (region.count.at(i) / 6))) / 2;

	    			while (copyBuffer.numVertices < size) {
	    				copyBuffer.reAlloc(ALLOC_1MB_OF_VERTICES + copyBuffer.numVertices);
	    			}
	    			while (region.vbo.numVertices < region.numFilledVertices + shiftBy) {
	    				region.vbo.reAlloc(ALLOC_1MB_OF_VERTICES + region.vbo.numVertices);
	    			}
	    			
	    			region.numFilledVertices += shiftBy;

	    			region.vbo.shift(copyBuffer.ID, offset, size, shiftBy);

	    			glClientWaitSync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), GL_SYNC_FLUSH_COMMANDS_BIT, 0);

	    			region.vbo.subUpload(region.baseVertex.at(i) / 2, region.chunksInRegion.at(i)->mesh);

	    			region.count.at(i) = 6 * (region.chunksInRegion.at(i)->mesh.size() / 8);

	    			region.chunksInRegion.at(i)->redoRegionMesh = false;

	    			// shallI = true;
	    		}
	    	}
	    }

	    region.shouldCompile = false;
	    region.ready = true;
	}
}

void Renderer::populateRegionData(Region &region, int index) {
	DrawCommandContainer drawCommands;
	for (int j = 0; j < REGION_SIZE * REGION_SIZE * REGION_SIZE; j++) {
    	if (!region.isChunkNull.test(j)) {
    		DrawCommand crntCommand = {
				.count = 0,
				.instanceCount = 0,
				.firstIndex = 0,
				.baseVertex = 0,
				.baseInstance = 0,
			};

			drawCommands.drawCommands[j] = crntCommand;
			// break;
    	} else {

	    	DrawCommand crntCommand = {
				.count = unsigned(region.count[j]),
				.instanceCount = 0, // region.chunksInRegion[j]->meshSize > 0 && region.chunksInRegion[j]->unCompiledChunk == 0 && region.chunksInRegion[j]->occlusionUnCulled == true/* && region.chunksInRegion[j]->frustumVisible == true*/,
				.firstIndex = unsigned(6 * (region.baseVertex[j] / 8)),
				.baseVertex = 0,
				.baseInstance = unsigned(index),
			};

			drawCommands.drawCommands[j] = crntCommand;
			region.regionData.chunkProperties[j].chunkID = region.chunksInRegion[j]->chunkID;
	    	region.regionData.chunkProperties[j].index = region.chunksInRegion[j]->neighbouringChunkIndices[13];
		}
	}

	commandBuffer.upload(&drawCommands, sizeof(DrawCommandContainer), index * sizeof(DrawCommandContainer));
	regionDataBuffer.upload(&region.regionData, sizeof(RegionDataContainer), index * sizeof(RegionDataContainer));
}

void Renderer::preRenderVoxelWorld() {

	double frustumOffset = (double(CHUNK_SIZE) / 2) / tan(FOV / 2);
    double cosineModifiedHalfFOV = cos(atan(screenDiag / (screenHeight / tan(FOV / 2))));

	frustumCuller.Activate();

	glUniform3fv(locCamPosFRC, 1, &glm::vec3(camera.Position)[0]);
	glUniform3fv(locCamDirFRC, 1, &glm::vec3(camera.Orientation)[0]);

	glUniform1f(locFrustumOffset, frustumOffset);
	glUniform1f(locCosineModifiedHalfFOV, cosineModifiedHalfFOV);

	frustumCuller.Dispatch(((RENDER_DISTANCE / 4) + 2), ((RENDER_DISTANCE / 4) + 2), ((RENDER_DISTANCE / 4) + 2));

	orderingDrawCalls.Activate();
	orderingDrawCalls.Dispatch(((RENDER_DISTANCE / 4) + 2), ((RENDER_DISTANCE / 4) + 2), ((RENDER_DISTANCE / 4) + 2));

	drawCallConstructor.Activate();
	drawCallConstructor.Dispatch(((RENDER_DISTANCE / 4) + 2), ((RENDER_DISTANCE / 4) + 2), ((RENDER_DISTANCE / 4) + 2));
}

void Renderer::renderVoxelWorld() {

	voxelShader.Activate();
    // please remember to change FOV in the render calls frustum culling
    camera.matrix(90.0, 0.001, 512.0, voxelShader, "cameraMatrix");

    // voxelBlockTextureSampler.bind(0);
    voxelBlockTextureArray.Bind();

	glUniform3fv(locCamPos, 1, &glm::vec3(camera.Position)[0]);
    glUniform3fv(locCamDir, 1, &glm::vec3(camera.Orientation)[0]);
    glUniform3fv(locSunDir, 1, &glm::vec3(sunDir)[0]);

    voxelWorldVertexArray.bind();
    alternateCommandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);

	chunkViewableBuffer.bind(GL_PARAMETER_BUFFER);



    for (int i = 0; i < regionContainer.regions.size(); i++) {
		Region &region = regionContainer.regions[i];
	    if (region.ready && !region.empty && region.mesh.size() > 0) {

	    	glVertexArrayVertexBuffer(voxelWorldVertexArray.ID, 0, region.vbo.ID, 0, 2 * sizeof(unsigned int));

	    	// glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)(i * sizeof(DrawCommandContainer)), std::min((chunkVisibleArr[(64 * i) + 63]), unsigned(64)), 0);

	    	glMultiDrawElementsIndirectCount(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)(i * sizeof(DrawCommandContainer)), ((64 * i) + 63) * sizeof(int), 64, 0);
	    }
    }

    voxelWorldVertexArray.unbind();
    alternateCommandBuffer.unbind(GL_DRAW_INDIRECT_BUFFER);
    chunkViewableBuffer.unbind(GL_PARAMETER_BUFFER);

    chunkVisibleArrSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}