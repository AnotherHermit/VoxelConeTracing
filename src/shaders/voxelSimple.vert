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

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 10) uniform CameraBuffer {
	Camera cam;
};

struct SceneParams {
	mat4 MTOmatrix[3];
	mat4 MTWmatrix;
	uint voxelDraw;
	uint view;
	uint voxelRes;
	uint voxelLayer;
};

layout (std140, binding = 11) uniform SceneBuffer {
	SceneParams scene;
};

void main(void)
{
	float size = float(scene.voxelRes);

	uint xPos = gl_InstanceID % scene.voxelRes;
	uint temp = gl_InstanceID / scene.voxelRes;
	uint yPos = temp % scene.voxelRes;
	uint zPos = temp / scene.voxelRes;
	voxelPos = vec3(float(xPos) / size, float(yPos) / size, float(zPos) / size);

	exNormal = inNormal;
	gl_Position = cam.VTPmatrix * cam.WTVmatrix * scene.MTWmatrix * vec4(inPosition + voxelPos, 1.0f);
}

