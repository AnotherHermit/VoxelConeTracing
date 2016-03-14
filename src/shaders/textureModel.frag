///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 exNormal;
in vec4 exPosition;

out vec4 outColor;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec4 normals[8];
	vec4 points[8];
	vec3 position;
	uint padding99;
	vec4 lodLevels;
};

struct Program {
	float currentT;
	float deltaT;
	float radius;
	float simSpeed;
};

layout (std140, binding = 10) uniform CameraBuffer {
	Camera cam;
};

layout (std140, binding = 12) uniform ProgramBuffer {
	Program prog;
};

void main()
{	
	// Calculate diffuse light
	vec3 light = mat3(cam.WTVmatrix) * vec3(0.707, 0.707, 0);
	float shade = max(dot(normalize(exNormal), light), 0.1);
	
	// Set constant color for textured models
	vec3 color = vec3(0.0, 1.0, 1.0) * shade;

	// Output complete color
	outColor =  vec4(color, 1.0f);
}
