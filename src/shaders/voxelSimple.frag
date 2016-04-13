///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 outNormal;
in vec4 outColor;
in vec4 outPosition;

out vec4 finalColor;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 2) uniform CameraBuffer {
	Camera cam;
};

void main()
{	
	vec3 n = normalize(outNormal);
	vec3 s = normalize(mat3(cam.WTVmatrix) * vec3(0.58, 0.58, 0.58));
	vec3 r = normalize(2 * n * dot(s,n) - s);
	vec3 v = normalize(-(outPosition.xyz / outPosition.w));

	// Calculate diffuse light
	float diff = max(0.2, dot(s, n));
	float spec = max(0.0, pow(dot(r, v), 5));

	// Set constant color for textureless models
	vec3 color = outColor.xyz * (diff + spec);

	finalColor = vec4(color, 1.0f);
}
