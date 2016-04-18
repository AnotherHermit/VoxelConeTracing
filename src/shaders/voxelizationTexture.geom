///////////////////////////////////////
//
//	Computer Graphics TSBK07
//	Conrad Wahl�n - conwa099
//
///////////////////////////////////////

#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 exNormal[3];
in vec2 exTexCoords[3];

flat out uint domInd;
out vec2 intTexCoords;

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	mat4 MTShadowMatrix;
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

void main()
{
	vec3 dir = abs(exNormal[0]);
	float maxComponent = max(dir.x, max(dir.y, dir.z));
	uint ind = maxComponent == dir.x ? 0 : maxComponent == dir.y ? 1 : 2;

	gl_Position = scene.MTOmatrix[ind] * gl_in[0].gl_Position;
	domInd = ind;
	intTexCoords = exTexCoords[0];
	EmitVertex();
	
	gl_Position = scene.MTOmatrix[ind] * gl_in[1].gl_Position;
	domInd = ind;
	intTexCoords = exTexCoords[1];
	EmitVertex();

	gl_Position = scene.MTOmatrix[ind] * gl_in[2].gl_Position;
	domInd = ind;
	intTexCoords = exTexCoords[2];
	EmitVertex();

	EndPrimitive();
}


