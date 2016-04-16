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
#define MAX_VOXEL_RES 512
// 512 ^ 3
#define MAX_SPARSE_BUFFER_SIZE 134217728

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
	DrawElementsIndirectCommand drawIndCmd[10];
	GLuint drawIndBuffer;

	// Compute indirect buffer and struct
	ComputeIndirectCommand compIndCmd[10];
	GLuint compIndBuffer;

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
	
	// AntTweakBar Stuff
	TwEnumVal viewTwEnum[3];
	TwEnumVal resTwEnum[5];
	TwStructMember sceneTwMembers[6];
	TwStructMember sceneOptionTwMembers[4];
	TwStructMember drawIndTwMembers[2];
	TwStructMember compIndTwMembers[1];
	static TwType* resTwType;
	static TwType* viewTwType;
	static TwType* sceneTwStruct;
	static TwType* sceneOptionsTwStruct;
	static TwType* drawIndTwStruct;
	static TwType* compIndTwStruct;
	static bool isInitialized;
	
	// Init functions
	bool InitAntBar();
	void InitBuffers();
	void InitMipMap();
	bool InitVoxel();
	
	// Setup functions
	void SetupDrawInd();
	void SetupCompInd();
	bool SetupScene(const char* path);
	void SetupTextures();

	// Debug funtions
	void PrintDrawIndCmd();
	void PrintCompIndCmd();
	void PrintBuffer(GLuint bufferID, GLuint elements);

	// Draw functions
	void DrawTextures();
	void DrawScene();
	void DrawVoxels();

public:
	Scene();

	bool Init(const char* path, ShaderList* initShaders);
	void Draw();
	void Voxelize();
	void MipMap();

	void UpdateBuffers();

	// AntTweakBar
	static TwType GetSceneTwType() { return *sceneTwStruct; }
	static TwType GetSceneOptionTwType() { return *sceneOptionsTwStruct; }
	static TwType GetDrawIndTwType() { return *drawIndTwStruct; }
	static TwType GetCompIndTwType() { return *compIndTwStruct; }
	static void TW_CALL SetSceneCB(const void* value, void* clientData);
	static void TW_CALL GetSceneCB(void* value, void* clientData);
	static void TW_CALL SetSceneOptionsCB(const void* value, void* clientData);
	static void TW_CALL GetSceneOptionsCB(void* value, void* clientData);
	static void TW_CALL SetDrawIndCB(const void* value, void* clientData);
	static void TW_CALL GetDrawIndCB(void* value, void* clientData);
	static void TW_CALL SetCompIndCB(const void* value, void* clientData);
	static void TW_CALL GetCompIndCB(void* value, void* clientData);
};

#endif // SCENE_H
