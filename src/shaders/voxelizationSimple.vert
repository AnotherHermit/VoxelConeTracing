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

struct OrthoCam {
	mat4 WTVmatrix[3];
	mat4 VTPmatrix;
};

struct LoaderParams {
	uint view;
};

layout (std140, binding = 10) uniform OrthoCamBuffer {
	OrthoCam cam;
};

layout (std140, binding = 11) uniform ModelLoaderBuffer {
	LoaderParams loader;
};

void main(void)
{
	exNormal = inNormal;
	gl_Position = cam.VTPmatrix * cam.WTVmatrix[loader.view] * vec4(inPosition, 1.0f);
}

