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

flat out uint domDir;

struct OrthoCam {
	mat4 WTVmatrix[3];
	mat4 VTPmatrix;
};

layout (std140, binding = 10) uniform OrthoCamBuffer {
	OrthoCam cam;
};

void main()
{
	vec3 dir = abs(exNormal[0]);
	float maxComponent = max(dir.x, max(dir.y, dir.z));
	uint tempDir = maxComponent == dir.x ? 0 : maxComponent == dir.y ? 1 : 2;

	gl_Position = cam.VTPmatrix * cam.WTVmatrix[tempDir] * gl_in[0].gl_Position;
	domDir = tempDir;
	EmitVertex();
	
	gl_Position = cam.VTPmatrix * cam.WTVmatrix[tempDir] * gl_in[1].gl_Position;
	domDir = tempDir;
	EmitVertex();

	gl_Position = cam.VTPmatrix * cam.WTVmatrix[tempDir] * gl_in[2].gl_Position;
	domDir = tempDir;
	EmitVertex();

	EndPrimitive();
}


