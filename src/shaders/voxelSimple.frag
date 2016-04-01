///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec4 outColor;
in vec3 outNormal;
out vec4 finalColor;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 10) uniform CameraBuffer {
	Camera cam;
};

void main()
{	
	// Calculate diffuse light
	vec3 light = mat3(cam.WTVmatrix) * vec3(0.58, 0.58, 0.58);
	float shade = max(dot(normalize(outNormal), light), 0.1);

	// Set constant color for textureless models
	vec3 color = outColor.xyz * shade;

	finalColor = vec4(color, 1.0f);
}
