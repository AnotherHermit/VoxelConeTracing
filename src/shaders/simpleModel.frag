///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahl�n - conwa099
//
///////////////////////////////////////

#version 430

in vec3 exNormal;
in vec4 exPosition;

out vec4 outColor;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

struct Program {
	float currentT;
	float deltaT;
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

	// Set constant color for textureless models
	vec3 color = vec3(1.0, 0.0, 1.0) * shade;
	
	// Output complete color
	outColor =  vec4(color, 1.0f);
}
