///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef CAMERA_H
#define CAMERA_H

#ifdef __APPLE__
#	include <OpenGL/gl3.h>
#	include <SDL2/SDL.h>
#else
#	ifdef  __linux__
#		define GL_GLEXT_PROTOTYPES
#		include <GL/gl.h>
#		include <GL/glu.h>
#		include <GL/glx.h>
#		include <GL/glext.h>
#		include <SDL2/SDL.h>
#	else
#		include "glew.h"
#		include "Windows/sdl2/SDL.h"
#	endif
#endif


#include "glm.hpp"
#include "gtc/type_ptr.hpp"

#include "AntTweakBar.h"

#include <vector>

// Uniform struct, needs to be arranged in multiples of 4 * 4 B for tight packing on GPU
struct CameraParam {
	glm::mat4 WTVmatrix;	// 16 * 4 B ->   0 -  15
	glm::mat4 VTPmatrix;	// 16 * 4 B ->  16 -  31
	glm::vec3 position;		//  3 * 4 B ->  32 -  34
};

class Camera {
protected:
	glm::vec3 lookp, yvec;
	glm::vec3 startPos;
	GLfloat frustumFar, fov, rspeed;

	bool isPaused;
	bool needUpdate;

	GLuint cameraBuffer;
	GLint *winWidth, *winHeight;

	CameraParam param;

	TwStructMember cameraTwMembers[3];
	TwType cameraTwStruct;

	virtual void UpdateParams(GLfloat deltaT) = 0;

	void UploadParams();

public:
	Camera();

	bool Init(GLfloat fovInit, GLint *screenWidth, GLint	*screenHeight, GLfloat farInit);

	virtual void Reset();

	void Resize();

	void Update(GLfloat deltaT = 1.0f);

	void TogglePause() { isPaused = !isPaused; }

	GLfloat* GetRotSpeedPtr() { return &rspeed; }

	TwType GetCameraTwType() { return cameraTwStruct; }
	CameraParam* GetCameraInfo() { return &param; }
};

class FPCamera : public Camera {
private:
	glm::vec3 forward, right, up, moveVec;
	GLfloat mspeed, phi, theta;

	virtual void UpdateParams(GLfloat deltaT);

public:
	FPCamera();

	bool Init(glm::vec3 startpos, GLfloat fovInit, GLint *screenWidth, GLint *screenHeight, GLfloat farInit);

	void Move(glm::vec3 vec);

	void MoveForward();

	void MoveBackward();

	void MoveRight();

	void MoveLeft();

	void MoveUp();

	void MoveDown();

	void Rotate(GLint dx, GLint dy);

	void Rotate(GLfloat dx, GLfloat dy);

	virtual void Reset();

	GLfloat *GetSpeedPtr() { return &mspeed; }
};

class OrbitCamera : public Camera {
private:
	glm::vec3 target, startTarget;
	GLfloat distance, polar, azimuth;

	virtual void UpdateParams(GLfloat deltaT);

public:
	OrbitCamera();

	virtual bool Init(glm::vec3 initTarget, GLfloat initDistance, GLfloat initPolar, GLfloat initAzimuth, GLfloat fovInit, GLint *screenWidth, GLint *screenHeight, GLfloat farInit);

	virtual void Reset();

	void Rotate(GLint dx, GLint dy);

	void Rotate(GLfloat dx, GLfloat dy);

	void Zoom(GLfloat factor);
};

#endif // CAMERA_H
