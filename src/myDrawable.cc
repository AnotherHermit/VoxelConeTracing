///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

#include "myDrawable.h"

#include "GL_utilities.h"

#include "glm.hpp"
#include "gtx/transform.hpp"

#include <iostream>

Model::Model(GLuint initProgram) {
	program = initProgram;


}

bool Model::Init(size_t numVertices, GLfloat* verticeData,
			size_t numNormals, GLfloat* normalData,
			size_t numIndices, GLuint* indexData) {

	nIndices = numIndices;

	// Create buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vertexbufferID);
	glGenBuffers(1, &normalbufferID);
	glGenBuffers(1, &indexbufferID);

	// Allocate enough memory for instanced drawing buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices, verticeData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numNormals, normalData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * numIndices, indexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Set the GPU pointers for drawing 
	glUseProgram(program);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
	GLuint vPos = glGetAttribLocation(program, "inPosition");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbufferID);
	GLuint vNorm = glGetAttribLocation(program, "inNormal");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferID);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void Model::Draw() {
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)nIndices, GL_UNSIGNED_INT, 0L);
	glBindVertexArray(0);
}

TextureModel::TextureModel(GLuint initProgram)
	: Model(initProgram) {}

bool TextureModel::Init(size_t numVertices, GLfloat* verticeData,
				   size_t numNormals, GLfloat* normalData,
				   size_t numIndices, GLuint* indexData) {
	return Model::Init(numVertices, verticeData, numNormals, normalData, numIndices, indexData);
}

void TextureModel::Draw() {
	Model::Draw();
}

bool ModelLoader::loadModels(const char* path) {
	// Load models
	std::string err;
	bool wasLoaded = tinyobj::LoadObj(shapes, materials, err, path, "resources/");
	if (!wasLoaded || !err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	/*
	std::cout << "Loaded following models: " << std::endl;
	for (size_t i = 0; i < shapes.size(); i++) {
		printf("Model %zd: %d\n", i, shapes[i].mesh.material_ids[0]);
	}
	*/
	/*
	std::cout << "Loaded following materials: " << std::endl;
	for(size_t i = 0; i < materials.size(); i++) {
		printf("Material %zd diffuse: %s\n", i, materials[i].diffuse_texname.c_str());
	}
	*/
	return true;
}

void ModelLoader::addModel(int id) {
	Model* model;

	if(materials[shapes[id].mesh.material_ids[0]].diffuse_texname.empty()) {
		model = new Model(simpleProgram);
		model->Init(shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(),
					shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(),
					shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data());
	} else if(!materials[shapes[id].mesh.material_ids[0]].bump_texname.empty()) {
		model = new TextureModel(bumpProgram);
		model->Init(shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(),
					shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(),
					shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data());
	} else if(!materials[shapes[id].mesh.material_ids[0]].diffuse_texname.empty()) {
		model = new TextureModel(textureProgram);
		model->Init(shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(),
					shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(),
					shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data());
	} else {
		model = new Model(errorProgram);
		model->Init(shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(),
					shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(),
					shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data());
	}

	models.push_back(model);
}

bool ModelLoader::Init(const char* path) {
	if (!loadModels(path)) return false;

	// Load shaders
	GLint err;
	simpleProgram = loadShaders("src/shaders/simpleModel.vert", "src/shaders/simpleModel.frag");
	glGetProgramiv(simpleProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	textureProgram = loadShaders("src/shaders/textureModel.vert", "src/shaders/textureModel.frag");
	glGetProgramiv(textureProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	bumpProgram = loadShaders("src/shaders/bumpModel.vert", "src/shaders/bumpModel.frag");
	glGetProgramiv(bumpProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;
	
	errorProgram = loadShaders("src/shaders/errorModel.vert", "src/shaders/errorModel.frag");
	glGetProgramiv(errorProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	// Read all models
	for(int i = 0; i < shapes.size(); i++) {
		addModel(i);
	}

	printError("init Model");
	return true;
}

void ModelLoader::Draw() {
	for(size_t i = 0; i < models.size(); i++){
		models[i]->Draw();
	}
	printError("Draw Models");
}