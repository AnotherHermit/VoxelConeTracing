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

#include "ModelLoader.h"

#include "GL_utilities.h"

#include <iostream>

bool ModelLoader::LoadScene(const char* path, std::vector<Model*>* outModels, ShaderList* initShaders, glm::vec3** outMaxVertex, glm::vec3** outMinVertex) {
	shaders = initShaders;
	maxVertex = *outMaxVertex;
	minVertex = *outMinVertex;
	models = outModels;

	if(!LoadModels(path)) return false;

	if(!LoadTextures()) return false;

	// Read all models
	AddModels();

	*outMaxVertex = maxVertex;
	*outMinVertex = minVertex;

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
			std::cerr << "Bpp is in unknown format\n";
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

void ModelLoader::AddModels() {
	for(auto shape = shapes.begin(); shape != shapes.end(); shape++) {

		// Check vertex data for min and max corners
		for(auto vertex = shape->mesh.positions.begin(); vertex != shape->mesh.positions.end()-3; vertex+=3) {
			glm::vec3 currentVertex = glm::vec3(vertex[0], vertex[1], vertex[2]);
			
			if(maxVertex == nullptr) {
				maxVertex = new glm::vec3(currentVertex);
			} else {
				*maxVertex = glm::max(currentVertex, *maxVertex);
			}

			if(minVertex == nullptr) {
				minVertex = new glm::vec3(currentVertex);
			} else {
				*minVertex = glm::min(currentVertex, *minVertex);
			}
		}

		Model* model = new Model();

		// Set material first since this determines shader program
		model->SetMaterial(textures[shape->mesh.material_ids[0]]);

		if(model->hasMaskTex()) {
			model->SetProgram(shaders->mask, shaders->voxelTexture);
		} else if(model->hasDiffuseTex()) {
			model->SetProgram(shaders->texture, shaders->voxelTexture);
		} else {
			model->SetProgram(shaders->simple, shaders->voxel);
		}

		// Load standard vertex data needed by all models, also creates VAO
		model->SetStandardData(shape->mesh.positions.size(), shape->mesh.positions.data(),
							   shape->mesh.normals.size(), shape->mesh.normals.data(),
							   shape->mesh.indices.size(), shape->mesh.indices.data());

		// If a texture is available also load texture coordinate data
		if(model->hasDiffuseTex()) {
			model->SetTextureData(shape->mesh.texcoords.size(), shape->mesh.texcoords.data());
		}
		
		// Sort masked models last in the drawing list since they are transparent
		if(model->hasMaskTex()) {
			models->push_back(model);
		} else {
			models->insert(models->begin(), model);
		}
	}
}
