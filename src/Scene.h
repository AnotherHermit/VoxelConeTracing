///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef SCENE_H
#define SCENE_H

#include "tiny_obj_loader.h"

#include "AntTweakBar.h"

#include "Model.h"
#include "GL_utilities.h"

struct SceneParam {
	glm::mat4 MTOmatrix[3]; // Centers and scales scene to fit inside +-1 from three different rotations
	glm::mat4 MTWmatrix; // Matrix for voxel data
	GLuint voxelDraw; // 
	GLuint view;
	GLuint voxelRes;
	GLuint voxelLayer;
};

enum ViewDirection {
	VIEW_X,
	VIEW_Y,
	VIEW_Z
};

enum VoxelResolutions {
	RES16 = 16,
	RES32 = 32,
	RES64 = 64,
	RES128 = 128,
	RES256 = 256,
	RES512 = 512
};

// ===== ModelLoader class =====

class Scene {
private:
	// All models contained in the scene
	std::vector<Model*>* models;
	Model* voxelModel;

	// Programs used to draw models
	ShaderList* shaders;

	// Scene settings
	bool skipNoTexture;
	bool drawVoxels;
	bool drawModels;
	bool drawTextures;

	// Uniform buffer with scene settings
	SceneParam param;
	GLuint sceneBuffer;
	void UploadParams();

	// Empty framebuffer for voxelization
	GLuint voxelFBO;

	// Voxel view textures
	GLuint xTex, yTex, zTex;
	GLuint voxelTex;

	// Scene information
	glm::vec3 *maxVertex, *minVertex, centerVertex;
	GLfloat scale;

	// Texture generation
	void GenViewTexture(GLuint* viewID);
	void GenVoxelTexture(GLuint* texID);

	// AntTweakBar Stuff
	TwEnumVal viewTwEnum[3];
	TwEnumVal resTwEnum[5];
	TwStructMember sceneTwMembers[4];
	static TwType* resTwType;
	static TwType* viewTwType;
	static TwType* sceneTwStruct;
	static bool isInitialized;

public:
	Scene();

	GLuint* GetVoxelDataDrawPtr() { return &param.voxelDraw; }
	GLuint* GetViewPtr() { return &param.view; }
	GLuint* GetVoxelResPtr() { return &param.voxelRes; }
	GLuint* GetLayerPtr() { return &param.voxelLayer; }

	bool* GetSkipNoTexturePtr() { return &skipNoTexture; }
	bool* GetModelDrawPtr() { return &drawModels; }
	bool* GetVoxelDrawPtr() { return &drawVoxels; }
	bool* GetTextureDrawPtr() { return &drawTextures; }

	bool Init(const char* path, ShaderList* initShaders);
	void Draw();
	void Voxelize();

	// AntTweakBar
	static TwType GetSceneTwType() { return *sceneTwStruct; }
	static void TW_CALL SetSceneCB(const void* value, void* clientData);
	static void TW_CALL GetSceneCB(void* value, void* clientData);
};

#endif // SCENE_H
