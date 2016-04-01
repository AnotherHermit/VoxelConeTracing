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

Scene::Scene() {
	skipNoTexture = false;
	drawVoxels = false;

	param.voxelRes = 128;
	param.voxelLayer = 0;
	param.voxelDraw = 0;
	param.view = 0;

	maxVertex = nullptr;
	minVertex = nullptr;

	models = new std::vector<Model*>();
	voxelModel = new Model();
}

void Scene::SetSkipNoTexture(bool setValue) {
	skipNoTexture = setValue;
}

bool Scene::Init(const char* path, ShaderList* initShaders) {
	
	shaders = initShaders;
	param.voxelRes = 128;
	param.voxelLayer = 0;
	param.voxelDraw = 0;
	param.view = 0;

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


	// Set constant uniforms for voxelization program
	glUseProgram(shaders->singleTriangle);
	glUniform1i(glGetUniformLocation(shaders->singleTriangle, "voxelData"), 3);


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
	if(!modelLoader.LoadModel("resources/voxel.obj", voxelModel, shaders->voxel)) {
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

	printError("init Scene");

	return true;
}

// Generate textures for render to texture
void Scene::GenViewTexture(GLuint* viewID) {
	glGenTextures(1, viewID);
	glBindTexture(GL_TEXTURE_2D, *viewID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, param.voxelRes, param.voxelRes, 0, GL_RGBA, GL_FLOAT, NULL);
}

// Create the 3D texture that contains the voxel data
void Scene::GenVoxelTexture(GLuint* texID) {
	glGenTextures(1, texID);
	glBindTexture(GL_TEXTURE_3D, *texID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, param.voxelRes, param.voxelRes, param.voxelRes, 0, GL_RGBA, GL_FLOAT, NULL);
}

void Scene::Voxelize() {
	// TODO: make this not update every draw call
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, sceneBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(SceneParam), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
		if(skipNoTexture && !(*model)->hasDiffuseTex()) {
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
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 800);

	printError("Voxelize");

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

	printError("Draw Voxels");
}

void Scene::Draw() {
	for(auto model = models->begin(); model != models->end(); model++) {
		
		// Don't draw models without texture
		if(skipNoTexture && !(*model)->hasDiffuseTex()) {
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
	printError("Draw Models");
}

void Scene::SetDrawVoxels(bool enable) {
	drawVoxels = enable;
}