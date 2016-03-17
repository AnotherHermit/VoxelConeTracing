///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#include "OrthoCam.h"

#include "GL_utilities.h"

#include "gtc/matrix_transform.hpp"

#include <iostream>

OrthoCam::OrthoCam(GLint initVoxelRes) {
	zeroVec = glm::vec3(0.0f);
	xVec = glm::vec3(1.0f, 0.0f, 0.0f);
	yVec = glm::vec3(0.0f, 1.0f, 0.0f);
	zVec = glm::vec3(0.0f, 0.0f, 1.0f);
	
	voxelRes = initVoxelRes;

	//cameraTwMembers[0] = { "Cam Pos x", TW_TYPE_FLOAT, offsetof(CameraParam, position.x), " readonly=true group=Info " };
	//cameraTwMembers[1] = { "Cam Pos y", TW_TYPE_FLOAT, offsetof(CameraParam, position.y), " readonly=true group=Info " };
	//cameraTwMembers[2] = { "Cam Pos z", TW_TYPE_FLOAT, offsetof(CameraParam, position.z), " readonly=true group=Info " };
	//cameraTwStruct = TwDefineStruct("OrthoCam", cameraTwMembers, 3, sizeof(OrthoCamParam), NULL, NULL);
}

bool OrthoCam::Init() {
	glGenBuffers(1, &orthoCamBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, orthoCamBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(OrthoCamParam), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Set starting WTVmatrix
	param.WTVmatrix[0] = lookAt(zeroVec, -zVec, yVec);
	param.WTVmatrix[1] = lookAt(zeroVec, -xVec, yVec);
	param.WTVmatrix[2] = lookAt(zeroVec, -yVec, -zVec);
	param.VTPmatrix = glm::ortho(-1.001f, 1.001f, -1.001f, 1.001f, -1.001f, 1.001f);

	UploadParams();

	printError("OrthoCam init");
	return true;
}

void OrthoCam::UploadParams() {
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, orthoCamBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(OrthoCamParam), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
