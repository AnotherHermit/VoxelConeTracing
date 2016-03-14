///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef myDrawable_H
#define myDrawable_H

#include "tiny_obj_loader.h"

#include "Camera.h"

#include "GL_utilities.h"

#include "glm.hpp"

// ===== ModelData struct =====

struct ModelData {

	GLuint drawBuffers[3];
	GLuint drawVAO;
};


// ===== Model class =====

class Model {
protected:
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<ModelData*> models;

	GLuint program;

	void addModel(int id);

	bool loadModels(const char* path);

public:
	Model();

	bool Init(const char* path);
	void Draw();
};

#endif // myDrawable_H
