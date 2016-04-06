///////////////////////////////////////
//
//	Computer Graphics TSBK07
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform usampler3D voxelData;

in vec3 voxelPos[];
in vec3 exNormal[];

out vec4 outColor;
out vec3 outNormal;
out vec4 outPosition;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 10) uniform CameraBuffer {
	Camera cam;
};

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
	vec4 tempPos;
	uvec4 color = convertIntToVec(texture(voxelData, voxelPos[0]).r);

	if(color.a < 128) {
		return;
	}

	for(int i = 0; i < 3; i++) {
		tempPos = gl_in[i].gl_Position;
		outPosition = tempPos;
		gl_Position = cam.VTPmatrix * tempPos;
		outNormal = exNormal[i];
		outColor = vec4(color) / 255.0f;
		EmitVertex();
	}
	
	EndPrimitive();
}


