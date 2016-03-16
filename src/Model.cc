///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#include "Model.h"

#include "GL_utilities.h"

#include <iostream>

void Model::SetMaterial(TextureData* textureData) {
	bumpID = textureData->bumpID;
	diffuseID = textureData->diffuseID;
	maskID = textureData->maskID;
}

void Model::SetProgram(GLuint initProgram) {
	program = initProgram;
}

void Model::SetStandardData(size_t numVertices, GLfloat* verticeData,
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

	printError("Set standard model data");
}

void Model::SetTextureData(size_t numTexCoords, GLfloat* texCoordData) {
	glGenBuffers(1, &texbufferID);

	// Allocate enough memory for instanced drawing buffers
	glBindBuffer(GL_ARRAY_BUFFER, texbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numTexCoords, texCoordData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(program);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, texbufferID);
	GLuint vTex = glGetAttribLocation(program, "inTexCoords");
	glEnableVertexAttribArray(vTex);
	glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	printError("Set texture data");
}

bool Model::hasBumpTex() {
	return bumpID != -1;
}

bool Model::hasDiffuseTex() {
	return diffuseID != -1;
}

bool Model::hasMaskTex() {
	return maskID != -1;
}

void Model::Draw() {
	glUseProgram(program);
	glBindVertexArray(vao);

	// Bind the color texture
	if(hasDiffuseTex()) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseID);

		glEnable(GL_CULL_FACE);
	}

	// Bind the masking texture
	if(hasMaskTex()) {
		glDisable(GL_CULL_FACE);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, maskID);
	}

	// Draw
	glDrawElements(GL_TRIANGLES, (GLsizei)nIndices, GL_UNSIGNED_INT, 0L);
	glBindVertexArray(0);

	printError("Draw Model");
}