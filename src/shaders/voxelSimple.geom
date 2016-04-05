///////////////////////////////////////
//
//	Computer Graphics TSBK07
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform sampler3D voxelData;

in vec3 voxelPos[];
in vec3 exNormal[];

out vec4 outColor;
out vec3 outNormal;
out vec4 outPosition;

struct Camera {
	mat4 WTVmatrix;
	mat4 VTPmatrix;
	vec3 position;
};

layout (std140, binding = 10) uniform CameraBuffer {
	Camera cam;
};

void main()
{
	vec4 tempPos;
	vec4 color = texture(voxelData, voxelPos[0]);

	if(color.a < 0.5f) {
		return;
	}

	for(int i = 0; i < 3; i++) {
		tempPos = gl_in[i].gl_Position;
		outPosition = tempPos;
		gl_Position = cam.VTPmatrix * tempPos;
		outNormal = exNormal[i];
		outColor = color;
		EmitVertex();
	}
	
	EndPrimitive();
}


