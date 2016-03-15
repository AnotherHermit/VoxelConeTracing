///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 exNormal;
in vec4 exPosition;
in vec2 exTexCoords;

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

uniform sampler2D diffuseUnit;

void main()
{	
	vec4 color = vec4(1.0, 0.0, 0.0, 1.0);

	// Calculate diffuse light
	vec3 light = mat3(cam.WTVmatrix) * vec3(0.707, 0.707, 0);
	float shade = max(dot(normalize(exNormal), light), 0.1);

	// Set constant color for textured models
	color = texture(diffuseUnit, exTexCoords) * shade;
	color.a = 1.0;

	// Output complete color
	outColor =  color;
}
