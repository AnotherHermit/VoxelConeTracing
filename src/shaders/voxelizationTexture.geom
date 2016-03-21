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
in vec2 exTexCoords[3];

out vec3 domDir;
out vec2 intTexCoords;

void main()
{
	vec3 dir = abs(exNormal[0]);
	float maxComponent = max(dir.x, max(dir.y, dir.z));
	dir = maxComponent == dir.x ? vec3(1.0f, 0.0f, 0.0f) : maxComponent == dir.y ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f);

	gl_Position = gl_in[0].gl_Position;
	domDir = dir;
	intTexCoords = exTexCoords[0];
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position;
	domDir = dir;
	intTexCoords = exTexCoords[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	domDir = dir;
	intTexCoords = exTexCoords[2];
	EmitVertex();

	EndPrimitive();
}


