///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#include "Camera.h"

#include "GL_utilities.h"

#include "gtc/matrix_transform.hpp"

#include <iostream>

Camera::Camera(glm::vec3 startpos, GLint * screenWidth, GLint * screenHeight, GLfloat farInit) {
	isPaused = true;
	needUpdate = true;

	param.position = startpos;
	yvec = glm::vec3(0.0f, 1.0f, 0.0f);

	mspeed = 10.0f;
	rspeed = 0.001f;
	phi = 2 * M_PI / 2.0f;
	theta = M_PI / 2.0f;

	frustumFar = farInit;

	winWidth = screenWidth;
	winHeight = screenHeight;

	SetFrustum();
	Update();

	cameraTwMembers[0] = { "Cam Pos x", TW_TYPE_FLOAT, offsetof(CameraParam, position.x), " readonly=true group=Info " };
	cameraTwMembers[1] = { "Cam Pos y", TW_TYPE_FLOAT, offsetof(CameraParam, position.y), " readonly=true group=Info " };
	cameraTwMembers[2] = { "Cam Pos z", TW_TYPE_FLOAT, offsetof(CameraParam, position.z), " readonly=true group=Info " };
	cameraTwStruct = TwDefineStruct("Camera", cameraTwMembers, 3, sizeof(CameraParam), NULL, NULL);
}

bool Camera::Init() {
	glGenBuffers(1, &cameraBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, cameraBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraParam), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Set starting WTVmatrix
	UploadParams();

	printError("camera init");
	return true;
}

void Camera::ResetCamera(glm::vec3 pos) {
	param.position = pos;

	phi = 7 * M_PI / 4;
	theta = M_PI / 2.0f;
}

void Camera::SetFrustum() {
	GLfloat ratio = (GLfloat)*winWidth / (GLfloat)*winHeight;
	GLfloat width = (ratio > 1.0f) ? 1.0f : ratio;
	GLfloat height = (ratio > 1.0f) ? 1.0f / ratio : 1.0f;

	param.VTPmatrix = glm::frustum(-width, width, -height, height, 1.0f, frustumFar);

	needUpdate = true;
}


void Camera::Update() {
	// Update directions
	heading = glm::normalize(glm::vec3(-sin(theta)*sin(phi), cos(theta), sin(theta)*cos(phi)));
	side = glm::normalize(glm::cross(heading, yvec));
	up = glm::normalize(glm::cross(side, heading));

	// Update camera matrix
	lookp = param.position + heading;
	param.WTVmatrix = lookAt(param.position, lookp, yvec);
}

void Camera::UploadParams() {
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, cameraBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(CameraParam), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::UpdateCamera() {
	if(needUpdate) {
		Update();
		UploadParams();
	}
	needUpdate = true; // Should be false later
}

void Camera::MoveForward(GLfloat deltaT) {
	if(!isPaused) {
		param.position += heading * mspeed * deltaT;
		needUpdate = true;
	}
}
void Camera::MoveRight(GLfloat deltaT) {
	if(!isPaused) {
		param.position += side * mspeed * deltaT;
		needUpdate = true;
	}
}
void Camera::MoveUp(GLfloat deltaT) {
	if(!isPaused) {
		param.position += up * mspeed * deltaT;
		needUpdate = true;
	}
}

void Camera::RotateCamera(GLint dx, GLint dy) {
	if(!isPaused) {
		float eps = 0.001f;

		phi += rspeed * dx;
		theta += rspeed * dy;

		phi = fmod(phi, 2.0f * M_PI);
		theta = theta < M_PI - eps ? (theta > eps ? theta : eps) : M_PI - eps;
		needUpdate = true;
	}
}