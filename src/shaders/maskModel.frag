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
uniform sampler2D maskUnit;

void main()
{	
	// Set constant color for textured models
	vec3 color = texture(diffuseUnit, exTexCoords).rgb;
	
	// Calculate diffuse light
	vec3 light = mat3(cam.WTVmatrix) * vec3(0.707, 0.707, 0);
	float shade = max(dot(normalize(exNormal), light), 0.1);

	// Apply light to texture
	vec3 shadedColor = color * shade;

	// Mask the texture color
	float outAlpha = texture(maskUnit, exTexCoords).r;

	// Output complete color
	outColor = vec4(shadedColor, outAlpha);
}