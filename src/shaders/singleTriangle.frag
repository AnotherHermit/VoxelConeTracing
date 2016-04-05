///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahl�n - conwa099
//
///////////////////////////////////////

#version 430

in vec2 exTexCoords;
out vec4 outColor;

uniform sampler2D usedView;
uniform sampler3D voxelData;

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
	uint mipLevel;
};

layout (std140, binding = 11) uniform SceneBuffer {
	SceneParams scene;
};

void main()
{	

	if(scene.voxelDraw == 1) {
		outColor = textureLod(usedView, exTexCoords, float(scene.mipLevel));
	} else {
		outColor = textureLod(voxelData, vec3(exTexCoords, float(scene.voxelLayer) / float(scene.voxelRes-1)), float(scene.mipLevel));
	}
}
