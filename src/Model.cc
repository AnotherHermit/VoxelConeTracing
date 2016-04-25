///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#include "Model.h"

#include "GL_utilities.h"

#include <iostream>

Model::Model() {
	subDrawID = 0;
	subVoxelizeID = 0;
	diffuseID = 0;
	maskID = 0;
	diffColor = glm::vec3(1.0f, 0.0f, 0.0f);

	vao = 0;
}

void Model::SetMaterial(TextureData* textureData) {
	subDrawID = textureData->subID;
	subVoxelizeID = (GLuint)(subDrawID != 0);
	diffuseID = textureData->diffuseID;
	maskID = textureData->maskID;
	diffColor = textureData->diffColor;
}

void Model::SetStandardData(size_t numVertices, GLfloat* verticeData,
							size_t numNormals, GLfloat* normalData,
							size_t numIndices, GLuint* indexData) {

	nIndices = numIndices;
	// Create buffers
	if(vao == 0) {
		glGenVertexArrays(1, &vao);
	}

	glGenBuffers(1, &vertexbufferID);
	glGenBuffers(1, &normalbufferID);
	glGenBuffers(1, &indexbufferID);

	// Allocate enough memory for instanced drawing buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices, verticeData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normalbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numNormals, normalData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat) * numIndices, indexData, GL_STATIC_DRAW);

	// Set the GPU pointers for drawing 
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
	glEnableVertexAttribArray(VERT_POS);
	glVertexAttribPointer(VERT_POS, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbufferID);
	glEnableVertexAttribArray(VERT_NORMAL);
	glVertexAttribPointer(VERT_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferID);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::SetTextureData(size_t numTexCoords, GLfloat* texCoordData) {
	if(vao == 0) {
		glGenVertexArrays(1, &vao);
	}
	glGenBuffers(1, &texbufferID);

	// Allocate enough memory for instanced drawing buffers
	glBindBuffer(GL_ARRAY_BUFFER, texbufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numTexCoords, texCoordData, GL_STATIC_DRAW);

	// Set the data pointer for the draw program
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, texbufferID);
	glEnableVertexAttribArray(VERT_TEX_COORD);
	glVertexAttribPointer(VERT_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void Model::SetPositionData(GLuint positionBufferID) {
	if(vao == 0) {
		glGenVertexArrays(1, &vao);
	}
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, positionBufferID);
	glEnableVertexAttribArray(DATA_POS);
	glVertexAttribIPointer(DATA_POS, 1, GL_UNSIGNED_INT, 0, 0);
	glVertexAttribDivisor(DATA_POS, 1);

	glBindVertexArray(0);
}

bool Model::hasDiffuseTex() {
	return diffuseID != 0;
}

bool Model::hasMaskTex() {
	return maskID != 0;
}

void Model::Voxelize() {
	glUniform3f(DIFF_COLOR, diffColor.r, diffColor.g, diffColor.b);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseID);

	glBindVertexArray(vao);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subVoxelizeID);

	glDrawElements(GL_TRIANGLES, (GLsizei)nIndices, GL_UNSIGNED_INT, 0L);
}

void Model::ShadowMap() {
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)nIndices, GL_UNSIGNED_INT, 0L);
}

void Model::Draw() {
	glUniform3f(DIFF_COLOR, diffColor.r, diffColor.g, diffColor.b);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseID);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, maskID);

	glBindVertexArray(vao);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subDrawID);

	// Disable cull faces for transparent models
	if(hasMaskTex()) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
	}

	glDrawElements(GL_TRIANGLES, (GLsizei)nIndices, GL_UNSIGNED_INT, 0L);
}
