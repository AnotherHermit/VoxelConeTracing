///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoords;

out vec3 exNormal;
out vec4 exPosition;
out vec2 exTexCoords;

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

void main(void)
{
	exNormal = mat3(cam.WTVmatrix) * inNormal;
		
	vec4 temp = cam.WTVmatrix * vec4(inPosition, 1.0f);
	
	exPosition = temp;
	
	gl_Position = cam.VTPmatrix * temp;

	exTexCoords = inTexCoords;
}
