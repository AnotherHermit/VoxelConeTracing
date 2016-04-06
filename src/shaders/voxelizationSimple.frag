///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

flat in uint domInd;

uniform vec3 diffColor;

uniform layout(RGBA32F) image2D xView;
uniform layout(RGBA32F) image2D yView;
uniform layout(RGBA32F) image2D zView;
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
	vec4 color = vec4(diffColor, 1.0f);
	uint uColor = convertVecToInt(uvec4(color*255));

	if(domInd == 0) {
		imageStore(xView, ivec2(gl_FragCoord.xy), color);
		imageAtomicMax(voxelData, ivec3(gl_FragCoord.z * scene.voxelRes,gl_FragCoord.y, scene.voxelRes - gl_FragCoord.x), uColor);
	} else if (domInd == 1) {
		imageStore(yView, ivec2(gl_FragCoord.xy), color);
		imageAtomicMax(voxelData, ivec3(gl_FragCoord.x, scene.voxelRes * gl_FragCoord.z, scene.voxelRes - gl_FragCoord.y), uColor);
	} else {
		imageStore(zView, ivec2(gl_FragCoord.xy), color);
		imageAtomicMax(voxelData, ivec3(gl_FragCoord.x, gl_FragCoord.y, scene.voxelRes * gl_FragCoord.z), uColor);
	}
}
