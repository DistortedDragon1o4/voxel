#include "../include/chunkList.h"
#include "buffers.h"
#include "chunkDataContainer.h"
#include "coordinateContainers.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/matrix.hpp"

Renderer::Renderer(Shader &voxelShader, TextureArray &voxelBlockTextureArray, Camera &camera, WorldContainer &worldContainer, ChunkLighting &lighting, BlockDefs &blocks,  HighlightCursor &highlightCursor, std::string dir) :
	voxelShader(voxelShader),
	voxelBlockTextureArray(voxelBlockTextureArray),
	shadowMapShader(dir + "/shaders/shadowMapVertex.glsl", dir + "/shaders/shadowMapFragment.glsl"),
	memAllocator(64 * 1024 * 1024, 16),
	frustumCuller(dir + "/shaders/frustumCullingCompute.glsl"),
	createDrawCommands(dir + "/shaders/createDrawCommands.glsl"),
	camera(camera),
	worldContainer(worldContainer),
	lighting(lighting),
	blocks(blocks),
	highlightCursor(highlightCursor) {

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
		locLightSpaceMatrix0 = glGetUniformLocation(voxelShader.ID, "lightSpaceMatrix0");
		locLightSpaceMatrix1 = glGetUniformLocation(voxelShader.ID, "lightSpaceMatrix1");
		locLightSpaceMatrix2 = glGetUniformLocation(voxelShader.ID, "lightSpaceMatrix2");
		locLightSpaceMatrix3 = glGetUniformLocation(voxelShader.ID, "lightSpaceMatrix3");

		commandBuffer.create();
		commandBuffer.allocate(2 * RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * sizeof(DrawCommandContainer), GL_DYNAMIC_STORAGE_BIT);
		commandBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);

		chunkViewableBuffer.create();
		chunkViewableBuffer.allocate((RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * sizeof(unsigned int)) + (2 * sizeof(unsigned int)), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
		chunkViewableBuffer.map(2 * sizeof(unsigned int), RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * sizeof(unsigned int), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		chunkViewableBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);

		locCamPosFRC = glGetUniformLocation(frustumCuller.ID, "camPos");
		locCameraMatrixPosFRC = glGetUniformLocation(frustumCuller.ID, "cameraMatrix");

		locLightSpaceMatrix0FRC = glGetUniformLocation(frustumCuller.ID, "lightSpaceMatrix0");
		locLightSpaceMatrix1FRC = glGetUniformLocation(frustumCuller.ID, "lightSpaceMatrix1");
		locLightSpaceMatrix2FRC = glGetUniformLocation(frustumCuller.ID, "lightSpaceMatrix2");
		locLightSpaceMatrix3FRC = glGetUniformLocation(frustumCuller.ID, "lightSpaceMatrix3");

		shadowMapFBO.create();
		shadowMapDepthTexture.create(GL_TEXTURE_2D_ARRAY, 2);
		shadowMapDepthTexture.genTextureArray2d(2048, 2048, 4, GL_DEPTH_COMPONENT32F);
		shadowMapFBO.bindTexture(shadowMapDepthTexture);

		shadowMapDepthTexture.texUnit(voxelShader, "shadowMap");

		locLightSpaceMatrix0shadowMap = glGetUniformLocation(shadowMapShader.ID, "lightSpaceMatrix0");
		locLightSpaceMatrix1shadowMap = glGetUniformLocation(shadowMapShader.ID, "lightSpaceMatrix1");
		locLightSpaceMatrix2shadowMap = glGetUniformLocation(shadowMapShader.ID, "lightSpaceMatrix2");
		locLightSpaceMatrix3shadowMap = glGetUniformLocation(shadowMapShader.ID, "lightSpaceMatrix3");
	}

void Renderer::regionCompileRoutine() {

	std::array<unsigned int, RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE> chunkVisibleArr = {0};

	bool registerModified = false;
	bool memoryBlockModified = false;
	for (int i = 0; i < RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE; i++) {
		ChunkDataContainer &chunk = worldContainer.chunks[i];
		if (chunk.reUploadMesh) {
			registerModified = true;
			memAllocator.deallocate(chunk);
			memoryBlockModified = memAllocator.allocate(chunk);
		}
		chunkVisibleArr[i] = !chunk.unCompiledChunk && (chunk.meshSize > 0);
	}
	if (registerModified) {
		glFlushMappedNamedBufferRange(memAllocator.memoryRegisterBuffer.ID, 0, sizeof(memAllocator.memoryRegister));
	}
	if (memoryBlockModified)
		glFlushMappedNamedBufferRange(memAllocator.memoryBlock.ID, 0, memAllocator.totalMemoryBlockSize);

	std::memcpy(chunkViewableBuffer.persistentMappedPtr, &chunkVisibleArr, sizeof(chunkVisibleArr));
	glFlushMappedNamedBufferRange(chunkViewableBuffer.ID, 0, RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * sizeof(unsigned int));
	
	chunkVisibleArrSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	GLenum waitReturn = GL_UNSIGNALED;
	while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
	    waitReturn = glClientWaitSync(chunkVisibleArrSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);

}

void Renderer::preRenderVoxelWorld() {
	createCameraMatrix();
	shadowMatrix();

	frustumCuller.Activate();

	glUniform3fv(locCamPosFRC, 1, &glm::vec3(camera.oldPosition)[0]);
	glUniformMatrix4fv(locCameraMatrixPosFRC, 1, GL_FALSE, glm::value_ptr(glm::mat4(cameraMatrix)));

	glUniformMatrix4fv(locLightSpaceMatrix0FRC, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[0])));
	glUniformMatrix4fv(locLightSpaceMatrix1FRC, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[1])));
	glUniformMatrix4fv(locLightSpaceMatrix2FRC, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[2])));
	glUniformMatrix4fv(locLightSpaceMatrix3FRC, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[3])));

	frustumCuller.Dispatch(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE / 8, 1, 1);

	createDrawCommands.Activate();
	createDrawCommands.Dispatch(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE / 8, 1, 1);
}

void Renderer::renderVoxelWorld() {
	glViewport(0, 0, 2048, 2048);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// glCullFace(GL_FRONT);

	shadowMapFBO.bind();

	glClear(GL_DEPTH_BUFFER_BIT);

	shadowMapShader.Activate();

	glUniformMatrix4fv(locLightSpaceMatrix0shadowMap, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[0])));
	glUniformMatrix4fv(locLightSpaceMatrix1shadowMap, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[1])));
	glUniformMatrix4fv(locLightSpaceMatrix2shadowMap, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[2])));
	glUniformMatrix4fv(locLightSpaceMatrix3shadowMap, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[3])));

	voxelWorldVertexArray.bind();
    commandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);
	chunkViewableBuffer.bind(GL_PARAMETER_BUFFER);

	glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)(RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE * 16), sizeof(unsigned int), RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE, 0);

	shadowMapFBO.unbind();

	// glCullFace(GL_BACK);



	glViewport(0, 0, camera.width, camera.height);

	glClearColor(0.2f, 0.5f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	voxelShader.Activate();

	glUniformMatrix4fv(locCameraMatrixPos, 1, GL_FALSE, glm::value_ptr(glm::mat4(cameraMatrix)));
	glUniformMatrix4fv(locLightSpaceMatrix0, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[0])));
	glUniformMatrix4fv(locLightSpaceMatrix1, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[1])));
	glUniformMatrix4fv(locLightSpaceMatrix2, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[2])));
	glUniformMatrix4fv(locLightSpaceMatrix3, 1, GL_FALSE, glm::value_ptr(glm::mat4(lightSpaceMatrix[3])));

    
	glUniform3fv(locCamPos, 1, &glm::vec3(camera.Position)[0]);
    glUniform3fv(locCamDir, 1, &glm::vec3(camera.Orientation)[0]);
    glUniform3fv(locSunDir, 1, &glm::vec3(sunDir)[0]);

	glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, 0, RENDER_DISTANCE * RENDER_DISTANCE * RENDER_DISTANCE, 0);

    voxelWorldVertexArray.unbind();
    commandBuffer.unbind(GL_DRAW_INDIRECT_BUFFER);
    chunkViewableBuffer.unbind(GL_PARAMETER_BUFFER);

    glDisable(GL_CULL_FACE);

	highlightCursor.renderCursor(cameraMatrix);

	glDisable(GL_DEPTH_TEST);
}

std::array<glm::dvec4, 8> Renderer::getFrustumCorners(glm::dmat4 frustumMatrix) {
	const auto inv = glm::inverse(frustumMatrix);

	std::array<glm::dvec4, 8> corners;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				const glm::dvec4 pt = inv * glm::dvec4((2.0 * i) - 1.0, (2.0 * j) - 1.0, (2.0 * k) - 1.0, 1.0);
				corners[(i << 2) + (j << 1) + k] = pt / pt.w;
			}
		}
	}

	return corners;
}

void Renderer::shadowMatrix() {

	//	Generating matrices for the different LODs
	for (int i = 0; i < 4; i++) {
		double near = double(RENDER_DISTANCE * CHUNK_SIZE / 2) / double(1 << (4 - i));
		double far = double(RENDER_DISTANCE * CHUNK_SIZE / 2) / double(1 << (3 - i));
		if (i == 0)
			near = 0.001;

		glm::dmat4 crntFrustumMatrix = glm::perspective(glm::radians(90.0), double(camera.width) / double(camera.height), near, far) * glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up);;

		std::array<glm::dvec4, 8> corners = getFrustumCorners(crntFrustumMatrix);

		glm::dvec3 center;
		for (const auto& v : corners)
			center += glm::dvec3(v);

		center = center / 8.0;

		glm::dmat4 lightView = glm::lookAt(center, center - glm::dvec3(sunDir), glm::dvec3(0.0f, 1.0f, 0.0f));

		double minX = std::numeric_limits<double>::max();
		double maxX = std::numeric_limits<double>::lowest();
		double minY = std::numeric_limits<double>::max();
		double maxY = std::numeric_limits<double>::lowest();
		double minZ = std::numeric_limits<double>::max();
		double maxZ = std::numeric_limits<double>::lowest();
		for (const auto& v : corners) {
		    const auto trf = lightView * v;
		    minX = std::min(minX, trf.x);
		    maxX = std::max(maxX, trf.x);
		    minY = std::min(minY, trf.y);
		    maxY = std::max(maxY, trf.y);
		    minZ = std::min(minZ, trf.z);
		    maxZ = std::max(maxZ, trf.z);
		}

		double zMult = 64.0;

		minZ *= zMult;
		maxZ *= zMult;
		   
		const glm::dmat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

		lightSpaceMatrix[i] = lightProjection * lightView;
		lightPos[i] = glm::dvec3(0.0);
	}
}

void Renderer::createCameraMatrix() {
	glm::dmat4 projection = glm::perspective(glm::radians(90.0), double(camera.width) / double(camera.height), 0.001, double(RENDER_DISTANCE * CHUNK_SIZE));
	glm::dmat4 view = glm::lookAt(glm::dvec3(0), glm::dvec3(0) + camera.Orientation, camera.Up);
	cameraMatrix = projection * view;
}