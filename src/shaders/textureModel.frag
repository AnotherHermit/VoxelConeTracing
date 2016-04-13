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

layout (std140, binding = 2) uniform CameraBuffer {
	Camera cam;
};

layout (std140, binding = 1) uniform ProgramBuffer {
	Program prog;
};

uniform sampler2D diffuseUnit;

void main()
{	
	// Set constant color for textured models
	vec3 color = texture(diffuseUnit, exTexCoords).rgb;
	
	vec3 n = normalize(exNormal);
	vec3 s = normalize(mat3(cam.WTVmatrix) * vec3(0.58, 0.58, 0.58));
	vec3 r = normalize(2 * n * dot(s,n) - s);
	vec3 v = normalize(-(exPosition.xyz / exPosition.w));

	// Calculate diffuse light
	float diff = max(0.2, dot(s, n));
	float spec = max(0.0, pow(dot(r, v), 5));

	// Set constant color for textureless models
	float shade = diff + spec;

	// Apply light to texture
	vec3 shadedColor = color * shade;

	// Output complete color
	outColor =  vec4(shadedColor, 1.0);
}
