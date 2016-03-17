///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef ORTHOCAM_H
#define ORTHOCAM_H

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
struct OrthoCamParam {
	glm::mat4 WTVmatrix[3];	// Order is front, right side, top
	glm::mat4 VTPmatrix;
};

class OrthoCam {
private:
	glm::vec3 zeroVec, xVec, yVec, zVec;

	GLuint orthoCamBuffer;
	GLint voxelRes;

	OrthoCamParam param;

	//TwStructMember cameraTwMembers[3];
	//TwType cameraTwStruct;

public:
	OrthoCam(GLint initVoxelRes);
	bool Init();

	void UploadParams();

	//TwType GetCameraTwType() { return cameraTwStruct; }
	//CameraParam* GetCameraInfo() { return &param; }
};

#endif // ORTHOCAM_H
