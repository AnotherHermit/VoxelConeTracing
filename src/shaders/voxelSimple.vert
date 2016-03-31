///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

in vec3 inPosition;
in vec3 inNormal;

out vec3 exNormal;
out vec3 voxelPos;

uniform int voxelRes;
uniform mat4 MTWmatrix;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 10) uniform CameraBuffer {
	Camera cam;
};

void main(void)
{
	float size = float(voxelRes);

	int xPos = mod(gl_InstanceID, voxelRes);
	int temp = gl_InstanceID / voxelRes;
	int yPos = mod(temp, voxelRes);
	int zPos = temp / voxelRes;
	voxelPos = vec3(float(xPos) / size, float(yPos) / size, float(zPos) / size);

	exNormal = inNormal;
	gl_Position = VTPmatrix * WTVmatrix * MTWmatrix * vec4(inPosition, 1.0f);
}

