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
GLuint CompileComputeShader(const char* compFileName);

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

// ===== Helpful data structs =====

// All available shaders
struct ShaderList {
	GLuint drawScene;
	GLuint drawData;
	GLuint voxelize;
	GLuint singleTriangle;
	GLuint voxel;
	GLuint mipmap;
	GLuint shadowMap;
	GLuint lightInjection;
};

// Draw Indirect command struct
struct DrawElementsIndirectCommand {
	GLuint vertexCount;
	GLuint instanceCount;
	GLuint firstVertex;
	GLuint baseVertex;
	GLuint baseInstance;
};

// Compute Indirect Command struct
struct ComputeIndirectCommand {
	GLuint workGroupSizeX;
	GLuint workGroupSizeY;
	GLuint workGroupSizeZ;
};


// ===== Helpful Enums for Buffer Bindings =====

enum UNIFORM_BINDING {
	CAMERA,
	SCENE,
	PROGRAM,
};

enum SHADER_STORAGE_BINDING {
	DRAW_IND,
	COMPUTE_IND,
	SPARSE_LIST
};

enum VERTEX_LOCATION_BINDING {
	VERT_POS,
	VERT_NORMAL,
	VERT_TEX_COORD,
	DATA_POS
};

enum UNIFORM_LOCATION_BINDING {
	DIFF_COLOR, //		0
	DIFF_UNIT, //		1
	MASK_UNIT, //		2
	VOXEL_TEXTURE, //	3
	VOXEL_DATA, //		4
	VOXEL_DATA_NEXT, // 5
	SHADOW_UNIT, //		6
	CURRENT_LEVEL, //	7
	SCENE_UNIT, //		8
	SCENE_DEPTH, //		9
};

enum SUBROUTINE_INDICES {
	CONSTANT,
	TEXTURE,
	MASK
};

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



#endif
