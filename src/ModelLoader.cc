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

#include "glm\gtc\constants.hpp"

#include <iostream>

bool ModelLoader::LoadScene(const char* path, std::vector<Model*>* outModels, glm::vec3** outMaxVertex, glm::vec3** outMinVertex) {
	if(!LoadModels(path)) return false;
	if(!LoadTextures()) return false;

	// Read all models
	if(!AddModels(outModels)) return false;

	// Find the size of the scene
	if(!CalculateMinMax(outMaxVertex, outMinVertex)) return false;

	// If we want to reuse the ModelLoader
	shapes.clear();
	materials.clear();

	return true;
}

bool ModelLoader::LoadModel(const char* path, Model* outModel) {
	if(!LoadModels(path)) return false;

	// Load standard vertex data needed by all models, also creates VAO
	outModel->SetStandardData(shapes[0].mesh.positions.size(), shapes[0].mesh.positions.data(),
							  shapes[0].mesh.normals.size(), shapes[0].mesh.normals.data(),
							  shapes[0].mesh.indices.size(), shapes[0].mesh.indices.data(),
							  shapes[0].mesh.tangents.size(), shapes[0].mesh.tangents.data(),
							  shapes[0].mesh.bitangents.size(), shapes[0].mesh.bitangents.data());

	return true;
}

bool ModelLoader::LoadScene(const char* path, std::vector<Model*>* outModels) {
	return LoadScene(path, outModels, nullptr, nullptr);
}

bool ModelLoader::LoadModels(const char* path) {
	// Load models
	std::string err;
	bool wasLoaded = tinyobj::LoadObj(shapes, materials, err, path, "resources/");
	if(!wasLoaded || !err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	CalculateTangents();

	return true;
}

bool ModelLoader::LoadTextures() {
	std::string startPath;

	for(size_t i = 0; i < materials.size(); i++) {
		TextureData* data = new TextureData;
		
		// Set subroutine to use;
		data->subID = CONSTANT;
		
		// Load color texture of available
		data->diffuseID = 0;
		startPath = "resources/";
		if(!materials[i].diffuse_texname.empty()) {
			GLuint texID = LoadTexture(startPath.append(materials[i].diffuse_texname).c_str());
			if(texID != 0) {
				data->diffuseID = texID;
				data->subID = TEXTURE;
			} else {
				std::cerr << "Tried loading texture: " << startPath << " but didn't succeed.\n";
				//return false;
			}
		}

		// Load texture mask if available
		data->maskID = 0;
		startPath = "resources/";
		if(!materials[i].alpha_texname.empty()) {
			GLuint texID = LoadTexture(startPath.append(materials[i].alpha_texname).c_str());
			if(texID != 0) {
				data->maskID = texID;
				data->subID = MASK;
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
		return 0;
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

	return texID;
}

bool ModelLoader::AddModels(std::vector<Model*>* models) {
	if(models == nullptr) {
		std::cout << "No model vector was supplied when loading scene" << std::endl;
		return false;
	}

	for(auto shape = shapes.begin(); shape != shapes.end(); shape++) {
		Model* model = new Model();

		// Load standard vertex data needed by all models, also creates VAO
		model->SetStandardData(shape->mesh.positions.size(), shape->mesh.positions.data(),
							   shape->mesh.normals.size(), shape->mesh.normals.data(),
							   shape->mesh.indices.size(), shape->mesh.indices.data(),
							   shape->mesh.tangents.size(), shape->mesh.tangents.data(),
							   shape->mesh.bitangents.size(), shape->mesh.bitangents.data());

		// Set material
		if(shape->mesh.material_ids[0] != -1) {
			model->SetMaterial(textures[shape->mesh.material_ids[0]]);
		}

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
	return true;
}

bool ModelLoader::CalculateMinMax(glm::vec3** maxVertex, glm::vec3** minVertex) {
	if(maxVertex == nullptr || minVertex == nullptr) {
		std::cout << "No min/max vector was supplied when loading a scene" << std::endl;
		return true;
	}


	for(auto shape = shapes.begin(); shape != shapes.end(); shape++) {

		// Check vertex data for min and max corners
		for(auto vertex = shape->mesh.positions.begin(); vertex != shape->mesh.positions.end() - 3; vertex += 3) {
			glm::vec3 currentVertex = glm::vec3(vertex[0], vertex[1], vertex[2]);

			if(*maxVertex == nullptr) {
				*maxVertex = new glm::vec3(currentVertex);
			} else {
				**maxVertex = glm::max(currentVertex, **maxVertex);
			}

			if(*minVertex == nullptr) {
				*minVertex = new glm::vec3(currentVertex);
			} else {
				**minVertex = glm::min(currentVertex, **minVertex);
			}
		}
	}
	return true;
}

void ModelLoader::CalculateTangents() {
	for(auto shape = shapes.begin(); shape != shapes.end(); shape++) {

		// Check vertex data for min and max corners
		for(auto normal = shape->mesh.normals.begin();
			normal != shape->mesh.normals.end() - 3; normal += 3) {
			glm::vec3 currentNormal = glm::normalize(glm::vec3(normal[0], normal[1], normal[2]));
			glm::vec3 yVec = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 zVec = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 tempTan1 = glm::cross(yVec, currentNormal);
			glm::vec3 tempTan2 = glm::cross(currentNormal, zVec);

			float a = normal[0];
			float b = normal[1];
			float c = normal[2];

			//            glm::vec3 tangCand[] = {glm::vec3(-b - c, a, a), glm::vec3(c, c, -a - b)};
			//            int tangSelect = int((c > glm::epsilon<float>()) && (-a - b > glm::epsilon<float>()));
			glm::vec3 tangCand[] = { tempTan1, tempTan2 };
			//            float res = glm::length(tempTan1);
			int tangSelect = int(glm::length(tempTan1) < glm::epsilon<float>());
			glm::vec3 tangent = glm::normalize(tangCand[tangSelect]);
			glm::vec3 bitangent = normalize(cross(currentNormal, tangent));

			shape->mesh.tangents.push_back(tangent.x);
			shape->mesh.tangents.push_back(tangent.y);
			shape->mesh.tangents.push_back(tangent.z);
			shape->mesh.bitangents.push_back(bitangent.x);
			shape->mesh.bitangents.push_back(bitangent.y);
			shape->mesh.bitangents.push_back(bitangent.z);
		}
	}
}