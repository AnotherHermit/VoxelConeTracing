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

out vec3 exNormal;
out vec4 exPosition;

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

void main(void)
{
	exNormal = mat3(cam.WTVmatrix) * inNormal;
		
	vec4 temp = cam.WTVmatrix * vec4(inPosition, 1.0f);
	
	exPosition = temp;
	
	gl_Position = cam.VTPmatrix * temp;
}
