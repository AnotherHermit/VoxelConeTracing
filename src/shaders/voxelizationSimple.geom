///////////////////////////////////////
//
//	Computer Graphics TSBK07
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 exNormal[3];

flat out uint domInd;

struct SceneParams {
	mat4 MTOmatrix[3];
	uint view;
};

layout (std140, binding = 11) uniform SceneBuffer {
	SceneParams scene;
};

void main()
{
	vec3 dir = abs(exNormal[0]);
	float maxComponent = max(dir.x, max(dir.y, dir.z));
	uint ind = maxComponent == dir.x ? 0 : maxComponent == dir.y ? 1 : 2;

	gl_Position = scene.MTOmatrix[ind] * gl_in[0].gl_Position;
	domInd = ind;
	EmitVertex();
	
	gl_Position = scene.MTOmatrix[ind] * gl_in[1].gl_Position;
	domInd = ind;
	EmitVertex();

	gl_Position = scene.MTOmatrix[ind] * gl_in[2].gl_Position;
	domInd = ind;
	EmitVertex();

	EndPrimitive();
}


