///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

out vec3 exNormal;
out vec4 exPosition;
out vec2 exTexCoords;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 0) uniform CameraBuffer {
	Camera cam;
};

void main(void)
{
	exNormal = mat3(cam.WTVmatrix) * inNormal;
		
	vec4 temp = cam.WTVmatrix * vec4(inPosition, 1.0f);
	
	exPosition = temp;
	
	gl_Position = cam.VTPmatrix * temp;

	exTexCoords = inTexCoords;
}
