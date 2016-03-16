///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#ifndef myDrawable_H
#define myDrawable_H

#include "tiny_obj_loader.h"

#include "Model.h"

// ===== ModelLoader class =====

class ModelLoader {
private:
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::vector<Model*> models;
	std::vector<TextureData*> textures;

	GLuint simpleProgram, textureProgram, bumpProgram, errorProgram, maskProgram;
	bool skipNoTexture;


	void AddModel(int id);

	bool LoadModels(const char* path);
	bool LoadTextures();

	GLuint LoadTexture(const char* path);

public:
	ModelLoader();

	void SetSkipNoTexture(bool setValue);
	bool* GetSkipNoTexturePtr() { return &skipNoTexture; }

	bool Init(const char* path);
	void Draw();
};

#endif // myDrawable_H
