#include "../include/chunkList.h"
#include "buffers.h"
#include "chunkDataContainer.h"
#include "coordinateContainers.h"

Renderer::Renderer(Shader &voxelShader, TextureArray &voxelBlockTextureArray, Camera &camera, WorldContainer &worldContainer, ChunkLighting &lighting, BlockDefs &blocks, std::string dir) :
	voxelShader(voxelShader),
	voxelBlockTextureArray(voxelBlockTextureArray),
	worldContainer(worldContainer),
	memAllocator(256 * 1024 * 1024, 16, worldContainer.chunks.size()),
	frustumCuller(dir + "/shaders/frustumCullingCompute.glsl"),
	createDrawCommands(dir + "/shaders/createDrawCommands.glsl"),
	camera(camera),
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

		locCamPos = glGetUniformLocation(voxelShader.ID, "camPos");
		locCamDir = glGetUniformLocation(voxelShader.ID, "camDir");
		locSunDir = glGetUniformLocation(voxelShader.ID, "sunDir");
		locCameraMatrixPos = glGetUniformLocation(voxelShader.ID, "cameraMatrix");

		commandBuffer.create();
		commandBuffer.allocate(worldContainer.chunks.size() * sizeof(DrawCommandContainer), GL_DYNAMIC_STORAGE_BIT);
		commandBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);

		chunkViewableBuffer.create();
		chunkViewableBuffer.allocate((worldContainer.chunks.size() * sizeof(unsigned int)) + sizeof(unsigned int), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
		chunkViewableBuffer.map(sizeof(unsigned int), worldContainer.chunks.size() * sizeof(unsigned int), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		chunkViewableBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);

		locCamPosFRC = glGetUniformLocation(frustumCuller.ID, "camPos");
		locCameraMatrixPosFRC = glGetUniformLocation(frustumCuller.ID, "cameraMatrix");
		// locCamDirFRC = glGetUniformLocation(frustumCuller.ID, "camDir");
		// locFrustumOffset = glGetUniformLocation(frustumCuller.ID, "frustumOffset");
		// locCosineModifiedHalfFOV = glGetUniformLocation(frustumCuller.ID, "cosineModifiedHalfFOV");
	}

void Renderer::regionCompileRoutine() {

	std::vector<unsigned int> chunkVisibleArr = std::vector<unsigned int>(worldContainer.chunks.size());

	bool registerModified = false;
	bool memoryBlockModified = false;
	for (int i = 0; i < worldContainer.chunks.size(); i++) {
		ChunkDataContainer &chunk = worldContainer.chunks[i];
		if (chunk.reUploadMesh) {
			registerModified = true;
			memAllocator.deallocate(chunk);
			memoryBlockModified = memAllocator.allocate(chunk);
			chunk.meshIsUseless = true;
		}
		if (chunk.reUploadLight) {
			registerModified = true;
			memAllocator.lightDeallocate(chunk);
			memoryBlockModified = memAllocator.lightAllocate(chunk);
			chunk.lightingIsUseless = true;
		}
		chunkVisibleArr[i] = (!chunk.unCompiledChunk || chunk.lodRecompilation) && (chunk.meshSize > 0);
		if (chunk.meshIsUseless && chunk.lightingIsUseless && !chunk.unCompiledChunk && chunk.chunkType > 0 && chunk.lightUpdateInstructions.size() == 0)
			chunk.contentsAreUseless = true;
	}
	if (registerModified) {
		glFlushMappedNamedBufferRange(memAllocator.memoryRegisterBuffer.ID, 0, sizeof(MemRegUnit) * memAllocator.memoryRegister.size());
	}
	if (memoryBlockModified)
		glFlushMappedNamedBufferRange(memAllocator.memoryBlock.ID, 0, memAllocator.totalMemoryBlockSize);

	std::memcpy(chunkViewableBuffer.persistentMappedPtr, chunkVisibleArr.data(), chunkVisibleArr.size() * sizeof(unsigned int));
	glFlushMappedNamedBufferRange(chunkViewableBuffer.ID, 0, worldContainer.chunks.size() * sizeof(unsigned int));
	
	chunkVisibleArrSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	GLenum waitReturn = GL_UNSIGNALED;
	while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
	    waitReturn = glClientWaitSync(chunkVisibleArrSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);

}

void Renderer::preRenderVoxelWorld() {
	camera.matrix(90.0, 0.0078125, 1024.0);

	frustumCuller.Activate();

	glUniform3fv(locCamPosFRC, 1, &glm::vec3(camera.oldPosition)[0]);
	glUniformMatrix4fv(locCameraMatrixPosFRC, 1, GL_FALSE, glm::value_ptr(glm::mat4(camera.cameraMatrix)));

	frustumCuller.Dispatch((worldContainer.chunks.size() + 7) / 8, 1, 1);

	createDrawCommands.Activate();
	createDrawCommands.Dispatch((worldContainer.chunks.size() + 7) / 8, 1, 1);
}

void Renderer::renderVoxelWorld() {

	voxelShader.Activate();

	glUniformMatrix4fv(locCameraMatrixPos, 1, GL_FALSE, glm::value_ptr(glm::mat4(camera.cameraMatrix)));
    
    voxelBlockTextureArray.Bind();

	glUniform3fv(locCamPos, 1, &glm::vec3(camera.Position)[0]);
    glUniform3fv(locCamDir, 1, &glm::vec3(camera.Orientation)[0]);
    glUniform3fv(locSunDir, 1, &glm::vec3(sunDir)[0]);

    voxelWorldVertexArray.bind();
    commandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);
	chunkViewableBuffer.bind(GL_PARAMETER_BUFFER);

	glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, 0, worldContainer.chunks.size(), 0);

    voxelWorldVertexArray.unbind();
    commandBuffer.unbind(GL_DRAW_INDIRECT_BUFFER);
    chunkViewableBuffer.unbind(GL_PARAMETER_BUFFER);
}