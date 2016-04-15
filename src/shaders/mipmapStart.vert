///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in uint inVoxelPos;

uniform uint currentLevel;

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

/*
layout(std430, binding = 1) writeonly buffer SparseBuffer {
	uint sparseList[];
};
*/

uint packARGB8(uvec4 input) {
	uint result = 0;

	result |= (input.a & 0xFF) << 24;
	result |= (input.r & 0xFF) << 16;
	result |= (input.g & 0xFF) << 8;
	result |= (input.b & 0xFF);

	return result;
}

uvec4 unpackARGB8(uint input) {
	uvec4 outVec;
	
	// Put a first to improve max operation but it should not be very noticable
	outVec.a = (input & 0xFF000000) >> 24;
	outVec.r = (input & 0x00FF0000) >> 16;
	outVec.g = (input & 0x0000FF00) >> 8;
	outVec.b = (input & 0x000000FF);

	return outVec;
}

uvec3 unpackRG11B10(uint input) {
	uvec3 outVec;

	outVec.r = (input & 0xFFE00000) >> 21;
	outVec.g = (input & 0x001FFC00) >> 10;
	outVec.b = (input & 0x000003FF);

	return outVec;
}

void main()
{	
	uvec3 voxelCoord = unpackRG11B10(inVoxelPos);

	uvec4 color = unpackARGB8(imageLoad(voxelData, voxelCoord).r);
	vec4 fColor = vec4(color) / float(color.a);
	color = uvec4(fColor * 31.0f);

	uvec3 nextVoxelCoord = voxelCoord >> 1;

	uint prevColor = imageAtomicAdd(voxelDataNextLevel, nextVoxelCoord, color);

	// Check if this voxel was empty before
	if(prevColor == 0) {
		// Write to number of voxels list
		uint nextIndex = atomicAdd(drawCmd[currentLevel].instanceCount, 1);

		// Write to position buffer
		sparseList[nextIndex + drawCmd[currentLevel].baseInstance] = packRG11B10(uvec3(nextVoxelCoord));
	}
}
