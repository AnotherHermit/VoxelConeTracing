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

layout (std140, binding = 0) uniform CameraBuffer {
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

layout (std140, binding = 1) uniform SceneBuffer {
	SceneParams scene;
};

uvec4 unpackARGB8(uint input) {
	uvec4 outVec;
	
	// Put a first to improve max operation but it should not be very noticable
	outVec.a = (input & 0xFF000000) >> 24;
	outVec.r = (input & 0x00FF0000) >> 16;
	outVec.g = (input & 0x0000FF00) >> 8;
	outVec.b = (input & 0x000000FF);

	return outVec;
}

void main()
{
	vec4 tempPos;
	uvec4 color = unpackARGB8(texture(voxelData, voxelPos[0]).r);

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


