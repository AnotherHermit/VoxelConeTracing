///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in uint inVoxelPos;

uniform layout(R32UI) uimage3D voxelData;
uniform layout(R32UI) uimage3D voxelDataNextLevel;

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
	uint numMipLevels;
	uint mipLevel;
};

layout (std140, binding = 1) uniform SceneBuffer {
	SceneParams scene;
};

struct DrawElementsIndirectCommand {
	uint vertexCount;
	uint instanceCount;
	uint firstVertex;
	uint baseVertex;
	uint baseInstance;
};

layout(std430, binding = 0) buffer DrawCmdBuffer {
	DrawElementsIndirectCommand drawCmd[9];
};

uint packARGB8(uvec4 input) {
	uint result = 0;

	result |= (input.a & 0xFF) << 24;
	result |= (input.r & 0xFF) << 16;
	result |= (input.g & 0xFF) << 8;
	result |= (input.b & 0xFF);

	return result;
}

uint packRG11B10(uvec3 input) {
	uint result = 0;

	result |= (input.r & 0x7FF) << 21;
	result |= (input.g & 0x7FF) << 10;
	result |= (input.b & 0x3FF);

	return result;
}

void main()
{	
	
}
