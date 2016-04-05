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

Scene::Scene() {
	options.skipNoTexture = false;
	options.drawModels = true;
	options.drawVoxels = false;
	options.drawTextures = false;

	param.voxelRes = 256;
	param.voxelLayer = 0;
	param.voxelDraw = 0;
	param.view = 0;
	param.mipLevel = 0;

	maxVertex = nullptr;
	minVertex = nullptr;

	models = new std::vector<Model*>();
	voxelModel = new Model();

	voxelTex = 0;
	xTex = 0;
	yTex = 0;
	zTex = 0;
}

bool Scene::Init(const char* path, ShaderList* initShaders) {

	shaders = initShaders;

	if(!InitializeAntBar()) return false;

	GenViewTexture(&xTex);
	GenViewTexture(&yTex);
	GenViewTexture(&zTex);

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
	glUniform1i(glGetUniformLocation(shaders->voxelize, "xView"), 0);
	glUniform1i(glGetUniformLocation(shaders->voxelize, "yView"), 1);
	glUniform1i(glGetUniformLocation(shaders->voxelize, "zView"), 2);
	glUniform1i(glGetUniformLocation(shaders->voxelize, "voxelData"), 3);

	// Set constant uniforms for voxel programs
	glUseProgram(shaders->voxelizeTexture);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "xView"), 0);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "yView"), 1);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "zView"), 2);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "voxelData"), 3);
	glUniform1i(glGetUniformLocation(shaders->voxelizeTexture, "diffuseUnit"), 4);

	// Set constant uniforms for simple triangle drawing
	glUseProgram(shaders->singleTriangle);
	glUniform1i(glGetUniformLocation(shaders->singleTriangle, "voxelData"), 3);

	// Set constant uniforms for drawing the voxel overlay
	glUseProgram(shaders->voxel);
	glUniform1i(glGetUniformLocation(shaders->voxel, "voxelData"), 3);

	// Set non-constant uniforms for all programs
	glGenBuffers(1, &sceneBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, sceneBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneParam), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

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

	UploadParams();

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
		sceneTwMembers[4] = { "MipLevel", TW_TYPE_UINT32, offsetof(SceneParam, mipLevel), " min=0 max=9  " };
		sceneTwStruct = new TwType;
		*sceneTwStruct = TwDefineStruct("SceneGPUStruct", sceneTwMembers, 5, sizeof(SceneParam), NULL, NULL);

		sceneOptionTwMembers[0] = { "SkipNoTexture", TW_TYPE_BOOL8, offsetof(SceneOptions, skipNoTexture), "  " };
		sceneOptionTwMembers[1] = { "DrawVoxels", TW_TYPE_BOOL8, offsetof(SceneOptions, drawVoxels), "  " };
		sceneOptionTwMembers[2] = { "DrawModels", TW_TYPE_BOOL8, offsetof(SceneOptions, drawModels), "  " };
		sceneOptionTwMembers[3] = { "DrawVoxelTextures", TW_TYPE_BOOL8, offsetof(SceneOptions, drawTextures), "  " };
		sceneOptionsTwStruct = new TwType;
		*sceneOptionsTwStruct = TwDefineStruct("SceneOptionsStruct", sceneOptionTwMembers, 4, sizeof(SceneOptions), NULL, NULL);

		// Check if AntTweak Setup is ok
		if(TwGetLastError() != NULL) return false;

		isInitialized = true;
	}

	return true;
}

// Generate textures for render to texture
void Scene::GenViewTexture(GLuint* viewID) {
	if(*viewID == 0) {
		glGenTextures(1, viewID);
	}
	glBindTexture(GL_TEXTURE_2D, *viewID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, param.voxelRes, param.voxelRes, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

// Create the 3D texture that contains the voxel data
void Scene::GenVoxelTexture(GLuint* texID) {
	if(*texID == 0) {
		glGenTextures(1, texID);
	}
	glBindTexture(GL_TEXTURE_3D, *texID);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, param.voxelRes, param.voxelRes, param.voxelRes, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Scene::UploadParams() {
	// Upload new params to GPU
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, sceneBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(SceneParam), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::Voxelize() {
	GLint origViewportSize[4];
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, sceneBuffer);
	glGetIntegerv(GL_VIEWPORT, origViewportSize);

	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, param.voxelRes);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, param.voxelRes);

	glViewport(0, 0, param.voxelRes, param.voxelRes);
	glDisable(GL_CULL_FACE);

	glClearTexImage(xTex, 0, GL_RGBA, GL_FLOAT, NULL);
	glClearTexImage(yTex, 0, GL_RGBA, GL_FLOAT, NULL);
	glClearTexImage(zTex, 0, GL_RGBA, GL_FLOAT, NULL);
	glClearTexImage(voxelTex, 0, GL_RGBA, GL_FLOAT, NULL);

	for(auto model = models->begin(); model != models->end(); model++) {

		// Don't draw models without texture
		if(options.skipNoTexture && !(*model)->hasDiffuseTex()) {
			continue;
		}

		glUseProgram((*model)->GetVoxelProgram());
		glBindVertexArray((*model)->GetVoxelVAO());

		// Bind the color texture
		if((*model)->hasDiffuseTex()) {
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, (*model)->GetDiffuseID());
		} else {
			glm::vec3 diffColor = (*model)->GetDiffColor();
			glUniform3f(glGetUniformLocation((*model)->GetVoxelProgram(), "diffColor"), diffColor.r, diffColor.g, diffColor.b);
		}

		glBindImageTexture(0, xTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(1, yTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(2, zTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(3, voxelTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		(*model)->Draw();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(origViewportSize[0], origViewportSize[1], origViewportSize[2], origViewportSize[3]);

	// Generate mipmaps
	glGenerateTextureMipmap(xTex);
	glGenerateTextureMipmap(yTex);
	glGenerateTextureMipmap(zTex);
	glBindTexture(GL_TEXTURE_3D, voxelTex);
	glGenerateMipmap(GL_TEXTURE_3D);
}

void Scene::Draw() {
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, sceneBuffer);
	if(options.drawTextures) {
		glUseProgram(shaders->singleTriangle);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, xTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, yTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, zTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_3D, voxelTex);
		glUniform1i(glGetUniformLocation(shaders->singleTriangle, "usedView"), param.view);

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

			glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)voxelModel->GetNumIndices(), GL_UNSIGNED_INT, 0L, param.voxelRes*param.voxelRes*(param.voxelRes - 1));
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

		obj->GenViewTexture(&obj->xTex);
		obj->GenViewTexture(&obj->yTex);
		obj->GenViewTexture(&obj->zTex);
		obj->GenVoxelTexture(&obj->voxelTex);

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