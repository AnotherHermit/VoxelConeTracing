///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 intTexCoords;
in vec4 shadowCoord;

flat in uint domInd;

layout(location = 1) uniform sampler2D diffuseUnit;

layout(location = 3) uniform layout(R32UI) uimage2DArray voxelTextures;
layout(location = 4) uniform layout(R32UI) uimage3D voxelData;
layout(location = 6) uniform usampler2D shadowMap;

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	mat4 MTShadowMatrix;
	vec3 lightDir;
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
	DrawElementsIndirectCommand drawCmd[10];
};

struct ComputeIndirectCommand {
	uint workGroupSizeX;
	uint workGroupSizeY;
	uint workGroupSizeZ;
};

layout(std430, binding = 1) buffer ComputeCmdBuffer {
	ComputeIndirectCommand compCmd[10];
};

layout(std430, binding = 2) writeonly buffer SparseBuffer {
	uint sparseList[];
};

struct VoxelData {
	vec4 color;
	uint light;
	uint count;
};

uint packARGB8(VoxelData input) {
	uint result = 0;

	uvec3 uiColor = uvec3(input.color.rgb * 31.0f * float(input.count));

	result |= (input.light & 0x0F) << 28;
	result |= (input.count & 0x0F) << 24;
	result |= (uiColor.r & 0xFF) << 16;
	result |= (uiColor.g & 0xFF) << 8;
	result |= (uiColor.b & 0xFF);

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
	// Set constant color for textured models
	VoxelData data = VoxelData(texture(diffuseUnit, intTexCoords), 0x0, 0x8);

	ivec3 voxelCoord;
	int depthCoord = int(gl_FragCoord.z * scene.voxelRes);

	if(domInd == 0) {
		voxelCoord = ivec3(depthCoord, gl_FragCoord.y, scene.voxelRes - gl_FragCoord.x);
	} else if (domInd == 1) {
		voxelCoord = ivec3(gl_FragCoord.x, depthCoord, scene.voxelRes - gl_FragCoord.y);
	} else {
		voxelCoord = ivec3(gl_FragCoord.x, gl_FragCoord.y, depthCoord);
	}

	// Calculate shadows
	vec3 lightCoord = shadowCoord.xyz / 2;
	lightCoord += vec3(0.5f);

	float shadowDepth = texture(shadowMap, lightCoord.xy).r / 65536.0f;
	float bias = 2 / float(scene.voxelRes);

	if(shadowDepth > (1.0f - lightCoord.z) - bias) {
		data.light = 0x1;
	}

	uint outData = packARGB8(data);

	imageAtomicMax(voxelTextures, ivec3(ivec2(gl_FragCoord.xy), domInd), outData);
	uint prevData = imageAtomicMax(voxelData, voxelCoord, outData);

	// Check if this voxel was empty before
	if(prevData == 0) {
		// Write to number of voxels list
		uint nextIndex = atomicAdd(drawCmd[0].instanceCount, 1);
		
		// Calculate and store number of workgroups needed
		uint compWorkGroups = ((nextIndex + 1) >> 6) + 1; // 6 = log2(workGroupSize = 64)
		atomicMax(compCmd[0].workGroupSizeX, compWorkGroups);

		// Write to position buffer
		sparseList[nextIndex + drawCmd[0].baseInstance] = packRG11B10(uvec3(voxelCoord));
	}
}