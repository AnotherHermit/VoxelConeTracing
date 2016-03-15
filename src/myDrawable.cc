///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION // define this in only *one* .cc
#define STBI_ONLY_TGA // define this in only *one* .cc
#include "stb_image.h"

#include "myDrawable.h"

#include "GL_utilities.h"

#include "glm.hpp"
#include "gtx/transform.hpp"

#include <iostream>


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
}

void Model::SetMaterial(TextureData* textureData) {
	bumpID = textureData->bumpID;
	diffuseID = textureData->diffuseID;
}

void Model::SetProgram(GLuint initProgram) {
	program = initProgram;
}

bool Model::hasBumpTex() {
	return bumpID != -1;
}

bool Model::hasDiffuseTex() {
	return diffuseID != -1;
}

void Model::Draw() {
	if(!hasDiffuseTex()) {
		return;
	}

	glUseProgram(program);
	glBindVertexArray(vao);

	if(hasDiffuseTex()) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseID);
	}

	if(hasBumpTex()) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bumpID);
	}

	glDrawElements(GL_TRIANGLES, (GLsizei)nIndices, GL_UNSIGNED_INT, 0L);
	glBindVertexArray(0);
}


bool ModelLoader::LoadModels(const char* path) {
	// Load models
	std::string err;
	bool wasLoaded = tinyobj::LoadObj(shapes, materials, err, path, "resources/");
	if(!wasLoaded || !err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	return true;
}

GLuint ModelLoader::LoadTexture(const char* path) {
	int x, y, nByte;
	unsigned char* data;
	GLuint texID, type;

	data = stbi_load(path, &x, &y, &nByte, 0);

	if(data == NULL) {
		return -1;
	}

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch(nByte) {
		case 1:
			type = GL_R;
			break;

		case 2:
			type = GL_RG;
			break;

		case 3:
			type = GL_RGB;
			break;

		case 4:
		default:
			type = GL_RGBA;
			break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, type, x, y, 0, type, GL_UNSIGNED_BYTE, data);

	/*glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);*/

	free(data);

	printError("Load Texture");
	return texID;
}

bool ModelLoader::LoadTextures() {
	std::string startPath;

	for(size_t i = 0; i < materials.size(); i++) {
		TextureData* data = new TextureData;

		data->diffuseID = -1;
		startPath = "resources/";
		if(!materials[i].diffuse_texname.empty()) {
			data->diffuseID = LoadTexture(startPath.append(materials[i].diffuse_texname).c_str());
		}

		data->bumpID = -1;
		/*startPath = "resources/";
		if(!materials[i].bump_texname.empty()) {
			data->bumpID = LoadTexture(startPath.append(materials[i].bump_texname).c_str());
		}*/

		textures.push_back(data);
	}

	return true;
}

void ModelLoader::AddModel(int id) {
	Model* model = new Model();
	GLuint setProgram;

	model->SetMaterial(textures[shapes[id].mesh.material_ids[0]]);

	if(model->hasBumpTex() || model->hasDiffuseTex()) {
		if(model->hasBumpTex()) {
			setProgram = bumpProgram;
		} else {
			setProgram = textureProgram;
		}
	} else {
		setProgram = simpleProgram;
	}

	model->SetProgram(setProgram);
	model->SetStandardData(shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(),
						   shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(),
						   shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data());

	if(!model->hasBumpTex() && model->hasDiffuseTex()) {
		model->SetTextureData(shapes[id].mesh.texcoords.size(), shapes[id].mesh.texcoords.data());
	}


	models.push_back(model);
}

bool ModelLoader::Init(const char* path) {
	if(!LoadModels(path)) return false;

	if(!LoadTextures()) return false;

	// Load shaders
	GLint err;
	simpleProgram = loadShaders("src/shaders/simpleModel.vert", "src/shaders/simpleModel.frag");
	glGetProgramiv(simpleProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	textureProgram = loadShaders("src/shaders/textureModel.vert", "src/shaders/textureModel.frag");
	glGetProgramiv(textureProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	//bumpProgram = loadShaders("src/shaders/bumpModel.vert", "src/shaders/bumpModel.frag");
	//glGetProgramiv(bumpProgram, GL_LINK_STATUS, &err);
	//if(err == GL_FALSE) return false;

	errorProgram = loadShaders("src/shaders/errorModel.vert", "src/shaders/errorModel.frag");
	glGetProgramiv(errorProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	// Read all models
	for(int i = 0; i < shapes.size(); i++) {
		AddModel(i);
	}

	// Set constant uniforms
	glUseProgram(textureProgram);
	glUniform1i(glGetUniformLocation(textureProgram, "diffuseUnit"), 0);

	//glUseProgram(bumpProgram);
	//glUniform1i(glGetUniformLocation(bumpProgram, "diffuseUnit"), 0);
	//glUniform1i(glGetUniformLocation(bumpProgram, "bumpUnit"), 1);

	printError("init Model");
	return true;
}

void ModelLoader::Draw() {
	for(size_t i = 0; i < models.size(); i++) {
		models[i]->Draw();
	}
	printError("Draw Models");
}