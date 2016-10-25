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

//
//
// CAMERA
//
//

Camera::Camera() {
	isPaused = true;
	needUpdate = true;

	rspeed = 0.001f;
	yvec = glm::vec3(0.0f, 1.0f, 0.0f);

	cameraTwMembers[0] = { "Cam Pos x", TW_TYPE_FLOAT, offsetof(CameraParam, position.x), " readonly=true group=Info " };
	cameraTwMembers[1] = { "Cam Pos y", TW_TYPE_FLOAT, offsetof(CameraParam, position.y), " readonly=true group=Info " };
	cameraTwMembers[2] = { "Cam Pos z", TW_TYPE_FLOAT, offsetof(CameraParam, position.z), " readonly=true group=Info " };
	cameraTwStruct = TwDefineStruct("Camera", cameraTwMembers, 3, sizeof(CameraParam), NULL, NULL);
}

bool Camera::Init(GLfloat fovInit, GLint *screenWidth, GLint *screenHeight, GLfloat farInit) {
	winHeight = screenHeight;
	winWidth = screenWidth;
	frustumFar = farInit;
	fov = fovInit;

	Resize();
	glGenBuffers(1, &cameraBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA, cameraBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraParam), NULL, GL_STREAM_DRAW);

	// Set starting WTVmatrix
	Update();

	return true;
}

void Camera::Reset() {
	param.position = startPos;
}

void Camera::Resize() {
	if(*winWidth > 0 && *winHeight > 0) {
		param.VTPmatrix = glm::perspectiveFov(glm::radians(fov), (GLfloat)*winWidth, (GLfloat)*winHeight, 1.0f, frustumFar);
		needUpdate = true;
	}
}

void Camera::Update(GLfloat deltaT) {
	if(needUpdate) {
		UpdateParams(deltaT);
		UploadParams();
	}
	needUpdate = false;
}

void Camera::UploadParams() {
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA, cameraBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(CameraParam), &param);
}

//
//
// FPCAMERA
//
//

FPCamera::FPCamera() : Camera() {
	mspeed = 10.0f;
	phi = 2.0f * (float)M_PI / 2.0f;
	theta = 2.0f * (float)M_PI / 4.0f;
	moveVec = glm::vec3(0, 0, 0);
}

bool FPCamera::Init(glm::vec3 startpos, GLfloat fovInit, GLint *screenWidth, GLint *screenHeight, GLfloat farInit) {
	startPos = startpos;
	param.position = startPos;

	return Camera::Init(fovInit, screenWidth, screenHeight, farInit);
}

void FPCamera::UpdateParams(GLfloat deltaT) {
	// Update directions
	forward = glm::normalize(glm::vec3(-sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi)));
	right = glm::normalize(glm::cross(forward, yvec));
	up = glm::normalize(glm::cross(right, forward));

	// Update camera matrix
	param.position += moveVec * deltaT;
	lookp = param.position + forward;
	param.WTVmatrix = lookAt(param.position, lookp, yvec);
	moveVec = glm::vec3(0.0f);
}

void FPCamera::Reset() {
	Camera::Reset();

	phi = 7.0f * (float)M_PI / 4.0f;
	theta = (float)M_PI / 2.0f;
}

void FPCamera::MoveForward() { Move(forward); }

void FPCamera::MoveBackward() { Move(-forward); }

void FPCamera::MoveRight() { Move(right); }

void FPCamera::MoveLeft() { Move(-right); }

void FPCamera::MoveUp() { Move(up); }

void FPCamera::MoveDown() { Move(-up); }

void FPCamera::Move(glm::vec3 vec) {
	if(!isPaused) {
		moveVec += vec * mspeed;
		needUpdate = true;
	}
}

void FPCamera::Rotate(GLint dx, GLint dy) {
	Rotate((GLfloat)dx, (GLfloat)dy);
}

void FPCamera::Rotate(GLfloat dx, GLfloat dy) {
	if(!isPaused) {
		float eps = 0.001f;

		phi += rspeed * dx;
		theta += rspeed * dy;

		phi = (float)fmod(phi, 2.0f * (float)M_PI);
		theta = theta < (float)M_PI - eps ? (theta > eps ? theta : eps) :
			(float)M_PI - eps;
		needUpdate = true;
	}
}

//
//
// ORBITCAMERA
//
//

OrbitCamera::OrbitCamera() {
	startTarget = glm::vec3(0, 0, 0);
	target = startTarget;

	distance = 1.0f;
}

void OrbitCamera::UpdateParams(GLfloat deltaT) {
	param.position = glm::vec3(sin(polar) * cos(azimuth),
							   cos(polar),
							   sin(polar) * sin(azimuth));
	param.position *= distance;
	param.position += target;

	lookp = target;

	param.WTVmatrix = lookAt(param.position, lookp, yvec);
}

bool OrbitCamera::Init(glm::vec3 initTarget, GLfloat initDistance, GLfloat initPolar, GLfloat initAzimuth, GLfloat fovInit, GLint *screenWidth, GLint *screenHeight, GLfloat farInit) {
	startTarget = initTarget;
	target = startTarget;
	distance = initDistance;

	polar = initPolar;
	azimuth = initAzimuth;

	return Camera::Init(fovInit, screenWidth, screenHeight, farInit);
}

void OrbitCamera::Reset() {
	Camera::Reset();
}

void OrbitCamera::Rotate(GLint dx, GLint dy) {
	Rotate((GLfloat)dx, (GLfloat)dy);
}

void OrbitCamera::Rotate(GLfloat dx, GLfloat dy) {
	if(!isPaused) {
		float eps = 0.001f;

		azimuth -= rspeed * dx;
		polar += rspeed * dy;

		azimuth = (float)fmod(azimuth, 2.0f * (float)M_PI);
		polar = polar < (float)M_PI - eps ? (polar > eps ? polar : eps) : (float)M_PI - eps;

		needUpdate = true;
	}
}

void OrbitCamera::Zoom(GLfloat factor) {
	if(!isPaused) {
		distance /= factor;

		needUpdate = true;
	}
}
