///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahl�n - conwa099
//
///////////////////////////////////////

#version 430

flat in uint domInd;

uniform vec3 diffColor;

uniform layout(R32UI) uimage2DArray voxelTextures;
uniform layout(R32UI) uimage3D voxelData;

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
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
	// Set constant color for textureless models
	uint color = convertVecToInt(uvec4(uvec3(diffColor*255), 255));

	imageAtomicMax(voxelTextures, ivec3(ivec2(gl_FragCoord.xy), domInd), color);

	if(domInd == 0) {
		imageAtomicMax(voxelData, ivec3(gl_FragCoord.z * scene.voxelRes,gl_FragCoord.y, scene.voxelRes - gl_FragCoord.x), color);
	} else if (domInd == 1) {
		imageAtomicMax(voxelData, ivec3(gl_FragCoord.x, scene.voxelRes * gl_FragCoord.z, scene.voxelRes - gl_FragCoord.y), color);
	} else {
		imageAtomicMax(voxelData, ivec3(gl_FragCoord.x, gl_FragCoord.y, scene.voxelRes * gl_FragCoord.z), color);
	}
}
