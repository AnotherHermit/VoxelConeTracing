///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 posValue;
in vec3 inPosition;
in vec3 inNormal;


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

void main(void)
{
	gl_Position = cam.VTPmatrix * cam.WTVmatrix * vec4(inPosition, 1.0f);
}
