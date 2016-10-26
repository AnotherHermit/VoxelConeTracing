///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec2 intTexCoords;
in vec4 shadowCoord;
in vec3 exNormal;

flat in uint domInd;

layout(location = 0) uniform vec3 diffColor;

layout(binding = 0) uniform sampler2D diffuseUnit;
layout(binding = 2, r32ui) uniform uimage2DArray voxelTextures;
layout(binding = 3, r32ui) uniform uimage3D voxelData;
layout(binding = 5) uniform sampler2DShadow shadowMap;

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

uint packARGB8(VoxelData dataIn) {
	uint result = 0;

	uvec3 uiColor = uvec3(dataIn.color.rgb * 31.0f * float(dataIn.count));

	result |= (dataIn.light & 0x0F) << 28;
	result |= (dataIn.count & 0x0F) << 24;
	result |= (uiColor.r & 0xFF) << 16;
	result |= (uiColor.g & 0xFF) << 8;
	result |= (uiColor.b & 0xFF);

	return result;
}

uint packRG11B10(uvec3 dataIn) {
	uint result = 0;

	result |= (dataIn.r & 0x7FF) << 21;
	result |= (dataIn.g & 0x7FF) << 10;
	result |= (dataIn.b & 0x3FF);

	return result;
}

subroutine vec4 SampleColor();

layout(index = 0) subroutine(SampleColor) 
vec4 DiffuseColor() {
	return vec4(diffColor, 1.0f);
}

layout(index = 1) subroutine(SampleColor)
vec4 TextureColor() {
	return vec4(texture(diffuseUnit, intTexCoords).rgb, 1.0f);
}

layout(location = 0) subroutine uniform SampleColor GetColor;

void main()
{	
	// Set constant color for textured models
	VoxelData data = VoxelData(vec4(0.0f), 0x0, 0x8);

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
	vec3 s = shadowCoord.xyz * 0.5f + 0.5f;
    vec3 l = scene.lightDir;
    float cosTheta = max(dot(l,normalize(exNormal)), 0.0f);
    float b = 0.005f*tan(acos(cosTheta));
    b = clamp(b, 0.0f,0.02f);
    s.z = s.z - b;

	float shadow = texture(shadowMap, s.xyz);
	data.light = uint(8.0f * (1.0f - shadow));

	// Set color
	data.color = GetColor() * cosTheta;
	
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