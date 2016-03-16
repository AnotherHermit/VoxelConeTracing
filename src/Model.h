///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef MODEL_H
#define MODEL_H

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

// ===== Texture Struct =====

struct TextureData {
	GLuint diffuseID;
	GLuint bumpID;
	GLuint maskID;
	glm::vec3 diffColor;
};

// ===== Model Class =====

class Model {
public:
	Model() {}

	void SetMaterial(TextureData* textureData);
	void SetProgram(GLuint initProgram);

	void SetStandardData(size_t numVertices, GLfloat* verticeData,
			  size_t numNormals, GLfloat* normalData,
			  size_t numIndices, GLuint* indexData);

	// Set standard data must be used first since it creates the vao
	// TODO: remove the ordering dependecy
	void SetTextureData(size_t numTexCoords, GLfloat* texCoordData);

	bool hasDiffuseTex();
	bool hasBumpTex();
	bool hasMaskTex();

	void Draw();

private:
	GLuint program;
	GLuint vao;
	GLuint vertexbufferID;
	GLuint normalbufferID;
	GLuint indexbufferID;
	size_t nIndices;

	// Only used by textured models
	GLuint diffuseID;
	GLuint bumpID;
	GLuint maskID;
	GLuint texbufferID;

	//
	glm::vec3 diffColor;
};


#endif // MODEL_H
