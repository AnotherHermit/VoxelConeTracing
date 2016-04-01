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

void main()
{
	vec4 color = texture(voxelData, voxelPos[0]);

	if(color.a < 0.5f) {
		return;
	}

	for(int i = 0; i < 3; i++) {
		gl_Position = gl_in[i].gl_Position;
		outNormal = exNormal[i];
		outColor = color;
		EmitVertex();
	}
	
	EndPrimitive();
}


