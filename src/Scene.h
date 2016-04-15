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

#define MAX_MIP_MAP_LEVELS 9

// Values the should exist on the GPU
struct SceneParam {
	glm::mat4 MTOmatrix[3]; // Centers and scales scene to fit inside +-1 from three different rotations
	glm::mat4 MTWmatrix; // Matrix for voxel data
	GLuint voxelDraw; // 
	GLuint view;
	GLuint voxelRes;
	GLuint voxelLayer;
	GLuint numMipLevels;
	GLuint mipLevel;
};

// Scene Options struct
struct SceneOptions {
	bool skipNoTexture;
	bool drawVoxels;
	bool drawModels;
	bool drawTextures;
};

// The different view directions
enum ViewDirection {
	VIEW_X,
	VIEW_Y,
	VIEW_Z
};

// Voxel Resolutions used, over 512 not really needed (even 512 is stretching it on low level HW)
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

	// Scene Options
	SceneOptions options;

	// Uniform buffer with scene settings
	SceneParam param;
	GLuint sceneBuffer;

	// Draw indirect buffer and struct
	DrawElementsIndirectCommand drawIndCmd[MAX_MIP_MAP_LEVELS];
	GLuint drawIndBuffer;

	// Sparse List Buffer
	GLuint sparseListBuffer;

	// MipMap Stuff
	GLuint mipmapVAO;

	// Empty framebuffer for voxelization
	GLuint voxelFBO;

	// Voxel view textures
	GLuint voxel2DTex;
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
	TwStructMember sceneTwMembers[6];
	TwStructMember sceneOptionTwMembers[4];
	TwStructMember drawIndTwMembers[5];
	static TwType* resTwType;
	static TwType* viewTwType;
	static TwType* sceneTwStruct;
	static TwType* sceneOptionsTwStruct;
	static TwType* drawIndTwStruct;
	static bool isInitialized;
	bool InitializeAntBar();

public:
	Scene();

	bool Init(const char* path, ShaderList* initShaders);
	void Draw();
	void Voxelize();
	void InitMipMap();
	void MipMap();
	void UploadParams();
	void UpdateBuffers();
	void ResizeBuffer();

	// AntTweakBar
	static TwType GetSceneTwType() { return *sceneTwStruct; }
	static TwType GetSceneOptionTwType() { return *sceneOptionsTwStruct; }
	static TwType GetDrawIndTwType() { return *drawIndTwStruct; }
	static void TW_CALL SetSceneCB(const void* value, void* clientData);
	static void TW_CALL GetSceneCB(void* value, void* clientData);
	static void TW_CALL SetSceneOptionsCB(const void* value, void* clientData);
	static void TW_CALL GetSceneOptionsCB(void* value, void* clientData);
	static void TW_CALL SetDrawIndCB(const void* value, void* clientData);
	static void TW_CALL GetDrawIndCB(void* value, void* clientData);
};

#endif // SCENE_H
