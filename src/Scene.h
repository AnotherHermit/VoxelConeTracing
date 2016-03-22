///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef SCENE_H
#define SCENE_H

#include "tiny_obj_loader.h"

#include "Model.h"
#include "GL_utilities.h"

struct SceneParam {
	glm::mat4 MTOmatrix[3]; // Centers and scales scene to fit inside +-1 from three different rotations
	GLuint view;
};

// ===== ModelLoader class =====

class Scene {
private:
	// All models contained in the scene
	std::vector<Model*>* models;

	// Programs used to draw models
	ShaderList* shaders;

	// Scene settings
	bool skipNoTexture;
	bool drawVoxels;

	// Uniform buffer with scene settings
	SceneParam param;
	GLuint sceneBuffer;

	// Empty framebuffer for voxelization
	GLuint voxelFBO;

	// Voxel 2D view textures
	GLuint frontTex, sideTex, topTex;
	GLuint voxelRes;

	// Scene information
	glm::vec3 *maxVertex, *minVertex, centerVertex;
	GLfloat scale;

	void GenViewTexture(GLuint* viewID);

public:
	Scene();

	void SetSkipNoTexture(bool setValue);
	bool* GetSkipNoTexturePtr() { return &skipNoTexture; }
	void SetDrawVoxels(bool enable);
	GLuint* GetVoxelResPtr() { return &voxelRes; }
	GLuint* GetViewPtr() { return &param.view; }

	bool Init(const char* path, ShaderList* initShaders);
	void Draw();
	void Voxelize();
};

#endif // SCENE_H
