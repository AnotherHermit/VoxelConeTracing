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

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
};

layout (std140, binding = 0) uniform SceneBuffer {
	SceneParams scene;
};

struct DrawElementsIndirectCommand {
	uint vertexCount;
	uint instanceCount;
	uint firstVertex;
	uint baseVertex;
	uint baseInstance;
};

// Atomic counter of rendered particles;
layout(std430, binding = 0) buffer DrawCmdBuffer {
	DrawElementsIndirectCommand drawCmd;
};

layout(std430, binding = 1) writeonly buffer SparseBuffer {
	int sparseList[];
};

ivec3 voxelCoord;

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
	if(convertIntToVec(prevColor).a == 0) {
		// Write to number of voxels list
		uint nextIndex = atomicAdd(drawCmd.instanceCount, 1);
		// Write to position buffer
		sparseList[nextIndex * 3] = voxelCoord.x;
		sparseList[nextIndex * 3 + 1] = voxelCoord.y;
		sparseList[nextIndex * 3 + 2] = voxelCoord.z;

		// Create a sparse list for the next level as well
		//nextIndex = atomicAdd(levelCounter, 1);
		//sparseListNextLevel[nextIndex] = voxelCoord >> 1;
		//imageAtomicAdd(voxelDataNextLevel, voxelCoord >> 1, 1);
	}
}
