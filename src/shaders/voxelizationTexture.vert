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
out vec2 exTexCoords;

struct OrthoCam {
	mat4 WTVmatrix[3];
	mat4 VTPmatrix;
};

struct SceneParams {
	mat4 MTOmatrix;
	uint view;
};

layout (std140, binding = 10) uniform OrthoCamBuffer {
	OrthoCam cam;
};

layout (std140, binding = 11) uniform SceneBuffer {
	SceneParams scene;
};

void main(void)
{
	exNormal = inNormal;
	exTexCoords = inTexCoords;
	gl_Position = cam.VTPmatrix * cam.WTVmatrix[scene.view] * scene.MTOmatrix * vec4(inPosition, 1.0f);
}
