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

struct ModelLoaderParam {
	GLuint view;
};

// ===== ModelLoader class =====

class ModelLoader {
private:
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::vector<Model*> models;
	std::vector<TextureData*> textures;

	GLuint simpleProgram, textureProgram, maskProgram;
	bool skipNoTexture;
	ModelLoaderParam param;
	GLuint modelLoaderBuffer;
	
	GLuint simpleVoxelProgram, textureVoxelProgram;

	void AddModel(int id);

	bool LoadModels(const char* path);
	bool LoadTextures();

	GLuint LoadTexture(const char* path);

public:
	ModelLoader();

	void SetSkipNoTexture(bool setValue);
	bool* GetSkipNoTexturePtr() { return &skipNoTexture; }
	void SetDrawVoxels(bool enable);
	GLuint* GetViewPtr() { return &param.view; }

	bool Init(const char* path);
	void Draw();
};

#endif // MODELLOADER_H
