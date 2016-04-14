///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#include "Scene.h"

#include "ModelLoader.h"

#include "GL_utilities.h"
#include "glm\gtx\transform.hpp"
#include "glm\gtc\constants.hpp"

#include <iostream>
#include <sstream>

bool Scene::isInitialized = false;
TwType* Scene::resTwType = nullptr;
TwType* Scene::viewTwType = nullptr;
TwType* Scene::sceneTwStruct = nullptr;
TwType* Scene::sceneOptionsTwStruct = nullptr;
TwType* Scene::drawIndTwStruct = nullptr;

Scene::Scene() {
	options.skipNoTexture = false;
	options.drawModels = true;
	options.drawVoxels = false;
	options.drawTextures = false;

	param.voxelRes = 64;
	param.voxelLayer = 0;
	param.voxelDraw = 0;
	param.view = 0;
	param.numMipLevels = (GLuint)log2(param.voxelRes) + 1;
	param.mipLevel = 0;

	drawIndCmd.baseInstance = 0;
	drawIndCmd.instanceCount = 0;
	drawIndCmd.firstVertex = 0;
	drawIndCmd.baseVertex = 0;
	drawIndCmd.vertexCount = 0;

	maxVertex = nullptr;
	minVertex = nullptr;

	models = new std::vector<Model*>();
	voxelModel = new Model();

	voxel2DTex = 0;
	voxelTex = 0;
	mutexTex = 0;
}

bool Scene::Init(const char* path, ShaderList* initShaders) {

	shaders = initShaders;

	if(!InitializeAntBar()) return false;

	GenViewTexture(&voxel2DTex);
	GenVoxelTexture(&voxelTex);

	// Init the framebuffer for drawing
	glGenFramebuffers(1, &voxelFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, param.voxelRes);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, param.voxelRes);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set constant uniforms for the drawing programs
	glUseProgram(shaders->texture);
	glUniform1i(glGetUniformLocation(shaders->texture, "diffuseUnit"), 0);

	glUseProgram(shaders->mask);
	glUniform1i(glGetUniformLocation(shaders->mask, "diffuseUnit"), 0);
	glUniform1i(glGetUniformLocation(shaders->mask, "maskUnit"), 1);

	// Set constant uniforms for voxel programs
	glUseProgram(shaders->voxelize);
	glUniform1i(glGetUniformLocation(shaders->voxelize, "voxelTextures"), 2);
	glUniform1i(glGetUniformLocation(shaders->voxelize, "voxelData"), 3);
	glUniform1i(glGetUniformLocation(shaders->voxelize, "voxelDataNextLevel"), 4);

	// Set constant uniforms for voxel programs
	glUseProgram(shaders->voxelizeTexture);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "diffuseUnit"), 0);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "voxelTextures"), 2);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "voxelData"), 3);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "voxelDataNextLevel"), 4);

	// Set constant uniforms for simple triangle drawing
	glUseProgram(shaders->singleTriangle);
	glUniform1i(glGetUniformLocation(shaders->singleTriangle, "voxelTextures"), 2);
	glUniform1i(glGetUniformLocation(shaders->singleTriangle, "voxelData"), 3);

	// Set constant uniforms for drawing the voxel overlay
	glUseProgram(shaders->voxel);
	glUniform1i(glGetUniformLocation(shaders->voxel, "voxelData"), 3);

	// Set non-constant uniforms for all programs
	glGenBuffers(1, &sceneBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, SCENE, sceneBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneParam), NULL, GL_STREAM_DRAW);

	ModelLoader modelLoader;
	if(!modelLoader.LoadScene(path, models, shaders, &maxVertex, &minVertex)) {
		std::cout << "Failed to load scene: " << path << std::endl;
		return false;
	}
	
	// Load a model for drawing the voxel
	if(!modelLoader.LoadModel("resources/voxelLarge.obj", voxelModel, shaders->voxel)) {
		std::cout << "Failed to load voxel model" << std::endl;
		return false;
	}

	// Calculate the scaling of the scene
	glm::vec3 diffVector = (*maxVertex - *minVertex);
	centerVertex = diffVector / 2.0f + *minVertex;
	scale = glm::max(diffVector.x, glm::max(diffVector.y, diffVector.z));

	// Set the matrices for looking at the scene in three different ways
	param.MTOmatrix[2] = glm::scale(glm::vec3(1.99f / scale)) * glm::translate(-centerVertex);
	param.MTOmatrix[0] = glm::rotate(-glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)) * param.MTOmatrix[2];
	param.MTOmatrix[1] = glm::rotate(glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)) * param.MTOmatrix[2];

	param.MTWmatrix = glm::inverse(param.MTOmatrix[2]);

	drawIndCmd.vertexCount = (GLuint)voxelModel->GetNumIndices();

	glGenBuffers(1, &sparseListBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPARSE_LIST, sparseListBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLint) * param.voxelRes * param.voxelRes * param.voxelRes, NULL, GL_STREAM_DRAW);

	voxelModel->SetPositionData(sparseListBuffer);

	UploadParams();

	// Draw Indirect Command buffer for drawing voxels
	glGenBuffers(1, &drawIndBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DRAW_IND, drawIndBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand), &drawIndCmd, GL_STREAM_DRAW);

	return true;
}

bool Scene::InitializeAntBar() {
	// This should only be done for the first scene
	if(!isInitialized) {
		// AntTweakBar Stuff
		viewTwEnum[0] = { VIEW_X, "Along X" };
		viewTwEnum[1] = { VIEW_Y, "Along Y" };
		viewTwEnum[2] = { VIEW_Z, "Along Z" };
		viewTwType = new TwType;
		*viewTwType = TwDefineEnum("Views", viewTwEnum, 3);

		resTwEnum[0] = { RES16, "16 ^ 3" };
		resTwEnum[1] = { RES32, "32 ^ 3" };
		resTwEnum[2] = { RES64, "64 ^ 3" };
		resTwEnum[3] = { RES128, "128 ^ 3" };
		resTwEnum[4] = { RES256, "256 ^ 3" };
		resTwEnum[5] = { RES512, "512 ^ 3" };
		resTwType = new TwType;
		*resTwType = TwDefineEnum("Resolution", resTwEnum, 6);

		sceneTwMembers[0] = { "DrawVoxelData", TW_TYPE_BOOL32, offsetof(SceneParam, voxelDraw), "  " };
		sceneTwMembers[1] = { "Direction", *viewTwType, offsetof(SceneParam, view), "  " };
		sceneTwMembers[2] = { "Resolution", *resTwType, offsetof(SceneParam, voxelRes), "  " };
		sceneTwMembers[3] = { "Layer", TW_TYPE_UINT32, offsetof(SceneParam, voxelLayer), " min=0 max=511  " };
		sceneTwMembers[4] = { "MipLevels", TW_TYPE_UINT32, offsetof(SceneParam, numMipLevels), " readonly=true " };
		sceneTwMembers[5] = { "MipLevel", TW_TYPE_UINT32, offsetof(SceneParam, mipLevel), " min=0 max=9  " };
		sceneTwStruct = new TwType;
		*sceneTwStruct = TwDefineStruct("SceneGPUStruct", sceneTwMembers, 6, sizeof(SceneParam), NULL, NULL);

		sceneOptionTwMembers[0] = { "SkipNoTexture", TW_TYPE_BOOL8, offsetof(SceneOptions, skipNoTexture), "  " };
		sceneOptionTwMembers[1] = { "DrawVoxels", TW_TYPE_BOOL8, offsetof(SceneOptions, drawVoxels), "  " };
		sceneOptionTwMembers[2] = { "DrawModels", TW_TYPE_BOOL8, offsetof(SceneOptions, drawModels), "  " };
		sceneOptionTwMembers[3] = { "DrawVoxelTextures", TW_TYPE_BOOL8, offsetof(SceneOptions, drawTextures), " key=t " };
		sceneOptionsTwStruct = new TwType;
		*sceneOptionsTwStruct = TwDefineStruct("SceneOptionsStruct", sceneOptionTwMembers, 4, sizeof(SceneOptions), NULL, NULL);

		drawIndTwMembers[0] = { "VertexCount", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, vertexCount), " readonly=true " };
		drawIndTwMembers[1] = { "InstanceCount", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, instanceCount), " readonly=true " };
		drawIndTwMembers[2] = { "FirstVertex", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, firstVertex), " readonly=true " };
		drawIndTwMembers[3] = { "BaseVertex", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, baseVertex), " readonly=true " };
		drawIndTwMembers[4] = { "BaseInstance", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, baseInstance), " readonly=true " };
		drawIndTwStruct = new TwType;
		*drawIndTwStruct = TwDefineStruct("DrawIndStruct", drawIndTwMembers, 5, sizeof(DrawElementsIndirectCommand), NULL, NULL);

		// Check if AntTweak Setup is ok
		if(TwGetLastError() != NULL) return false;

		isInitialized = true;
	}

	return true;
}

// Generate textures for render to texture, only for debugging purposes
void Scene::GenViewTexture(GLuint* viewID) {
	if(*viewID != 0) {
		glDeleteTextures(1, viewID);
	}
	glGenTextures(1, viewID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, *viewID);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, param.voxelRes, param.voxelRes, 3);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

// Create the 3D texture that contains the voxel data
void Scene::GenVoxelTexture(GLuint* texID) {
	if(*texID != 0) {
		glDeleteTextures(1, texID);
	}
	glGenTextures(1, texID);
	glBindTexture(GL_TEXTURE_3D, *texID);
	glTexStorage3D(GL_TEXTURE_3D, param.numMipLevels, GL_R32UI, param.voxelRes, param.voxelRes, param.voxelRes);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
}

void Scene::UploadParams() {
	// Upload new params to GPU
	glBindBufferBase(GL_UNIFORM_BUFFER, SCENE, sceneBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(SceneParam), &param);
}

void Scene::UpdateBuffers() {
	glBindBufferBase(GL_UNIFORM_BUFFER, SCENE, sceneBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(SceneParam), &param);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DRAW_IND, drawIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPARSE_LIST, sparseListBuffer);
}

void Scene::ResizeBuffer() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPARSE_LIST, sparseListBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLint) * param.voxelRes * param.voxelRes * param.voxelRes, NULL, GL_STREAM_DRAW);
}

void Scene::Voxelize() {
	GLint origViewportSize[4];
	glGetIntegerv(GL_VIEWPORT, origViewportSize);

	// Enable rendering to framebuffer with voxelRes resolution
	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, param.voxelRes);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, param.voxelRes);

	glViewport(0, 0, param.voxelRes, param.voxelRes);
	glDisable(GL_CULL_FACE);

	// Clear the last voxelization data
	glClearTexImage(voxel2DTex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glClearTexImage(voxelTex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	// Bind the textures used to hold the voxelization data
	glBindImageTexture(2, voxel2DTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(3, voxelTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(4, voxelTex, 1, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI); // First mip level, just used for writing the occupancy

	// Reset the sparse texture count
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DRAW_IND, drawIndBuffer);
	glClearBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, 0 * sizeof(DrawElementsIndirectCommand) + sizeof(GLuint), sizeof(GLuint), GL_RED, GL_UNSIGNED_INT, NULL); // Clear data before since data is used when drawing

	for(auto model = models->begin(); model != models->end(); model++) {

		// Don't draw models without texture
		if(options.skipNoTexture && !(*model)->hasDiffuseTex()) {
			continue;
		}

		glUseProgram((*model)->GetVoxelProgram());
		glBindVertexArray((*model)->GetVoxelVAO());

		// Bind the color texture
		if((*model)->hasDiffuseTex()) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*model)->GetDiffuseID());
		} else {
			glm::vec3 diffColor = (*model)->GetDiffColor();
			glUniform3f(glGetUniformLocation((*model)->GetVoxelProgram(), "diffColor"), diffColor.r, diffColor.g, diffColor.b);
		}


		(*model)->Draw();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	// Restore the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(origViewportSize[0], origViewportSize[1], origViewportSize[2], origViewportSize[3]);
	
	
	// Optional read of the Indirect command buffer, to see the number of voxels actually used
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DRAW_IND, drawIndBuffer);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawElementsIndirectCommand), &drawIndCmd);

	// Optional read of the voxel positions
	//GLuint temp[300];
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPARSE_LIST, sparseListBuffer);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLint) * 300, temp);
	//for(size_t i = 0; i < 100; i++)
	//	std::cout << "First Element: " << temp[i * 3] << ", " << temp[i * 3 + 1] << ", " << temp[i * 3 + 2] << std::endl;

}

void Scene::Draw() {
	if(options.drawTextures) {
		glUseProgram(shaders->singleTriangle);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_ARRAY, voxel2DTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_3D, voxelTex);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	} else {

		if(options.drawModels) {
			for(auto model = models->begin(); model != models->end(); model++) {

				// Don't draw models without texture
				if(options.skipNoTexture && !(*model)->hasDiffuseTex()) {
					continue;
				}

				glUseProgram((*model)->GetDrawProgram());
				glBindVertexArray((*model)->GetDrawVAO());

				glEnable(GL_CULL_FACE);

				// Bind the color texture
				if((*model)->hasDiffuseTex()) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, (*model)->GetDiffuseID());
				} else {
					glm::vec3 diffColor = (*model)->GetDiffColor();
					glUniform3f(glGetUniformLocation((*model)->GetDrawProgram(), "diffColor"), diffColor.r, diffColor.g, diffColor.b);
				}

				// Bind the masking texture
				if((*model)->hasMaskTex()) {
					glDisable(GL_CULL_FACE);

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, (*model)->GetMaskID());
				}

				(*model)->Draw();
			}
		}

		if(options.drawVoxels) {
			glEnable(GL_CULL_FACE);

			glUseProgram(voxelModel->GetDrawProgram());
			glBindVertexArray(voxelModel->GetDrawVAO());

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_3D, voxelTex);

			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0L);
		}
	}
}

void TW_CALL Scene::SetSceneCB(const void* value, void* clientData) {
	Scene* obj = static_cast<Scene*>(clientData);
	SceneParam input = *static_cast<const SceneParam*>(value);

	obj->param.voxelDraw = input.voxelDraw;
	obj->param.voxelLayer = input.voxelLayer;
	obj->param.view = input.view;
	obj->param.mipLevel = input.mipLevel;

	// Update texture if new size
	if(input.voxelRes != obj->param.voxelRes) {
		obj->param.voxelRes = input.voxelRes;
		obj->param.numMipLevels = (GLuint)log2(input.voxelRes) + 1;

		obj->GenViewTexture(&obj->voxel2DTex);
		obj->GenVoxelTexture(&obj->voxelTex);

		obj->ResizeBuffer();

		obj->UploadParams();

		obj->Voxelize();
	} else {
		obj->UploadParams();
	}
}

void TW_CALL Scene::GetSceneCB(void* value, void* clientData) {
	*static_cast<SceneParam*>(value) = static_cast<Scene*>(clientData)->param;
}

void TW_CALL Scene::SetSceneOptionsCB(const void* value, void* clientData) {
	Scene* obj = static_cast<Scene*>(clientData);
	SceneOptions input = *static_cast<const SceneOptions*>(value);

	if(obj->options.skipNoTexture != input.skipNoTexture) {
		obj->options = input;

		obj->Voxelize();
	} else {
		obj->options = input;
	}
}

void TW_CALL Scene::GetSceneOptionsCB(void* value, void* clientData) {
	*static_cast<SceneOptions*>(value) = static_cast<Scene*>(clientData)->options;
}

void TW_CALL Scene::SetDrawIndCB(const void* value, void* clientData) {
	Scene* obj = static_cast<Scene*>(clientData);
	obj->drawIndCmd = *static_cast<const DrawElementsIndirectCommand*>(value);
}

void TW_CALL Scene::GetDrawIndCB(void* value, void* clientData) {
	*static_cast<DrawElementsIndirectCommand*>(value) = static_cast<Scene*>(clientData)->drawIndCmd;
}