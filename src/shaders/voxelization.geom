///////////////////////////////////////
//
//	Computer Graphics TSBK07
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 exTexCoords[3];

flat out uint domInd;
out vec2 intTexCoords;
out vec4 shadowCoord;
out vec3 exNormal;

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

vec3 CalculateNormal(vec3 a, vec3 b, vec3 c) {
	vec3 ab = b - a;
	vec3 ac = c - a;
	return normalize(cross(ab,ac));
}

void main()
{
	vec3 dir = abs(CalculateNormal(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz));
	exNormal = dir;
	float maxComponent = max(dir.x, max(dir.y, dir.z));
	uint ind = maxComponent == dir.x ? 0 : maxComponent == dir.y ? 1 : 2;
	domInd = ind;

	gl_Position = scene.MTOmatrix[ind] * gl_in[0].gl_Position;
	shadowCoord = scene.MTShadowMatrix * scene.MTOmatrix[2] * gl_in[0].gl_Position;
	intTexCoords = exTexCoords[0];
	EmitVertex();
	
	gl_Position = scene.MTOmatrix[ind] * gl_in[1].gl_Position;
	shadowCoord = scene.MTShadowMatrix * scene.MTOmatrix[2] * gl_in[1].gl_Position;
	intTexCoords = exTexCoords[1];
	EmitVertex();

	gl_Position = scene.MTOmatrix[ind] * gl_in[2].gl_Position;
	shadowCoord = scene.MTShadowMatrix * scene.MTOmatrix[2] * gl_in[2].gl_Position;
	intTexCoords = exTexCoords[2];
	EmitVertex();

	EndPrimitive();
}


