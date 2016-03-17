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
	diffuseID = textureData->diffuseID;
	maskID = textureData->maskID;
	diffColor = textureData->diffColor;
}

void Model::SetProgram(GLuint initProgram, GLuint initVoxelProgram) {
	drawProgram = initProgram;
	voxelProgram = initVoxelProgram;
	useProgram = drawProgram;
}

void Model::SetStandardData(size_t numVertices, GLfloat* verticeData,
							size_t numNormals, GLfloat* normalData,
							size_t numIndices, GLuint* indexData) {

	nIndices = numIndices;

	// Create buffers
	glGenVertexArrays(1, &drawVAO);
	glGenVertexArrays(1, &voxelVAO);
	useVAO = drawVAO;

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
	glUseProgram(drawProgram);
	glBindVertexArray(drawVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
	GLuint vPos = glGetAttribLocation(drawProgram, "inPosition");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbufferID);
	GLuint vNorm = glGetAttribLocation(drawProgram, "inNormal");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferID);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Set the GPU pointers for voxelization
	glUseProgram(voxelProgram);
	glBindVertexArray(voxelVAO);

	vNorm = glGetAttribLocation(voxelProgram, "inNormal");
	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferID);
	vPos = glGetAttribLocation(voxelProgram, "inPosition");
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
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

	// Set the data pointer for the draw program
	glUseProgram(drawProgram);
	glBindVertexArray(drawVAO);

	glBindBuffer(GL_ARRAY_BUFFER, texbufferID);

	GLuint vTex = glGetAttribLocation(drawProgram, "inTexCoords");
	glEnableVertexAttribArray(vTex);
	glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	// Set the data pointer for the voxel program
	glUseProgram(voxelProgram);
	glBindVertexArray(voxelVAO);

	vTex = glGetAttribLocation(voxelProgram, "inTexCoords");
	glEnableVertexAttribArray(vTex);
	glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	printError("Set texture data");
}

bool Model::hasDiffuseTex() {
	return diffuseID != -1;
}

bool Model::hasMaskTex() {
	return maskID != -1;
}

void Model::Draw() {
	glUseProgram(useProgram);
	glBindVertexArray(useVAO);
	
	glEnable(GL_CULL_FACE);

	// Bind the color texture
	if(hasDiffuseTex()) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseID);
	} else {
		glUniform3f(glGetUniformLocation(useProgram, "diffColor"), diffColor.r, diffColor.g, diffColor.b);
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

void Model::SetVoxelDraw(bool enable) {
	if(enable) {
		useProgram = voxelProgram;
		useVAO = voxelVAO;
	} else {
		useProgram = drawProgram;
		useVAO = drawVAO;
	}
}