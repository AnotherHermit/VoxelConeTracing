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

uniform vec3 diffColor;

void main()
{	
	vec3 n = normalize(exNormal);
	vec3 s = normalize(mat3(cam.WTVmatrix) * vec3(0.58, 0.58, 0.58));
	vec3 r = normalize(2 * n * dot(s,n) - s);
	vec3 v = normalize(-(exPosition.xyz / exPosition.w));

	// Calculate diffuse light
	float diff = max(0.2, dot(s, n));
	float spec = max(0.0, pow(dot(r, v), 5));

	// Set constant color for textureless models
	float shade = diff + spec;

	// Set constant color for textureless models
	vec3 color = diffColor * shade;
	
	// Output complete color
	outColor =  vec4(color, 1.0f);
}
