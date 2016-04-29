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
	// Needed by the model loader, should be cleared after usage by each function
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	// Needed since multiple functions refer to this one
	std::vector<TextureData*> textures;
	
	bool AddModels(std::vector<Model*>* models, ShaderList* shaders);
	bool CalculateMinMax(glm::vec3** maxVertex, glm::vec3** minVertex);

	bool LoadModels(const char* path);
	bool LoadTextures();

	void CalculateTangents();

public:
	ModelLoader() {};

	bool LoadScene(const char* path, std::vector<Model*>* outModels, ShaderList* initShaders);
	bool LoadScene(const char* path, std::vector<Model*>* outModels, ShaderList* initShaders, glm::vec3** outMaxVertex, glm::vec3** outMinVertex);

	bool LoadModel(const char* path, Model* outModel, GLuint shader);

	GLuint LoadTexture(const char* path);
};

#endif // MODELLOADER_H
