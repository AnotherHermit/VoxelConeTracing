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

// ===== Texture Struct =====

struct TextureData {
	GLuint diffuseID;
	GLuint bumpID;
	GLuint maskID;
};

// ===== Model Class =====

class Model {
public:
	Model() {}

	virtual void Draw();

	void SetStandardData(size_t numVertices, GLfloat* verticeData,
			  size_t numNormals, GLfloat* normalData,
			  size_t numIndices, GLuint* indexData);

	void SetTextureData(size_t numTexCoords, GLfloat* texCoordData);

	void SetProgram(GLuint initProgram);
	void SetMaterial(TextureData* textureData);

	bool hasDiffuseTex();
	bool hasBumpTex();
	bool hasMaskTex();

private:
	GLuint program;
	GLuint vao;
	GLuint vertexbufferID;
	GLuint normalbufferID;
	GLuint indexbufferID;
	size_t nIndices;

	GLuint diffuseID;
	GLuint bumpID;
	GLuint maskID;
	GLuint texbufferID;
};

// ===== ModelLoader class =====

class ModelLoader {
protected:
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::vector<Model*> models;
	std::vector<TextureData*> textures;

	GLuint simpleProgram, textureProgram, bumpProgram, errorProgram, maskProgram;

	void AddModel(int id);

	bool LoadModels(const char* path);
	bool LoadTextures();

	GLuint LoadTexture(const char* path);

public:
	ModelLoader() {}

	bool Init(const char* path);
	void Draw();
};

#endif // myDrawable_H
