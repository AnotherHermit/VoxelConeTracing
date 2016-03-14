#ifndef _GL_UTILITIES_
#define _GL_UTILITIES_

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

#include <chrono>

typedef std::chrono::high_resolution_clock myTime;
typedef std::chrono::duration<float> fsec;

char* readFile(const char *file);

void dumpInfo(void);
GLint printError(const char *functionName);
GLint printShaderInfoLog(GLuint obj, const char *fn);
GLint printProgramInfoLog(GLuint obj, const char *vfn, const char *ffn,
						 const char *gfn, const char *tcfn, const char *tefn);

GLuint loadShaders(const char *vertFileName, const char *fragFileName);
GLuint loadShadersG(const char *vertFileName, const char *fragFileName, const char *geomFileName);
GLuint loadShadersGT(const char *vertFileName, const char *fragFileName, const char *geomFileName,
					 const char *tcFileName, const char *teFileName);
GLint CompileComputeShader(GLuint* program, const char* path);

void initKeymapManager();
char keyIsDown(unsigned char c);
char keyPressed(unsigned char c);

// FBO support

//------------a structure for FBO information-------------------------------------
typedef struct {
	GLuint texid;
	GLuint fb;
	GLuint rb;
	GLuint depth;
	int width, height;
} FBOstruct;

FBOstruct *initFBO(int width, int height, int int_method);
FBOstruct *initFBO2(int width, int height, int int_method, int create_depthimage);
void useFBO(FBOstruct *out, FBOstruct *in1, FBOstruct *in2);
void updateScreenSizeForFBOHandler(int w, int h); // Temporary workaround to inform useFBO of screen size changes


// ===== Timers =====

class GLTimer {
private:
	GLuint queryID;
	GLuint64 time;

public:
	GLTimer();
	~GLTimer();

	void startTimer();
	void endTimer();
	GLfloat getTime();
	GLfloat getTimeMS();
};

class Timer {
public:
	Timer();

	void startTimer();
	void endTimer();
	GLfloat getTime();
	GLfloat getLapTime();
	GLfloat getTimeMS();

private:
	myTime::time_point startTime;
	GLfloat lapTime, time;
};

// Draw Indirect command struct
struct DrawElementsIndirectCommand {
	GLuint vertexCount;
	GLuint instanceCount;
	GLuint firstVertex;
	GLuint baseVertex;
	GLuint baseInstance;
};


#endif
