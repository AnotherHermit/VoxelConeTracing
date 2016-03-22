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

#include <iostream>

Scene::Scene() {
	skipNoTexture = false;
	param.view = 0;
	drawVoxels = false;
	maxVertex = nullptr;
	minVertex = nullptr;
	models = new std::vector<Model*>();
}

void Scene::SetSkipNoTexture(bool setValue) {
	skipNoTexture = setValue;
}

bool Scene::Init(const char* path, ShaderList* initShaders, GLuint initVoxelRes) {
	
	shaders = initShaders;
	voxelRes = initVoxelRes;

	GenViewTexture(&frontTex);
	GenViewTexture(&sideTex);
	GenViewTexture(&topTex);

	// Init the framebuffer for drawing
	glGenFramebuffers(1, &voxelFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, voxelRes);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, voxelRes);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set constant uniforms for the drawing programs
	glUseProgram(shaders->texture);
	glUniform1i(glGetUniformLocation(shaders->texture, "diffuseUnit"), 0);

	glUseProgram(shaders->mask);
	glUniform1i(glGetUniformLocation(shaders->mask, "diffuseUnit"), 0);
	glUniform1i(glGetUniformLocation(shaders->mask, "maskUnit"), 1);

	// Set constant uniforms for voxel programs
	glUseProgram(shaders->voxel);
	glUniform1i(glGetUniformLocation(shaders->voxel, "diffuseUnit"), 0);
	glUniform1i(glGetUniformLocation(shaders->voxel, "frontView"), 0);
	glUniform1i(glGetUniformLocation(shaders->voxel, "sideView"), 1);
	glUniform1i(glGetUniformLocation(shaders->voxel, "topView"), 2);

	// Set constant uniforms for voxelization program
	glUseProgram(shaders->singleTriangle);
	glUniform1i(glGetUniformLocation(shaders->singleTriangle, "usedView"), 0);

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

	glm::vec3 diffVector = (*maxVertex - *minVertex);
	centerVertex = diffVector / 2.0f + *minVertex;
	scale = glm::max(diffVector.x, glm::max(diffVector.y, diffVector.z));

	param.MTOmatrix = glm::scale(glm::vec3(2.0f / scale)) * glm::translate(-centerVertex);

	printError("init Scene");

	//=========================

	tempTex = modelLoader.LoadTexture("resources/textures/lion.tga");


	//=============



	return true;
}

void Scene::GenViewTexture(GLuint* viewID) {
	glGenTextures(1, viewID);
	glBindTexture(GL_TEXTURE_2D, *viewID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, voxelRes, voxelRes, 0, GL_RGBA, GL_FLOAT, NULL);
}

void Scene::Voxelize() {
	
	glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, voxelRes);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, voxelRes);

	glViewport(0, 0, voxelRes, voxelRes);
	glEnable(GL_CULL_FACE);

	int test = 7;

	glClearTexImage(frontTex, 0, GL_RGBA, GL_FLOAT, NULL);

	for(auto model = models->begin(); model != models->begin()+2; model++) {

		// Don't draw models without texture
		if(skipNoTexture && !(*model)->hasDiffuseTex()) {
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

		glBindImageTexture(0, frontTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		
		(*model)->Draw();

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 800);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	printError("Voxelize");

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaders->singleTriangle);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, frontTex);


	glUniform1i(glGetUniformLocation(shaders->singleTriangle, "usedView"), 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	printError("Draw Voxels");
}

void Scene::Draw() {
	// TODO: make this not update every draw call
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, sceneBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(SceneParam), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	if(drawVoxels) {
		Voxelize();
		return;
	}

	GLuint activeProgram;
	for(auto model = models->begin(); model != models->end(); model++) {
		
		// Don't draw models without texture
		if(skipNoTexture && !(*model)->hasDiffuseTex()) {
			continue;
		}
		
		if(drawVoxels) {
			activeProgram = (*model)->GetVoxelProgram();
			glBindVertexArray((*model)->GetVoxelVAO());

			glDisable(GL_CULL_FACE);
		} else {
			activeProgram = (*model)->GetDrawProgram();
			glBindVertexArray((*model)->GetDrawVAO());

			glEnable(GL_CULL_FACE);
		}
		glUseProgram(activeProgram);
		

		// Bind the color texture
		if((*model)->hasDiffuseTex()) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*model)->GetDiffuseID());
		} else {
			glm::vec3 diffColor = (*model)->GetDiffColor();
			glUniform3f(glGetUniformLocation(activeProgram, "diffColor"), diffColor.r, diffColor.g, diffColor.b);
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