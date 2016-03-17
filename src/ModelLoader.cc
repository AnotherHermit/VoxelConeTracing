﻿///////////////////////////////////////
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

#include "ModelLoader.h"

#include "GL_utilities.h"

#include <iostream>

ModelLoader::ModelLoader() {
	skipNoTexture = false;
	param.view = 0;
}

void ModelLoader::SetSkipNoTexture(bool setValue) {
	skipNoTexture = setValue;
}

void ModelLoader::SetDrawVoxels(bool enable) {
	for(auto i = models.begin(); i != models.end(); i++) {
		(*i)->SetVoxelDraw(enable);
	}
}

bool ModelLoader::Init(const char* path) {
	if(!LoadModels(path)) return false;

	if(!LoadTextures()) return false;

	// Load shaders for drawing
	GLint err;
	simpleProgram = loadShaders("src/shaders/simpleModel.vert", "src/shaders/simpleModel.frag");
	glGetProgramiv(simpleProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	textureProgram = loadShaders("src/shaders/textureModel.vert", "src/shaders/textureModel.frag");
	glGetProgramiv(textureProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	maskProgram = loadShaders("src/shaders/maskModel.vert", "src/shaders/maskModel.frag");
	glGetProgramiv(maskProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	// Set constant uniforms for the drawing programs
	glUseProgram(textureProgram);
	glUniform1i(glGetUniformLocation(textureProgram, "diffuseUnit"), 0);

	glUseProgram(maskProgram);
	glUniform1i(glGetUniformLocation(maskProgram, "diffuseUnit"), 0);
	glUniform1i(glGetUniformLocation(maskProgram, "maskUnit"), 1);

	// Load shaders for voxelization
	simpleVoxelProgram = loadShaders("src/shaders/voxelizationSimple.vert", "src/shaders/voxelizationSimple.frag");
	glGetProgramiv(maskProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	textureVoxelProgram = loadShaders("src/shaders/voxelizationTexture.vert", "src/shaders/voxelizationTexture.frag");
	glGetProgramiv(maskProgram, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	// Set constant uniforms for voxel programs
	glUseProgram(textureVoxelProgram);
	glUniform1i(glGetUniformLocation(textureVoxelProgram, "diffuseUnit"), 0);

	// Set non-constant uniforms for all programs
	glGenBuffers(1, &modelLoaderBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, modelLoaderBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ModelLoaderParam), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Read all models
	for(int i = 0; i < shapes.size(); i++) {
		AddModel(i);
	}

	printError("init Model");
	return true;
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

bool ModelLoader::LoadTextures() {
	std::string startPath;

	for(size_t i = 0; i < materials.size(); i++) {
		TextureData* data = new TextureData;

		// Load color texture of available
		data->diffuseID = -1;
		startPath = "resources/";
		if(!materials[i].diffuse_texname.empty()) {
			GLuint texID = LoadTexture(startPath.append(materials[i].diffuse_texname).c_str());
			if(texID != -1) {
				data->diffuseID = texID;
			} else {
				std::cerr << "Tried loading texture: " << startPath << " but didn't succeed.\n";
				//return false;
			}
		}

		// Load texture mask if available
		data->maskID = -1;
		startPath = "resources/";
		if(!materials[i].alpha_texname.empty()) {
			GLuint texID = LoadTexture(startPath.append(materials[i].alpha_texname).c_str());
			if(texID != -1) {
				data->maskID = texID;
			} else {
				std::cerr << "Tried loading texture: " << startPath << " but didn't succeed.\n";
				//return false;
			}
		}

		data->diffColor = glm::vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);

		textures.push_back(data);
	}

	return true;
}

GLuint ModelLoader::LoadTexture(const char* path) {
	int width, height, nByte;
	unsigned char* textureData;
	GLuint texID, type;

	textureData = stbi_load(path, &width, &height, &nByte, 0);

	// No texture at the path set id as if no texture was 
	if(textureData == NULL) {
		return -1;
	}

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Select appropriate type of data depending on bytes per pixel
	switch(nByte) {
		case 1:
			type = GL_ALPHA;
			break;

		case 3:
			type = GL_RGB;
			break;

		case 4:
			type = GL_RGBA;
			break;

		default:
			std::cerr << "Bpp is in unknown format...\n";
			return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, textureData);

	// Mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// Dont need the data anymore since it is now handled by OpenGL
	free(textureData);

	printError("Load Texture");
	return texID;
}

void ModelLoader::AddModel(int id) {
	Model* model = new Model();
	GLuint setProgram, voxelProgram;

	// Set material first since this determines shader program
	model->SetMaterial(textures[shapes[id].mesh.material_ids[0]]);

	// Select shader program based on textures available
	if(model->hasMaskTex()) {
		setProgram = maskProgram;
		voxelProgram = textureProgram;
	} else if(model->hasDiffuseTex()) {
		setProgram = textureProgram;
		voxelProgram = textureProgram;
	} else {
		setProgram = simpleProgram;
		voxelProgram = simpleVoxelProgram;
	}

	model->SetProgram(setProgram, voxelProgram);

	// Load standard vertex data needed by all models, also creates VAO
	model->SetStandardData(shapes[id].mesh.positions.size(), shapes[id].mesh.positions.data(),
						   shapes[id].mesh.normals.size(), shapes[id].mesh.normals.data(),
						   shapes[id].mesh.indices.size(), shapes[id].mesh.indices.data());

	// If a texture is available also load texture coordinate data
	if(setProgram != simpleProgram) {
		model->SetTextureData(shapes[id].mesh.texcoords.size(), shapes[id].mesh.texcoords.data());
	}

	// Sort masked models last in the drawing list since they are transparent
	if(setProgram == maskProgram) {
		models.push_back(model);
	} else {
		models.insert(models.begin(), model);
	}
}

void ModelLoader::Draw() {
	// TODO: make this not update every draw call
	glBindBufferBase(GL_UNIFORM_BUFFER, 11, modelLoaderBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(modelLoaderBuffer), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	for(size_t i = 0; i < models.size(); i++) {

		// Don't draw models without texture
		if(skipNoTexture && !models[i]->hasDiffuseTex()) {
			continue;
		}

		models[i]->Draw();
	}
	printError("Draw Models");
}