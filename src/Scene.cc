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
TwType* Scene::compIndTwStruct = nullptr;

Scene::Scene() {
	options.skipNoTexture = false;
	options.drawModels = true;
	options.drawVoxels = false;
	options.drawTextures = false;

	param.voxelRes = 256;
	param.voxelLayer = 0;
	param.voxelDraw = 0;
	param.view = 0;
	param.numMipLevels = (GLuint)log2(param.voxelRes);
	param.mipLevel = 0;

	maxVertex = nullptr;
	minVertex = nullptr;

	models = new std::vector<Model*>();
	voxelModel = new Model();

	voxel2DTex = 0;
	voxelTex = 0;
}

bool Scene::Init(const char* path, ShaderList* initShaders) {

	shaders = initShaders;

	InitBuffers();
	//InitMipMap();

	if(!InitAntBar()) return false;
	if(!SetupScene(path)) return false;
	if(!InitVoxel()) return false;
	
	SetupDrawInd();
	SetupCompInd();
	SetupTextures();

	UpdateBuffers();

	return true;
}

void Scene::InitBuffers() {
	// Init the framebuffer for drawing
	glGenFramebuffers(1, &voxelFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);

	// Set non-constant uniforms for all programs
	glGenBuffers(1, &sceneBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, SCENE, sceneBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneParam), NULL, GL_STREAM_DRAW);

	// Set up the sparse active voxel buffer
	glGenBuffers(1, &sparseListBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPARSE_LIST, sparseListBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (sizeof(GLuint) * MAX_SPARSE_BUFFER_SIZE), NULL, GL_STREAM_DRAW);
}

bool Scene::InitAntBar() {
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

		drawIndTwMembers[0] = { "InstanceCount", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, instanceCount), " readonly=true " };
		drawIndTwMembers[1] = { "BaseInstance", TW_TYPE_UINT32, offsetof(DrawElementsIndirectCommand, baseInstance), " readonly=true " };
		drawIndTwStruct = new TwType;
		*drawIndTwStruct = TwDefineStruct("DrawIndStruct", drawIndTwMembers, 2, sizeof(DrawElementsIndirectCommand), NULL, NULL);

		compIndTwMembers[0] = { "WorkGroupSizeX", TW_TYPE_UINT32, offsetof(ComputeIndirectCommand, workGroupSizeX), " readonly=true " };
		compIndTwStruct = new TwType;
		*compIndTwStruct = TwDefineStruct("CompIndStruct", compIndTwMembers, 1, sizeof(ComputeIndirectCommand), NULL, NULL);

		// Check if AntTweak Setup is ok
		if(TwGetLastError() != NULL) return false;

		isInitialized = true;
	}

	return true;
}

bool Scene::SetupScene(const char* path) {
	ModelLoader modelLoader;
	if(!modelLoader.LoadScene(path, models, shaders, &maxVertex, &minVertex)) {
		std::cout << "Failed to load scene: " << path << std::endl;
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

	return true;
}

bool Scene::InitVoxel() {
	// Load a model for drawing the voxel
	ModelLoader modelLoader;
	if(!modelLoader.LoadModel("resources/voxelLarge.obj", voxelModel, shaders->voxel)) {
		std::cout << "Failed to load voxel model" << std::endl;
		return false;
	}

	voxelModel->SetPositionData(sparseListBuffer);

	return true;
}

void Scene::InitMipMap() {
	glGenVertexArrays(1, &mipmapVAO);

	// Allocate enough memory for instanced drawing buffers
	// Set the GPU pointers for drawing 
	glUseProgram(shaders->mipmap);
	glBindVertexArray(mipmapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sparseListBuffer);
	GLuint vPos = glGetAttribLocation(shaders->mipmap, "inVoxelPos");
	glEnableVertexAttribArray(vPos);
	glVertexAttribIPointer(vPos, 1, GL_UNSIGNED_INT, 0, 0);

	glBindVertexArray(0);
}

void Scene::SetupDrawInd() {
	// Initialize the indirect drawing buffer
	for(int i = MAX_MIP_MAP_LEVELS, j = 0; i >= 0; i--, j++) {
		drawIndCmd[i].vertexCount = (GLuint)voxelModel->GetNumIndices();
		drawIndCmd[i].instanceCount = 0;
		drawIndCmd[i].firstVertex = 0;
		drawIndCmd[i].baseVertex = 0;

		// TODO: Change the base instance to fit all lower than initial level completely
		if(i == MAX_MIP_MAP_LEVELS) {
			drawIndCmd[i].baseInstance = MAX_SPARSE_BUFFER_SIZE - 1;
		} else if(i != 0) {
			drawIndCmd[i].baseInstance = drawIndCmd[i+1].baseInstance - (1 << (3 * j));
		} else {
			drawIndCmd[i].baseInstance = 0;
		}
	}

	// Draw Indirect Command buffer for drawing voxels
	glGenBuffers(1, &drawIndBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DRAW_IND, drawIndBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(drawIndCmd), drawIndCmd, GL_STREAM_DRAW);
}

void Scene::SetupCompInd() {
	// Initialize the indirect compute buffer
	for(size_t i = 0; i < MAX_MIP_MAP_LEVELS; i++) {
		compIndCmd[i].workGroupSizeX = 0;
		compIndCmd[i].workGroupSizeY = 1;
		compIndCmd[i].workGroupSizeZ = 1;
	}

	// Draw Indirect Command buffer for drawing voxels
	glGenBuffers(1, &compIndBuffer);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, compIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_IND, compIndBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(compIndCmd), compIndCmd, GL_STREAM_DRAW);
}

void Scene::SetupTextures() {
	
	// Generate textures for render to texture, only for debugging purposes
	if(voxel2DTex != 0) {
		glDeleteTextures(1, &voxel2DTex);
	}
	glGenTextures(1, &voxel2DTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, voxel2DTex);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, param.voxelRes, param.voxelRes, 3);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Create the 3D texture that contains the voxel data
	if(voxelTex != 0) {
		glDeleteTextures(1, &voxelTex);
	}
	glGenTextures(1, &voxelTex);
	glBindTexture(GL_TEXTURE_3D, voxelTex);
	glTexStorage3D(GL_TEXTURE_3D, param.numMipLevels, GL_R32UI, param.voxelRes, param.voxelRes, param.voxelRes);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
}

void Scene::UpdateBuffers() {
	// Upload new params to GPU
	glBindBufferBase(GL_UNIFORM_BUFFER, SCENE, sceneBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(SceneParam), &param);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndBuffer);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, compIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, DRAW_IND, drawIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, COMPUTE_IND, compIndBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPARSE_LIST, sparseListBuffer);
}

void Scene::Voxelize() {
	// Setup framebuffer for rendering offscreen
	GLint origViewportSize[4];
	glGetIntegerv(GL_VIEWPORT, origViewportSize);

	// Enable rendering to framebuffer with voxelRes resolution
	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, param.voxelRes);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, param.voxelRes);
	glViewport(0, 0, param.voxelRes, param.voxelRes);


	// Clear the last voxelization data
	glClearTexImage(voxel2DTex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glClearTexImage(voxelTex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	// Reset the sparse voxel count
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawIndBuffer);
	for(size_t i = 0; i < MAX_MIP_MAP_LEVELS; i++) {
		glClearBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, i * sizeof(DrawElementsIndirectCommand) + sizeof(GLuint), sizeof(GLuint), GL_RED, GL_UNSIGNED_INT, NULL); // Clear data before since data is used when drawing
	}

	// Reset the sparse voxel count for compute shader
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, compIndBuffer);
	for(size_t i = 0; i < MAX_MIP_MAP_LEVELS; i++) {
		glClearBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, i * sizeof(ComputeIndirectCommand), sizeof(GLuint), GL_RED, GL_UNSIGNED_INT, NULL); // Clear data before since data is used when drawing
	}

	// Bind the textures used to hold the voxelization data
	glBindImageTexture(2, voxel2DTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(3, voxelTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	// All faces must be rendered
	glDisable(GL_CULL_FACE);

	for(auto model = models->begin(); model != models->end(); model++) {
		// Don't draw models without texture if set to skip
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
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Restore the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(origViewportSize[0], origViewportSize[1], origViewportSize[2], origViewportSize[3]);

	MipMap();
}

void Scene::MipMap() {
	for(GLuint level = 0; level < param.numMipLevels-1; level++) {
		glUseProgram(shaders->mipmap);

		glBindImageTexture(3, voxelTex, level, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
		glBindImageTexture(4, voxelTex, level + 1, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI); 

		glUniform1ui(glGetUniformLocation(shaders->mipmap, "currentLevel"), level);

		glDispatchComputeIndirect(NULL);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	}
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
}

void Scene::DrawTextures() {
	glUseProgram(shaders->singleTriangle);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_ARRAY, voxel2DTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, voxelTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Scene::DrawScene() {
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

void Scene::DrawVoxels() {
	glEnable(GL_CULL_FACE);

	glUseProgram(voxelModel->GetDrawProgram());
	glBindVertexArray(voxelModel->GetDrawVAO());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, voxelTex);

	glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)(sizeof(DrawElementsIndirectCommand) * param.mipLevel));
}

void Scene::Draw() {
	if(options.drawTextures) DrawTextures();
	else {
		if(options.drawModels) DrawScene();
		if(options.drawVoxels) DrawVoxels();
	}
}

void Scene::PrintDrawIndCmd() {
#ifdef DEBUG
	// Print the Indirect command buffer, to see the number of voxels actually used
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawIndBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(drawIndCmd), &drawIndCmd);
#endif // DEBUG
}

void Scene::PrintCompIndCmd() {
#ifdef DEBUG
	// Print the Indirect command buffer, to see the number of voxels actually used
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, compIndBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(compIndCmd), &compIndCmd);
#endif // DEBUG
}

void Scene::PrintBuffer(GLuint bufferID, GLuint elements) {
	// Optional read of the voxel positions
	GLuint* temp = new GLuint[elements];
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, (sizeof(GLuint) * elements), temp);
	for(size_t i = 0; i < elements; i++)
		std::cout << "Element " << i << ": " << temp[i] << std::endl;
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
		obj->param.numMipLevels = (GLuint)log2(input.voxelRes);

		obj->SetupDrawInd();
		obj->SetupTextures();
		obj->UpdateBuffers();

		obj->Voxelize();
	} else {
		obj->UpdateBuffers();
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
	obj->drawIndCmd[obj->param.mipLevel] = *static_cast<const DrawElementsIndirectCommand*>(value);
}

void TW_CALL Scene::GetDrawIndCB(void* value, void* clientData) {
	Scene* obj = static_cast<Scene*>(clientData);
	obj->PrintDrawIndCmd();
	*static_cast<DrawElementsIndirectCommand*>(value) = obj->drawIndCmd[obj->param.mipLevel];
}

void TW_CALL Scene::SetCompIndCB(const void* value, void* clientData) {
	Scene* obj = static_cast<Scene*>(clientData);
	obj->compIndCmd[obj->param.mipLevel] = *static_cast<const ComputeIndirectCommand*>(value);
}

void TW_CALL Scene::GetCompIndCB(void* value, void* clientData) {
	Scene* obj = static_cast<Scene*>(clientData);
	obj->PrintCompIndCmd();
	*static_cast<ComputeIndirectCommand*>(value) = obj->compIndCmd[obj->param.mipLevel];
}