///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

flat in uint domInd;

uniform vec3 diffColor;

uniform layout(R32UI) uimage2DArray voxelTextures;
uniform layout(R32UI) uimage3D voxelData;
uniform layout(R32UI) uimage3D voxelDataNextLevel;

ivec3 voxelCoord;

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

struct ComputeIndirectCommand {
	GLuint workGroupSizeX;
	GLuint workGroupSizeY;
	GLuint workGroupSizeZ;
};

layout(std430, binding = 0) buffer ComputeCmdBuffer {
	ComputeIndirectCommand ComputeCmd[9];
};

layout(std430, binding = 2) writeonly buffer SparseBuffer {
	uint sparseList[];
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
	// Set constant color for textureless models
	uint color = packARGB8(uvec4(uvec3(diffColor*255), 255));

	imageAtomicMax(voxelTextures, ivec3(ivec2(gl_FragCoord.xy), domInd), color);

	int depthCoord = int(gl_FragCoord.z * scene.voxelRes);

	if(domInd == 0) {
		voxelCoord = ivec3(depthCoord, gl_FragCoord.y, scene.voxelRes - gl_FragCoord.x);
	} else if (domInd == 1) {
		voxelCoord = ivec3(gl_FragCoord.x, depthCoord, scene.voxelRes - gl_FragCoord.y);
	} else {
		voxelCoord = ivec3(gl_FragCoord.x, gl_FragCoord.y, depthCoord);
	}

	uint prevColor = imageAtomicMax(voxelData, voxelCoord, color);

	// Check if this voxel was empty before
	if(prevColor == 0) {
		// Write to number of voxels list
		uint nextIndex = atomicAdd(drawCmd[0].instanceCount, 1);
		// Write to position buffer
		sparseList[nextIndex] = packRG11B10(uvec3(voxelCoord));

		// Count the number of voxels contributing to the next level
		uint firstWrite = imageAtomicAdd(voxelDataNextLevel, voxelCoord >> 1, packARGB8(uvec4(uvec3(diffColor*31),31)));

		if(firstWrite == 0) {
			// Create a sparse list for the next level as well
			nextIndex = atomicAdd(drawCmd[1].instanceCount, 1);
			sparseList[nextIndex + drawCmd[1].baseInstance] = packRG11B10(uvec3(voxelCoord >> 1));
		}
	}
}
