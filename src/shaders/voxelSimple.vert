///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 inPosition;
in vec3 inNormal;
in ivec3 inVoxelPos;

out vec4 outPosition;
out vec3 outNormal;
out vec4 outColor;

uniform usampler3D voxelData;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 2) uniform CameraBuffer {
	Camera cam;
};

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

layout (std140, binding = 0) uniform SceneBuffer {
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

void main(void)
{
	float size = float(scene.voxelRes);
	vec3 voxelPos = vec3(inVoxelPos) / size;

	uvec4 color = convertIntToVec(texture(voxelData, voxelPos).r);
	outColor = vec4(color) / 255.0f;
	
	outNormal = mat3(cam.WTVmatrix) * inNormal;
	vec4 temp = cam.WTVmatrix * scene.MTWmatrix * vec4(inPosition / size + 2.0f * voxelPos - vec3(1.0f), 1.0f);
	outPosition = temp;
	gl_Position = cam.VTPmatrix * temp;
}

