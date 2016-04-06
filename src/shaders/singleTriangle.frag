///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 exTexCoords;
out vec4 outColor;

uniform usampler2DArray voxelTextures;
uniform usampler3D voxelData;

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

uvec4 convertIntToVec(uint input) {
	uint r,g,b,a;
	
	b = input & 0xFF;
	input = input >> 8;
	g = input & 0xFF;
	input = input >> 8;
	r = input & 0xFF;
	input = input >> 8;
	a = input;

	return uvec4(r,g,b,a);
}

uint convertVecToInt(uvec4 input) {
	uint result = input.a;

	result = result << 8;
	result |= input.r;
	result = result << 8;
	result |= input.g;
	result = result << 8;
	result |= input.b;

	return result;
}

void main()
{	
	vec4 color2D = vec4(convertIntToVec(texture(voxelTextures, vec3(exTexCoords, float(scene.view))).r)) / 255.0f;
	vec4 color3D = vec4(convertIntToVec(texture(voxelData, vec3(exTexCoords, float(scene.voxelLayer) / float(scene.voxelRes-1))).r)) / 255.0f;
	outColor = color2D * (scene.voxelDraw) + color3D * (1 - (scene.voxelDraw));
}
