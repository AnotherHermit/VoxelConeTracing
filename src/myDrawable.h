///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef myDrawable_H
#define myDrawable_H

#include "tiny_obj_loader.h"

#include "Camera.h"

#include "GL_utilities.h"

#include "glm.hpp"

// ===== Model Class =====

class Model {
public:
	Model(GLuint initProgram);

	virtual void Draw();
	bool Init(size_t numVertices, GLfloat* verticeData,
			  size_t numNormals, GLfloat* normalData,
			  size_t numIndices, GLuint* indexData);

private:
	GLuint program;
	GLuint vao;
	GLuint vertexbufferID;
	GLuint normalbufferID;
	GLuint indexbufferID;
	size_t nIndices;
};

class TextureModel : public Model {
public:
	TextureModel(GLuint initProgram);

	void Draw();
	bool Init(size_t numVertices, GLfloat* verticeData,
			   size_t numNormals, GLfloat* normalData,
			   size_t numIndices, GLuint* indexData);

private:
	GLuint textureID;
};

class BumpModel : public TextureModel {
public:
	BumpModel(GLuint initProgram);

	void Draw();
	bool Init();

private:
	GLuint bumpID;
};

// ===== ModelLoader class =====

class ModelLoader {
protected:
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<Model*> models;

	GLuint simpleProgram, textureProgram, bumpProgram, errorProgram;

	void addModel(int id);

	bool loadModels(const char* path);

public:
	ModelLoader() {};

	bool Init(const char* path);
	void Draw();
};

#endif // myDrawable_H
