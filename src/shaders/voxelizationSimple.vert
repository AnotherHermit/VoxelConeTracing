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

struct SceneParams {
	mat4 MTOmatrix;
	uint view;
};

layout (std140, binding = 11) uniform SceneBuffer {
	SceneParams scene;
};

void main(void)
{
	exNormal = inNormal;
	gl_Position = scene.MTOmatrix * vec4(inPosition, 1.0f);
}

