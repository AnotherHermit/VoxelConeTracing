///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahl�n - conwa099
//
///////////////////////////////////////

#ifndef PROGRAM_H
#define PROGRAM_H

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

#include "Camera.h"
#include "Scene.h"

#include "GL_utilities.h"

#include "AntTweakBar.h"

struct ProgramStruct {
	GLfloat currentT;
	GLfloat deltaT;
};

class Program {
private:
	SDL_Window* screen;
	SDL_GLContext glcontext;
	GLint winWidth, winHeight;

	bool isRunning;

	Timer time;
	GLfloat FPS;
	ProgramStruct param;
	GLuint programBuffer;

	bool useOrtho;
	bool drawVoxelOverlay;
	Camera* cam;
	TwBar* antBar;

	// Shaders
	ShaderList shaders;

	// Model
	GLuint sceneSelect;
	std::vector<Scene*> scenes;

	// Program params
	glm::vec3 cameraStartPos;
	GLfloat cameraFrustumFar;

	// Methods
	void UploadParams();

	Scene* GetCurrentScene() { return scenes[sceneSelect]; }

public:
	Program();

	int Execute();

	void timeUpdate();

	bool Init();

	void OnEvent(SDL_Event *Event);
	void OnKeypress(SDL_Event *Event);
	void OnMouseMove(SDL_Event *Event);
	void CheckKeyDowns();

	void Update();
	void Render();

	void Clean();
};

#endif // PROGRAM_H
