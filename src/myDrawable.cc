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

Model::Model() {
}

bool Model::loadModels(const char* path) {
	// Load models
	std::string err;
	bool wasLoaded = tinyobj::LoadObj(shapes, materials, err, path, "resources/");
	if (!wasLoaded || !err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}
	std::cout << "Loaded following models: " << std::endl;
	for (size_t i = 0; i < shapes.size(); i++) {
		printf("Model (LoD %zd): %s\n", i, shapes[i].name.c_str());
	}
	return true;
}

void Model::addModel(int id) {
	ModelData* data = new ModelData();

	// Create buffers
	glGenVertexArrays(1, &data->drawVAO);
	glGenBuffers(3, data->drawBuffers);

	models.push_back(data);

	// Allocate enough memory for instanced drawing buffers
	glBindBuffer(GL_ARRAY_BUFFER, data->drawBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, data->drawBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->drawBuffers[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Set the GPU pointers for drawing 
	glUseProgram(program);
	glBindVertexArray(data->drawVAO);

	glBindBuffer(GL_ARRAY_BUFFER, data->drawBuffers[0]);
	GLuint vPos = glGetAttribLocation(program, "inPosition");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, data->drawBuffers[1]);
	GLuint vNorm = glGetAttribLocation(program, "inNormal");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->drawBuffers[2]);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool Model::Init(const char* path) {
	if (!loadModels(path)) return false;

	// Load shaders
	program = loadShaders("src/shaders/simpleModel.vert", "src/shaders/simpleModel.frag");

	// Combine into one big model
	for(int i = 0; i < shapes.size(); i++) {
		addModel(i);
	}

	printError("init Model");
	return true;
}

void Model::Draw() {
	glUseProgram(program);

	for(size_t i = 0; i < models.size(); i++){
		glBindVertexArray(models[i]->drawVAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)shapes[i].mesh.indices.size(), GL_UNSIGNED_INT, 0L);
		glBindVertexArray(0);
	}


	printError("Draw Models");
}