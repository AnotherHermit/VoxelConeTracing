///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "tiny_obj_loader.h"

#include "Model.h"

#include "GL_utilities.h"

// ===== ModelLoader class =====

class ModelLoader {
private:
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::vector<Model*>* models;
	std::vector<TextureData*> textures;

	ShaderList* shaders;

	glm::vec3 *maxVertex, *minVertex;

	void AddModels();

	bool LoadModels(const char* path);
	bool LoadTextures();

	GLuint LoadTexture(const char* path);

public:
	ModelLoader() {};

	bool LoadScene(const char* path, std::vector<Model*>* outModels, ShaderList* initShaders, glm::vec3** outMaxVertex, glm::vec3** outMinVertex);
};

#endif // MODELLOADER_H
